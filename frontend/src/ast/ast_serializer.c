#include "frontend/ast/ast_serializer.h"

#include <setjmp.h>
#include <string.h>

struct ast_serializer {
    jmp_buf err_buf;
    FILE* file;
};

static void serializer_write(struct ast_serializer* d,
                             const void* buffer,
                             size_t size,
                             size_t count) {
    if (fwrite(buffer, size, count, d->file) < count) {
        longjmp(d->err_buf, 0);
    }
}

static void serialize_file_info(struct ast_serializer* d,
                                const struct file_info* info);

static void serialize_translation_unit(struct ast_serializer* d,
                                       const struct translation_unit* tl);

bool serialize_ast(const struct translation_unit* tl,
                   const struct file_info* file_info,
                   FILE* f) {
    struct ast_serializer d = {
        .file = f,
    };

    if (setjmp(d.err_buf) == 0) {
        serialize_file_info(&d, file_info);
        serialize_translation_unit(&d, tl);
    } else {
        return false;
    }
    return true;
}

static void serialize_bool(struct ast_serializer* d, bool b) {
    serializer_write(d, &b, sizeof b, 1);
}

static void serialize_uint(struct ast_serializer* d, uint64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_int(struct ast_serializer* d, int64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_float(struct ast_serializer* d, double f) {
    serializer_write(d, &f, sizeof f, 1);
}

static void serialize_str(struct ast_serializer* d, const struct str* str) {
    const size_t len = str_len(str);
    serialize_uint(d, len);
    const char* data = str_get_data(str);
    serializer_write(d, data, sizeof *data, len);
}

static void serialize_file_info(struct ast_serializer* d,
                                const struct file_info* info) {
    serialize_uint(d, info->len);
    for (size_t i = 0; i < info->len; ++i) {
        serialize_str(d, &info->paths[i]);
    }
}

static void serialize_ast_node_info(struct ast_serializer* d,
                                    const struct ast_node_info* info) {
    serialize_uint(d, info->loc.file_idx);
    serialize_uint(d, info->loc.file_loc.line);
    serialize_uint(d, info->loc.file_loc.index);
}

static void serialize_type_quals(struct ast_serializer* d,
                                 const struct type_quals* quals) {
    serialize_bool(d, quals->is_const);
    serialize_bool(d, quals->is_restrict);
    serialize_bool(d, quals->is_volatile);
    serialize_bool(d, quals->is_atomic);
}

static void serialize_type_name(struct ast_serializer* d,
                                const struct type_name* name);

static void serialize_atomic_type_spec(struct ast_serializer* d,
                                       const struct atomic_type_spec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_type_name(d, spec->type_name);
}

static void serialize_identifier(struct ast_serializer* d,
                                 const struct identifier* id) {
    serialize_ast_node_info(d, &id->info);
    serialize_str(d, &id->spelling);
}

static void serialize_int_value(struct ast_serializer* d,
                                const struct int_value* val) {
    serialize_uint(d, val->type);
    if (int_value_is_signed(val->type)) {
        serialize_int(d, val->int_val);
    } else {
        serialize_uint(d, val->uint_val);
    }
}

static void serialize_float_value(struct ast_serializer* d,
                                  const struct float_value* val) {
    serialize_uint(d, val->type);
    serialize_float(d, val->val);
}

static void serialize_constant(struct ast_serializer* d,
                               const struct constant* constant) {
    serialize_ast_node_info(d, &constant->info);
    const uint64_t type = constant->type;
    assert((enum constant_type)type == constant->type);
    serialize_uint(d, type);
    switch (constant->type) {
        case CONSTANT_ENUM:
            serialize_str(d, &constant->spelling);
            break;
        case CONSTANT_INT:
            serialize_int_value(d, &constant->int_val);
            break;
        case CONSTANT_FLOAT:
            serialize_float_value(d, &constant->float_val);
            break;
    }
}

static void serialize_string_literal(struct ast_serializer* d,
                                     const struct string_literal_node* lit) {
    serialize_ast_node_info(d, &lit->info);
    serialize_str(d, &lit->spelling);
}

static void serialize_string_constant(struct ast_serializer* d,
                                      const struct string_constant* constant) {
    serialize_bool(d, constant->is_func);
    if (constant->is_func) {
        serialize_ast_node_info(d, &constant->info);
    } else {
        serialize_string_literal(d, &constant->lit);
    }
}

static void serialize_unary_expr(struct ast_serializer* d,
                                 const struct unary_expr* expr);

static void serialize_cond_expr(struct ast_serializer* d,
                                const struct cond_expr* expr);

static void serialize_assign_expr(struct ast_serializer* d,
                                  const struct assign_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct unary_and_op* item = &expr->assign_chain[i];
        serialize_unary_expr(d, item->unary);
        const uint64_t op = item->op;
        assert((enum assign_expr_op)op == item->op);
        serialize_uint(d, op);
    }
    serialize_cond_expr(d, expr->value);
}

static void serialize_expr(struct ast_serializer* d, const struct expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_assign_expr(d, &expr->assign_exprs[i]);
    }
}

static void serialize_generic_assoc(struct ast_serializer* d,
                                    const struct generic_assoc* assoc) {
    serialize_ast_node_info(d, &assoc->info);
    const bool has_type_name = assoc->type_name != NULL;
    serialize_bool(d, has_type_name);
    if (has_type_name) {
        serialize_type_name(d, assoc->type_name);
    }
    serialize_assign_expr(d, assoc->assign);
}

static void serialize_generic_assoc_list(struct ast_serializer* d,
                                         const struct generic_assoc_list* lst) {
    serialize_ast_node_info(d, &lst->info);
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_generic_assoc(d, &lst->assocs[i]);
    }
}

static void serialize_generic_sel(struct ast_serializer* d,
                                  const struct generic_sel* sel) {
    serialize_ast_node_info(d, &sel->info);
    serialize_assign_expr(d, sel->assign);
    serialize_generic_assoc_list(d, &sel->assocs);
}

static void serialize_primary_expr(struct ast_serializer* d,
                                   const struct primary_expr* expr) {
    const uint64_t type = expr->type;
    serialize_uint(d, type);
    assert((enum primary_expr_type)type == expr->type);
    switch (expr->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            serialize_identifier(d, expr->identifier);
            break;
        case PRIMARY_EXPR_CONSTANT:
            serialize_constant(d, &expr->constant);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            serialize_string_constant(d, &expr->string);
            break;
        case PRIMARY_EXPR_BRACKET:
            serialize_ast_node_info(d, &expr->info);
            serialize_expr(d, expr->bracket_expr);
            break;
        case PRIMARY_EXPR_GENERIC:
            serialize_generic_sel(d, expr->generic);
            break;
    }
}

static void serialize_const_expr(struct ast_serializer* d,
                                 const struct const_expr* expr);

static void serialize_designator(struct ast_serializer* d,
                                 const struct designator* des) {
    serialize_ast_node_info(d, &des->info);
    serialize_bool(d, des->is_index);
    if (des->is_index) {
        serialize_const_expr(d, des->arr_index);
    } else {
        serialize_identifier(d, des->identifier);
    }
}

static void serialize_designator_list(struct ast_serializer* d,
                                      const struct designator_list* des) {
    serialize_uint(d, des->len);
    for (size_t i = 0; i < des->len; ++i) {
        serialize_designator(d, &des->designators[i]);
    }
}

static void serialize_designation(struct ast_serializer* d,
                                  const struct designation* des) {
    serialize_designator_list(d, &des->designators);
}

static void serialize_init_list(struct ast_serializer* d,
                                const struct init_list* lst);

static void serialize_initializer(struct ast_serializer* d,
                                  const struct initializer* init) {
    serialize_ast_node_info(d, &init->info);
    serialize_bool(d, init->is_assign);
    if (init->is_assign) {
        serialize_assign_expr(d, init->assign);
    } else {
        serialize_init_list(d, &init->init_list);
    }
}

static void serialize_designation_init(struct ast_serializer* d,
                                       const struct designation_init* init) {
    const bool has_designation = is_valid_designation(&init->designation);
    serialize_bool(d, has_designation);
    if (has_designation) {
        serialize_designation(d, &init->designation);
    }
    serialize_initializer(d, &init->init);
}

static void serialize_init_list(struct ast_serializer* d,
                                const struct init_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_designation_init(d, &lst->inits[i]);
    }
}

static void serialize_arg_expr_list(struct ast_serializer* d,
                                    const struct arg_expr_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_assign_expr(d, &lst->assign_exprs[i]);
    }
}

static void serialize_postfix_suffix(struct ast_serializer* d,
                                     const struct postfix_suffix* suffix) {
    const uint64_t type = suffix->type;
    serialize_uint(d, type);
    assert((enum postfix_suffix_type)type == suffix->type);

    switch (suffix->type) {
        case POSTFIX_INDEX:
            serialize_expr(d, suffix->index_expr);
            break;
        case POSTFIX_BRACKET:
            serialize_arg_expr_list(d, &suffix->bracket_list);
            break;
        case POSTFIX_ACCESS:
        case POSTFIX_PTR_ACCESS:
            serialize_identifier(d, suffix->identifier);
            break;
        case POSTFIX_INC:
        case POSTFIX_DEC:
            break;
    }
}

static void serialize_postfix_expr(struct ast_serializer* d,
                                   const struct postfix_expr* expr) {
    serialize_bool(d, expr->is_primary);
    if (expr->is_primary) {
        serialize_primary_expr(d, expr->primary);
    } else {
        serialize_ast_node_info(d, &expr->info);
        serialize_type_name(d, expr->type_name);
        serialize_init_list(d, &expr->init_list);
    }
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_postfix_suffix(d, &expr->suffixes[i]);
    }
}

static void serialize_cast_expr(struct ast_serializer* d,
                                const struct cast_expr* expr);

static void serialize_unary_expr(struct ast_serializer* d,
                                 const struct unary_expr* expr) {
    serialize_ast_node_info(d, &expr->info);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const uint64_t unary_op = expr->ops_before[i];
        assert((enum unary_expr_op)unary_op == expr->ops_before[i]);
        serialize_uint(d, unary_op);
    }
    const uint64_t type = expr->type;
    serialize_uint(d, type);
    assert((enum unary_expr_type)type == expr->type);
    switch (expr->type) {
        case UNARY_POSTFIX:
            serialize_postfix_expr(d, expr->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            serialize_cast_expr(d, expr->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            serialize_type_name(d, expr->type_name);
            break;
    }
}

static void serialize_cast_expr(struct ast_serializer* d,
                                const struct cast_expr* expr) {
    serialize_ast_node_info(d, &expr->info);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_type_name(d, &expr->type_names[i]);
    }
    serialize_unary_expr(d, expr->rhs);
}

static void serialize_mul_expr(struct ast_serializer* d,
                               const struct mul_expr* expr) {
    serialize_cast_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct cast_expr_and_op* item = &expr->mul_chain[i];
        const uint64_t mul_op = item->op;
        assert((enum mul_expr_op)mul_op == item->op);
        serialize_uint(d, mul_op);
        serialize_cast_expr(d, item->rhs);
    }
}

static void serialize_add_expr(struct ast_serializer* d,
                               const struct add_expr* expr) {
    serialize_mul_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct mul_expr_and_op* item = &expr->add_chain[i];
        const uint64_t add_op = item->op;
        assert((enum add_expr_op)add_op == item->op);
        serialize_uint(d, add_op);
        serialize_mul_expr(d, item->rhs);
    }
}

static void serialize_shift_expr(struct ast_serializer* d,
                                 const struct shift_expr* expr) {
    serialize_add_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct add_expr_and_op* item = &expr->shift_chain[i];
        const uint64_t shift_op = item->op;
        assert((enum shift_expr_op)shift_op == item->op);
        serialize_uint(d, shift_op);
        serialize_add_expr(d, item->rhs);
    }
}

static void serialize_rel_expr(struct ast_serializer* d,
                               const struct rel_expr* expr) {
    serialize_shift_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct shift_expr_and_op* item = &expr->rel_chain[i];
        const uint64_t rel_op = item->op;
        assert((enum rel_expr_op)rel_op == item->op);
        serialize_uint(d, rel_op);
        serialize_shift_expr(d, item->rhs);
    }
}

static void serialize_eq_expr(struct ast_serializer* d,
                              const struct eq_expr* expr) {
    serialize_rel_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct rel_expr_and_op* item = &expr->eq_chain[i];
        const uint64_t eq_op = item->op;
        assert((enum eq_expr_op)eq_op == item->op);
        serialize_uint(d, eq_op);
        serialize_rel_expr(d, item->rhs);
    }
}

static void serialize_and_expr(struct ast_serializer* d,
                               const struct and_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_eq_expr(d, &expr->eq_exprs[i]);
    }
}

static void serialize_xor_expr(struct ast_serializer* d,
                               const struct xor_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_and_expr(d, &expr->and_exprs[i]);
    }
}

static void serialize_or_expr(struct ast_serializer* d,
                              const struct or_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_xor_expr(d, &expr->xor_exprs[i]);
    }
}

static void serialize_log_and_expr(struct ast_serializer* d,
                                   const struct log_and_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_or_expr(d, &expr->or_exprs[i]);
    }
}

static void serialize_log_or_expr(struct ast_serializer* d,
                                  const struct log_or_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_log_and_expr(d, &expr->log_ands[i]);
    }
}

static void serialize_cond_expr(struct ast_serializer* d,
                                const struct cond_expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct log_or_and_expr* item = &expr->conditionals[i];
        serialize_log_or_expr(d, item->log_or);
        serialize_expr(d, item->expr);
    }
    serialize_log_or_expr(d, expr->last_else);
}

static void serialize_const_expr(struct ast_serializer* d,
                                 const struct const_expr* expr) {
    serialize_cond_expr(d, &expr->expr);
}

static void serialize_static_assert_declaration(
    struct ast_serializer* d,
    const struct static_assert_declaration* decl) {
    serialize_const_expr(d, decl->const_expr);
    serialize_string_literal(d, &decl->err_msg);
}

static void serialize_pointer(struct ast_serializer* d,
                              const struct pointer* ptr) {
    serialize_ast_node_info(d, &ptr->info);
    serialize_uint(d, ptr->num_indirs);
    for (size_t i = 0; i < ptr->num_indirs; ++i) {
        serialize_type_quals(d, &ptr->quals_after_ptr[i]);
    }
}

static void serialize_param_type_list(struct ast_serializer* d,
                                      const struct param_type_list* lst);

static void serialize_abs_arr_or_func_suffix(
    struct ast_serializer* d,
    const struct abs_arr_or_func_suffix* suffix) {
    serialize_ast_node_info(d, &suffix->info);
    const uint64_t type = suffix->type;
    assert((enum abs_arr_or_func_suffix_type)type == suffix->type);
    serialize_uint(d, type);
    switch (suffix->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            serialize_bool(d, suffix->has_asterisk);
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            serialize_bool(d, suffix->is_static);
            serialize_type_quals(d, &suffix->type_quals);
            const bool has_assign = suffix->assign != NULL;
            serialize_bool(d, has_assign);
            if (has_assign) {
                serialize_assign_expr(d, suffix->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            serialize_param_type_list(d, &suffix->func_types);
            break;
    }
}

static void serialize_abs_declarator(struct ast_serializer* d,
                                     const struct abs_declarator* decl);

static void serialize_direct_abs_declarator(
    struct ast_serializer* d,
    const struct direct_abs_declarator* decl) {
    serialize_ast_node_info(d, &decl->info);
    const bool has_bracket_decl = decl->bracket_decl != NULL;
    serialize_bool(d, has_bracket_decl);
    if (has_bracket_decl) {
        serialize_abs_declarator(d, decl->bracket_decl);
    }
    serialize_uint(d, decl->len);
    for (size_t i = 0; i < decl->len; ++i) {
        serialize_abs_arr_or_func_suffix(d, &decl->following_suffixes[i]);
    }
}

static void serialize_abs_declarator(struct ast_serializer* d,
                                     const struct abs_declarator* decl) {
    const bool has_ptr = decl->ptr != NULL;
    serialize_bool(d, has_ptr);
    if (has_ptr) {
        serialize_pointer(d, decl->ptr);
    }
    const bool has_direct_abs_decl = decl->direct_abs_decl != NULL;
    serialize_bool(d, has_direct_abs_decl);
    if (has_direct_abs_decl) {
        serialize_direct_abs_declarator(d, decl->direct_abs_decl);
    }
}

static void serialize_declaration_specs(struct ast_serializer* d,
                                        const struct declaration_specs* specs);

static void serialize_declarator(struct ast_serializer* d,
                                 const struct declarator* decl);

static void serialize_param_declaration(struct ast_serializer* d,
                                        const struct param_declaration* decl) {
    serialize_declaration_specs(d, decl->decl_specs);
    const uint64_t type = decl->type;
    assert((enum param_decl_type)type == decl->type);
    serialize_uint(d, type);
    switch (decl->type) {
        case PARAM_DECL_DECL:
            serialize_declarator(d, decl->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            serialize_abs_declarator(d, decl->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            break;
    }
}

static void serialize_param_list(struct ast_serializer* d,
                                 const struct param_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_param_declaration(d, &lst->decls[i]);
    }
}

static void serialize_param_type_list(struct ast_serializer* d,
                                      const struct param_type_list* lst) {
    serialize_bool(d, lst->is_variadic);
    serialize_param_list(d, &lst->param_list);
}

static void serialize_identifier_list(struct ast_serializer* d,
                                      const struct identifier_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_identifier(d, &lst->identifiers[i]);
    }
}

static void serialize_arr_suffix(struct ast_serializer* d,
                                 const struct arr_suffix* suffix) {
    serialize_bool(d, suffix->is_static);
    serialize_type_quals(d, &suffix->type_quals);
    serialize_bool(d, suffix->is_asterisk);
    const bool has_assign_expr = suffix->arr_len != NULL;
    serialize_bool(d, has_assign_expr);
    if (has_assign_expr) {
        serialize_assign_expr(d, suffix->arr_len);
    }
}

static void serialize_arr_or_func_suffix(
    struct ast_serializer* d,
    const struct arr_or_func_suffix* suffix) {
    serialize_ast_node_info(d, &suffix->info);
    const uint64_t type = suffix->type;
    assert((enum arr_or_func_suffix_type)type == suffix->type);
    serialize_uint(d, type);
    switch (suffix->type) {
        case ARR_OR_FUNC_ARRAY:
            serialize_arr_suffix(d, &suffix->arr_suffix);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            serialize_param_type_list(d, &suffix->fun_types);
            break;
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            serialize_identifier_list(d, &suffix->fun_params);
            break;
        case ARR_OR_FUNC_FUN_EMPTY:
            break;
    }
}

static void serialize_direct_declarator(struct ast_serializer* d,
                                        const struct direct_declarator* decl) {
    serialize_ast_node_info(d, &decl->info);
    serialize_bool(d, decl->is_id);
    if (decl->is_id) {
        serialize_identifier(d, decl->id);
    } else {
        serialize_declarator(d, decl->bracket_decl);
    }
    serialize_uint(d, decl->len);
    for (size_t i = 0; i < decl->len; ++i) {
        serialize_arr_or_func_suffix(d, &decl->suffixes[i]);
    }
}

static void serialize_declarator(struct ast_serializer* d,
                                 const struct declarator* decl) {
    const bool has_ptr = decl->ptr != NULL;
    serialize_bool(d, has_ptr);
    if (has_ptr) {
        serialize_pointer(d, decl->ptr);
    }
    serialize_direct_declarator(d, decl->direct_decl);
}

static void serialize_struct_declarator(struct ast_serializer* d,
                                        const struct struct_declarator* decl) {
    const bool has_decl = decl->decl != NULL;
    serialize_bool(d, has_decl);
    if (has_decl) {
        serialize_declarator(d, decl->decl);
    }
    const bool has_bit_field = decl->bit_field != NULL;
    serialize_bool(d, has_bit_field);
    if (has_bit_field) {
        serialize_const_expr(d, decl->bit_field);
    }
}

static void serialize_struct_declarator_list(
    struct ast_serializer* d,
    const struct struct_declarator_list* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_struct_declarator(d, &decls->decls[i]);
    }
}

static void serialize_struct_declaration(
    struct ast_serializer* d,
    const struct struct_declaration* decl) {
    serialize_bool(d, decl->is_static_assert);
    if (decl->is_static_assert) {
        serialize_static_assert_declaration(d, decl->assert);
    } else {
        serialize_declaration_specs(d, decl->decl_specs);
        serialize_struct_declarator_list(d, &decl->decls);
    }
}

static void serialize_struct_declaration_list(
    struct ast_serializer* d,
    const struct struct_declaration_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_struct_declaration(d, &lst->decls[i]);
    }
}

static void serialize_struct_union_spec(struct ast_serializer* d,
                                        const struct struct_union_spec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_bool(d, spec->is_struct);
    const bool has_identifier = spec->identifier != NULL;
    serialize_bool(d, has_identifier);
    if (has_identifier) {
        serialize_identifier(d, spec->identifier);
    }
    serialize_struct_declaration_list(d, &spec->decl_list);
}

static void serialize_enumerator(struct ast_serializer* d,
                                 const struct enumerator* enumerator) {
    serialize_identifier(d, enumerator->identifier);
    const bool has_enum_val = enumerator->enum_val != NULL;
    serialize_bool(d, has_enum_val);
    if (has_enum_val) {
        serialize_const_expr(d, enumerator->enum_val);
    }
}

static void serialize_enum_list(struct ast_serializer* d,
                                const struct enum_list* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_enumerator(d, &lst->enums[i]);
    }
}

static void serialize_enum_spec(struct ast_serializer* d,
                                const struct enum_spec* spec) {
    serialize_ast_node_info(d, &spec->info);
    const bool has_identifier = spec->identifier != NULL;
    serialize_bool(d, has_identifier);
    if (has_identifier) {
        serialize_identifier(d, spec->identifier);
    }
    serialize_enum_list(d, &spec->enum_list);
}

static void serialize_type_modifiers(struct ast_serializer* d,
                                     const struct type_modifiers* mods) {
    serialize_bool(d, mods->is_unsigned);
    serialize_bool(d, mods->is_signed);
    serialize_bool(d, mods->is_short);
    serialize_uint(d, mods->num_long);
    serialize_bool(d, mods->is_complex);
    serialize_bool(d, mods->is_imaginary);
}

static void serialize_type_specs(struct ast_serializer* d,
                                 const struct type_specs* specs) {
    serialize_type_modifiers(d, &specs->mods);
    const uint64_t type = specs->type;
    assert((enum type_spec_type)type == specs->type);
    serialize_uint(d, type);
    switch (specs->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            break;
        case TYPE_SPEC_ATOMIC:
            serialize_atomic_type_spec(d, specs->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            serialize_struct_union_spec(d, specs->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            serialize_enum_spec(d, specs->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            serialize_identifier(d, specs->typedef_name);
            break;
    }
}

static void serialize_spec_qual_list(struct ast_serializer* d,
                                     const struct spec_qual_list* lst) {
    serialize_ast_node_info(d, &lst->info);
    serialize_type_quals(d, &lst->quals);
    serialize_type_specs(d, &lst->specs);
}

static void serialize_type_name(struct ast_serializer* d,
                                const struct type_name* name) {
    serialize_spec_qual_list(d, name->spec_qual_list);
    const bool has_abs_decl = name->abstract_decl != NULL;
    serialize_bool(d, has_abs_decl);
    if (has_abs_decl) {
        serialize_abs_declarator(d, name->abstract_decl);
    }
}

static void serialize_align_spec(struct ast_serializer* d,
                                 const struct align_spec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_bool(d, spec->is_type_name);
    if (spec->is_type_name) {
        serialize_type_name(d, spec->type_name);
    } else {
        serialize_const_expr(d, spec->const_expr);
    }
}

static void serialize_func_specs(struct ast_serializer* d,
                                 const struct func_specs* specs) {
    serialize_bool(d, specs->is_inline);
    serialize_bool(d, specs->is_noreturn);
}

static void serialize_storage_class(struct ast_serializer* d,
                                    const struct storage_class* class) {
    serialize_bool(d, class->is_typedef);
    serialize_bool(d, class->is_extern);
    serialize_bool(d, class->is_static);
    serialize_bool(d, class->is_thread_local);
    serialize_bool(d, class->is_auto);
    serialize_bool(d, class->is_register);
}

static void serialize_declaration_specs(struct ast_serializer* d,
                                        const struct declaration_specs* specs) {
    serialize_ast_node_info(d, &specs->info);
    serialize_func_specs(d, &specs->func_specs);
    serialize_storage_class(d, &specs->storage_class);
    serialize_type_quals(d, &specs->type_quals);
    serialize_uint(d, specs->num_align_specs);
    for (size_t i = 0; i < specs->num_align_specs; ++i) {
        serialize_align_spec(d, &specs->align_specs[i]);
    }

    serialize_type_specs(d, &specs->type_specs);
}

static void serialize_declaration(struct ast_serializer* d,
                                  const struct declaration* decl);

static void serialize_declaration_list(struct ast_serializer* d,
                                       const struct declaration_list* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_declaration(d, &decls->decls[i]);
    }
}

static void serialize_statement(struct ast_serializer* d,
                                const struct statement* stat);

static void serialize_labeled_statement(struct ast_serializer* d,
                                        const struct labeled_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t type = stat->type;
    assert((enum labeled_statement_type)type == stat->type);
    serialize_uint(d, type);
    switch (stat->type) {
        case LABELED_STATEMENT_CASE:
            serialize_const_expr(d, stat->case_expr);
            break;
        case LABELED_STATEMENT_LABEL:
            serialize_identifier(d, stat->label);
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
    }
    serialize_statement(d, stat->stat);
}

static void serialize_expr_statement(struct ast_serializer* d,
                                     const struct expr_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    serialize_expr(d, &stat->expr);
}

static void serialize_selection_statement(
    struct ast_serializer* d,
    const struct selection_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    serialize_bool(d, stat->is_if);
    serialize_expr(d, stat->sel_expr);
    serialize_statement(d, stat->sel_stat);
    const bool has_else = stat->else_stat != NULL;
    serialize_bool(d, has_else);
    if (has_else) {
        serialize_statement(d, stat->else_stat);
    }
}

static void serialize_for_loop(struct ast_serializer* d,
                               const struct for_loop* loop) {
    serialize_bool(d, loop->is_decl);
    if (loop->is_decl) {
        serialize_declaration(d, &loop->init_decl);
    } else {
        serialize_expr_statement(d, loop->init_expr);
    }
    serialize_expr_statement(d, loop->cond);
    serialize_expr(d, loop->incr_expr);
}

static void serialize_iteration_statement(
    struct ast_serializer* d,
    const struct iteration_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t type = stat->type;
    assert((enum iteration_statement_type)type == stat->type);
    serialize_uint(d, type);
    serialize_statement(d, stat->loop_body);
    switch (stat->type) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            serialize_expr(d, stat->while_cond);
            break;
        case ITERATION_STATEMENT_FOR:
            serialize_for_loop(d, &stat->for_loop);
            break;
    }
}

static void serialize_jump_statement(struct ast_serializer* d,
                                     const struct jump_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t type = stat->type;
    assert((enum jump_statement_type)type == stat->type);
    serialize_uint(d, type);
    switch (stat->type) {
        case JUMP_STATEMENT_GOTO:
            serialize_identifier(d, stat->goto_label);
            break;
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            break;
        case JUMP_STATEMENT_RETURN: {
            const bool has_ret_val = stat->ret_val != NULL;
            serialize_bool(d, has_ret_val);
            if (has_ret_val) {
                serialize_expr(d, stat->ret_val);
            }
            break;
        }
    }
}

static void serialize_compound_statement(struct ast_serializer* d,
                                         const struct compound_statement* stat);

static void serialize_statement(struct ast_serializer* d,
                                const struct statement* stat) {
    const uint64_t type = stat->type;
    assert((enum statement_type)type == stat->type);
    serialize_uint(d, type);
    switch (stat->type) {
        case STATEMENT_LABELED:
            serialize_labeled_statement(d, stat->labeled);
            break;
        case STATEMENT_COMPOUND:
            serialize_compound_statement(d, stat->comp);
            break;
        case STATEMENT_EXPRESSION:
            serialize_expr_statement(d, stat->expr);
            break;
        case STATEMENT_SELECTION:
            serialize_selection_statement(d, stat->sel);
            break;
        case STATEMENT_ITERATION:
            serialize_iteration_statement(d, stat->it);
            break;
        case STATEMENT_JUMP:
            serialize_jump_statement(d, stat->jmp);
            break;
    }
}

static void serialize_block_item(struct ast_serializer* d,
                                 const struct block_item* item) {
    serialize_bool(d, item->is_decl);
    if (item->is_decl) {
        serialize_declaration(d, &item->decl);
    } else {
        serialize_statement(d, &item->stat);
    }
}

static void serialize_compound_statement(
    struct ast_serializer* d,
    const struct compound_statement* stat) {
    serialize_ast_node_info(d, &stat->info);
    serialize_uint(d, stat->len);
    for (size_t i = 0; i < stat->len; ++i) {
        serialize_block_item(d, &stat->items[i]);
    }
}

static void serialize_func_def(struct ast_serializer* d,
                               const struct func_def* def) {
    serialize_declaration_specs(d, def->specs);
    serialize_declarator(d, def->decl);
    serialize_declaration_list(d, &def->decl_list);
    serialize_compound_statement(d, &def->comp);
}

static void serialize_init_declarator(struct ast_serializer* d,
                                      const struct init_declarator* decl) {
    serialize_declarator(d, decl->decl);
    const bool has_init = decl->init != NULL;
    serialize_bool(d, has_init);
    if (has_init) {
        serialize_initializer(d, decl->init);
    }
}

static void serialize_init_declarator_list(
    struct ast_serializer* d,
    const struct init_declarator_list* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_init_declarator(d, &decls->decls[i]);
    }
}

static void serialize_declaration(struct ast_serializer* d,
                                  const struct declaration* decl) {
    serialize_bool(d, decl->is_normal_decl);
    if (decl->is_normal_decl) {
        serialize_declaration_specs(d, decl->decl_specs);
        serialize_init_declarator_list(d, &decl->init_decls);
    } else {
        serialize_static_assert_declaration(d, decl->static_assert_decl);
    }
}

static void serialize_external_declaration(
    struct ast_serializer* d,
    const struct external_declaration* decl) {
    serialize_bool(d, decl->is_func_def);
    if (decl->is_func_def) {
        serialize_func_def(d, &decl->func_def);
    } else {
        serialize_declaration(d, &decl->decl);
    }
}

static void serialize_translation_unit(struct ast_serializer* d,
                                       const struct translation_unit* tl) {
    serialize_uint(d, tl->len);
    for (size_t i = 0; i < tl->len; ++i) {
        serialize_external_declaration(d, &tl->external_decls[i]);
    }
}
