#include "frontend/ast/ast_bin_dumper.h"

#include <setjmp.h>

struct ast_bin_dumper {
    jmp_buf err_buf;
    FILE* file;
};

static void bin_dumper_write(struct ast_bin_dumper* d,
                             const void* buffer,
                             size_t size,
                             size_t count) {
    if (fwrite(buffer, size, count, d->file) < size) {
        longjmp(d->err_buf, 0);
    }
}

static void bin_dump_file_info(struct ast_bin_dumper* d,
                               const struct file_info* info);

static void bin_dump_translation_unit(struct ast_bin_dumper* d,
                                      const struct translation_unit* tl);

bool bin_dump_ast(const struct translation_unit* tl,
                  const struct file_info* file_info,
                  FILE* f) {
    struct ast_bin_dumper d = {
        .file = f,
    };

    if (setjmp(d.err_buf) == 0) {
        bin_dump_file_info(&d, file_info);
        bin_dump_translation_unit(&d, tl);
    } else {
        return false;
    }
    return true;
}

static void bin_dump_bool(struct ast_bin_dumper* d, bool b) {
    bin_dumper_write(d, &b, sizeof b, 1);
}

static void bin_dump_uint(struct ast_bin_dumper* d, uint64_t i) {
    bin_dumper_write(d, &i, sizeof i, 1);
}

static void bin_dump_int(struct ast_bin_dumper* d, int64_t i) {
    bin_dumper_write(d, &i, sizeof i, 1);
}

static void bin_dump_float(struct ast_bin_dumper* d, long double f) {
    bin_dumper_write(d, &f, sizeof f, 1);
}

static void bin_dump_str(struct ast_bin_dumper* d, const struct str* str) {
    const size_t len = str_len(str);
    bin_dump_uint(d, len);
    const char* data = str_get_data(str);
    bin_dumper_write(d, data, sizeof *data, len);
}

static void bin_dump_file_info(struct ast_bin_dumper* d,
                               const struct file_info* info) {
    bin_dump_uint(d, info->len);
    for (size_t i = 0; i < info->len; ++i) {
        bin_dump_str(d, &info->paths[i]);
    }
}

static void bin_dump_ast_node_info(struct ast_bin_dumper* d,
                                   const struct ast_node_info* info) {
    bin_dump_uint(d, info->loc.file_idx);
    bin_dump_uint(d, info->loc.file_loc.line);
    bin_dump_uint(d, info->loc.file_loc.index);
}

static void bin_dump_type_quals(struct ast_bin_dumper* d,
                                const struct type_quals* quals) {
    bin_dump_bool(d, quals->is_const);
    bin_dump_bool(d, quals->is_restrict);
    bin_dump_bool(d, quals->is_volatile);
    bin_dump_bool(d, quals->is_atomic);
}

static void bin_dump_type_name(struct ast_bin_dumper* d,
                               const struct type_name* name);

static void bin_dump_atomic_type_spec(struct ast_bin_dumper* d,
                                      const struct atomic_type_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_type_name(d, spec->type_name);
}

static void bin_dump_identifier(struct ast_bin_dumper* d,
                                const struct identifier* id) {
    bin_dump_ast_node_info(d, &id->info);
    bin_dump_str(d, &id->spelling);
}

static void bin_dump_int_value(struct ast_bin_dumper* d,
                               const struct int_value* val) {
    bin_dump_uint(d, val->type);
    if (int_value_is_signed(val->type)) {
        bin_dump_int(d, val->int_val);
    } else {
        bin_dump_uint(d, val->uint_val);
    }
}

static void bin_dump_float_value(struct ast_bin_dumper* d,
                                 const struct float_value* val) {
    bin_dump_uint(d, val->type);
    bin_dump_float(d, val->val);
}

static void bin_dump_constant(struct ast_bin_dumper* d,
                              const struct constant* constant) {
    bin_dump_ast_node_info(d, &constant->info);
    const uint64_t type = constant->type;
    assert((enum constant_type)type == constant->type);
    bin_dump_uint(d, type);
    switch (constant->type) {
        case CONSTANT_ENUM:
            bin_dump_str(d, &constant->spelling);
            break;
        case CONSTANT_INT:
            bin_dump_int_value(d, &constant->int_val);
            break;
        case CONSTANT_FLOAT:
            bin_dump_float_value(d, &constant->float_val);
            break;
    }
}

static void bin_dump_string_literal(struct ast_bin_dumper* d,
                                    const struct string_literal* lit) {
    bin_dump_ast_node_info(d, &lit->info);
    bin_dump_str(d, &lit->spelling);
}

static void bin_dump_string_constant(struct ast_bin_dumper* d,
                                     const struct string_constant* constant) {
    bin_dump_bool(d, constant->is_func);
    if (constant->is_func) {
        bin_dump_ast_node_info(d, &constant->info);
    } else {
        bin_dump_string_literal(d, &constant->lit);
    }
}

static void bin_dump_unary_expr(struct ast_bin_dumper* d,
                                const struct unary_expr* expr);

static void bin_dump_cond_expr(struct ast_bin_dumper* d,
                               const struct cond_expr* expr);

static void bin_dump_assign_expr(struct ast_bin_dumper* d,
                                 const struct assign_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct unary_and_op* item = &expr->assign_chain[i];
        bin_dump_unary_expr(d, item->unary);
        const uint64_t op = item->op;
        assert((enum assign_expr_op)op == item->op);
        bin_dump_uint(d, op);
    }
    bin_dump_cond_expr(d, expr->value);
}

static void bin_dump_expr(struct ast_bin_dumper* d, const struct expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_assign_expr(d, &expr->assign_exprs[i]);
    }
}

static void bin_dump_generic_assoc(struct ast_bin_dumper* d,
                                   const struct generic_assoc* assoc) {
    bin_dump_ast_node_info(d, &assoc->info);
    const bool has_type_name = assoc->type_name != NULL;
    bin_dump_bool(d, has_type_name);
    if (has_type_name) {
        bin_dump_type_name(d, assoc->type_name);
    }
    bin_dump_assign_expr(d, assoc->assign);
}

static void bin_dump_generic_assoc_list(struct ast_bin_dumper* d,
                                        const struct generic_assoc_list* lst) {
    bin_dump_ast_node_info(d, &lst->info);
    bin_dump_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        bin_dump_generic_assoc(d, &lst->assocs[i]);
    }
}

static void bin_dump_generic_sel(struct ast_bin_dumper* d,
                                 const struct generic_sel* sel) {
    bin_dump_ast_node_info(d, &sel->info);
    bin_dump_assign_expr(d, sel->assign);
    bin_dump_generic_assoc_list(d, &sel->assocs);
}

static void bin_dump_primary_expr(struct ast_bin_dumper* d,
                                  const struct primary_expr* expr) {
    const uint64_t type = expr->type;
    bin_dump_uint(d, type);
    assert((enum primary_expr_type)type == expr->type);
    switch (expr->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            bin_dump_identifier(d, expr->identifier);
            break;
        case PRIMARY_EXPR_CONSTANT:
            bin_dump_constant(d, &expr->constant);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            bin_dump_string_constant(d, &expr->string);
            break;
        case PRIMARY_EXPR_BRACKET:
            bin_dump_expr(d, expr->bracket_expr);
            break;
        case PRIMARY_EXPR_GENERIC:
            bin_dump_generic_sel(d, expr->generic);
            break;
    }
}

static void bin_dump_designation(struct ast_bin_dumper* d,
                                 const struct designation* des) {
    (void)d;
    (void)des;
    // TODO:
}

static void bin_dump_initializer(struct ast_bin_dumper* d,
                                 const struct initializer* init) {
    (void)d;
    (void)init;
    // TODO:
}

static void bin_dump_designation_init(struct ast_bin_dumper* d,
                                      const struct designation_init* init) {
    const bool has_designation = init->designation != NULL;
    bin_dump_bool(d, has_designation);
    if (has_designation) {
        bin_dump_designation(d, init->designation);
    }
    bin_dump_initializer(d, init->init);
}

static void bin_dump_init_list(struct ast_bin_dumper* d,
                               const struct init_list* lst) {
    bin_dump_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        bin_dump_designation_init(d, &lst->inits[i]);
    }
}

static void bin_dump_arg_expr_list(struct ast_bin_dumper* d,
                                   const struct arg_expr_list* lst) {
    (void)d;
    (void)lst;
    // TODO:
}

static void bin_dump_postfix_suffix(struct ast_bin_dumper* d,
                                    const struct postfix_suffix* suffix) {
    const uint64_t type = suffix->type;
    bin_dump_uint(d, type);
    assert((enum postfix_suffix_type)type == suffix->type);

    switch (suffix->type) {
        case POSTFIX_INDEX:
            bin_dump_expr(d, suffix->index_expr);
            break;
        case POSTFIX_BRACKET:
            bin_dump_arg_expr_list(d, &suffix->bracket_list);
            break;
        case POSTFIX_ACCESS:
        case POSTFIX_PTR_ACCESS:
            bin_dump_identifier(d, suffix->identifier);
            break;
        case POSTFIX_INC:
        case POSTFIX_DEC:
            break;
    }
}

static void bin_dump_postfix_expr(struct ast_bin_dumper* d,
                                  const struct postfix_expr* expr) {
    bin_dump_bool(d, expr->is_primary);
    if (expr->is_primary) {
        bin_dump_primary_expr(d, expr->primary);
    } else {
        bin_dump_ast_node_info(d, &expr->info);
        bin_dump_type_name(d, expr->type_name);
        bin_dump_init_list(d, &expr->init_list);
    }
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_postfix_suffix(d, &expr->suffixes[i]);
    }
}

static void bin_dump_cast_expr(struct ast_bin_dumper* d,
                               const struct cast_expr* expr);

static void bin_dump_unary_expr(struct ast_bin_dumper* d,
                                const struct unary_expr* expr) {
    bin_dump_ast_node_info(d, &expr->info);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const uint64_t unary_op = expr->ops_before[i];
        assert((enum unary_expr_op)unary_op == expr->ops_before[i]);
        bin_dump_uint(d, unary_op);
    }
    const uint64_t type = expr->type;
    bin_dump_uint(d, type);
    assert((enum unary_expr_type)type == expr->type);
    switch (expr->type) {
        case UNARY_POSTFIX:
            bin_dump_postfix_expr(d, expr->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            bin_dump_cast_expr(d, expr->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            bin_dump_type_name(d, expr->type_name);
            break;
    }
}

static void bin_dump_cast_expr(struct ast_bin_dumper* d,
                               const struct cast_expr* expr) {
    bin_dump_ast_node_info(d, &expr->info);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_type_name(d, &expr->type_names[i]);
    }
    bin_dump_unary_expr(d, expr->rhs);
}

static void bin_dump_mul_expr(struct ast_bin_dumper* d,
                              const struct mul_expr* expr) {
    bin_dump_cast_expr(d, expr->lhs);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct cast_expr_and_op* item = &expr->mul_chain[i];
        const uint64_t mul_op = item->op;
        assert((enum mul_expr_op)mul_op == item->op);
        bin_dump_uint(d, mul_op);
        bin_dump_cast_expr(d, item->rhs);
    }
}

static void bin_dump_add_expr(struct ast_bin_dumper* d,
                              const struct add_expr* expr) {
    bin_dump_mul_expr(d, expr->lhs);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct mul_expr_and_op* item = &expr->add_chain[i];
        const uint64_t add_op = item->op;
        assert((enum add_expr_op)add_op == item->op);
        bin_dump_uint(d, add_op);
        bin_dump_mul_expr(d, item->rhs);
    }
}

static void bin_dump_shift_expr(struct ast_bin_dumper* d,
                                const struct shift_expr* expr) {
    bin_dump_add_expr(d, expr->lhs);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct add_expr_and_op* item = &expr->shift_chain[i];
        const uint64_t shift_op = item->op;
        assert((enum shift_expr_op)shift_op == item->op);
        bin_dump_uint(d, shift_op);
        bin_dump_add_expr(d, item->rhs);
    }
}

static void bin_dump_rel_expr(struct ast_bin_dumper* d,
                              const struct rel_expr* expr) {
    bin_dump_shift_expr(d, expr->lhs);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct shift_expr_and_op* item = &expr->rel_chain[i];
        const uint64_t rel_op = item->op;
        assert((enum rel_expr_op)rel_op == item->op);
        bin_dump_uint(d, rel_op);
        bin_dump_shift_expr(d, item->rhs);
    }
}

static void bin_dump_eq_expr(struct ast_bin_dumper* d,
                             const struct eq_expr* expr) {
    bin_dump_rel_expr(d, expr->lhs);
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct rel_expr_and_op* item = &expr->eq_chain[i];
        const uint64_t eq_op = item->op;
        assert((enum eq_expr_op)eq_op == item->op);
        bin_dump_uint(d, eq_op);
        bin_dump_rel_expr(d, item->rhs);
    }
}

static void bin_dump_and_expr(struct ast_bin_dumper* d,
                              const struct and_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_eq_expr(d, &expr->eq_exprs[i]);
    }
}

static void bin_dump_xor_expr(struct ast_bin_dumper* d,
                              const struct xor_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_and_expr(d, &expr->and_exprs[i]);
    }
}

static void bin_dump_or_expr(struct ast_bin_dumper* d,
                             const struct or_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_xor_expr(d, &expr->xor_exprs[i]);
    }
}

static void bin_dump_log_and_expr(struct ast_bin_dumper* d,
                                  const struct log_and_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_or_expr(d, &expr->or_exprs[i]);
    }
}

static void bin_dump_log_or_expr(struct ast_bin_dumper* d,
                                 const struct log_or_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_log_and_expr(d, &expr->log_ands[i]);
    }
}

static void bin_dump_cond_expr(struct ast_bin_dumper* d,
                               const struct cond_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct log_or_and_expr* item = &expr->conditionals[i];
        bin_dump_log_or_expr(d, item->log_or);
        bin_dump_expr(d, item->expr);
    }
    bin_dump_log_or_expr(d, expr->last_else);
}

static void bin_dump_const_expr(struct ast_bin_dumper* d,
                                const struct const_expr* expr) {
    bin_dump_cond_expr(d, &expr->expr);
}

static void bin_dump_static_assert_declaration(
    struct ast_bin_dumper* d,
    const struct static_assert_declaration* decl) {
    bin_dump_const_expr(d, decl->const_expr);
    bin_dump_string_literal(d, &decl->err_msg);
}

static void bin_dump_struct_declarator_list(
    struct ast_bin_dumper* d,
    const struct struct_declarator_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_declaration_specs(struct ast_bin_dumper* d,
                                       const struct declaration_specs* specs);

static void bin_dump_struct_declaration(struct ast_bin_dumper* d,
                                        const struct struct_declaration* decl) {
    bin_dump_bool(d, decl->is_static_assert);
    if (decl->is_static_assert) {
        bin_dump_static_assert_declaration(d, decl->assert);
    } else {
        bin_dump_declaration_specs(d, decl->decl_specs);
        bin_dump_struct_declarator_list(d, &decl->decls);
    }
}

static void bin_dump_struct_declaration_list(
    struct ast_bin_dumper* d,
    const struct struct_declaration_list* lst) {
    bin_dump_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        bin_dump_struct_declaration(d, &lst->decls[i]);
    }
}

static void bin_dump_struct_union_spec(struct ast_bin_dumper* d,
                                       const struct struct_union_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_bool(d, spec->is_struct);
    const bool has_identifier = spec->identifier != NULL;
    bin_dump_bool(d, has_identifier);
    if (has_identifier) {
        bin_dump_identifier(d, spec->identifier);
    }
    bin_dump_struct_declaration_list(d, &spec->decl_list);
}

static void bin_dump_enum_spec(struct ast_bin_dumper* d,
                               const struct enum_spec* spec) {
    (void)d;
    (void)spec;
    // TODO:
}

static void bin_dump_type_modifiers(struct ast_bin_dumper* d,
                                    const struct type_modifiers* mods) {
    bin_dump_bool(d, mods->is_unsigned);
    bin_dump_bool(d, mods->is_signed);
    bin_dump_bool(d, mods->is_short);
    bin_dump_uint(d, mods->num_long);
    bin_dump_bool(d, mods->is_complex);
    bin_dump_bool(d, mods->is_imaginary);
}

static void bin_dump_type_specs(struct ast_bin_dumper* d,
                                const struct type_specs* specs) {
    bin_dump_type_modifiers(d, &specs->mods);
    const uint64_t type = specs->type;
    assert((enum type_spec_type)type == specs->type);
    bin_dump_uint(d, type);
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
            bin_dump_atomic_type_spec(d, specs->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            bin_dump_struct_union_spec(d, specs->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            bin_dump_enum_spec(d, specs->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            bin_dump_identifier(d, specs->typedef_name);
            break;
    }
}

static void bin_dump_spec_qual_list(struct ast_bin_dumper* d,
                                    const struct spec_qual_list* lst) {
    bin_dump_ast_node_info(d, &lst->info);
    bin_dump_type_quals(d, &lst->quals);
    bin_dump_type_specs(d, &lst->specs);
}

static void bin_dump_abs_declarator(struct ast_bin_dumper* d,
                                    const struct abs_declarator* decl) {
    (void)d;
    (void)decl;
    // TODO:
}

static void bin_dump_type_name(struct ast_bin_dumper* d,
                               const struct type_name* name) {
    bin_dump_spec_qual_list(d, name->spec_qual_list);
    const bool has_abs_decl = name->abstract_decl != NULL;
    bin_dump_bool(d, has_abs_decl);
    if (has_abs_decl) {
        bin_dump_abs_declarator(d, name->abstract_decl);
    }
}

static void bin_dump_align_spec(struct ast_bin_dumper* d,
                                const struct align_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_bool(d, spec->is_type_name);
    if (spec->is_type_name) {
        bin_dump_type_name(d, spec->type_name);
    } else {
        bin_dump_const_expr(d, spec->const_expr);
    }
}

static void bin_dump_func_specs(struct ast_bin_dumper* d,
                                const struct func_specs* specs) {
    bin_dump_bool(d, specs->is_inline);
    bin_dump_bool(d, specs->is_noreturn);
}

static void bin_dump_storage_class(struct ast_bin_dumper* d,
                                   const struct storage_class* class) {
    bin_dump_bool(d, class->is_typedef);
    bin_dump_bool(d, class->is_extern);
    bin_dump_bool(d, class->is_static);
    bin_dump_bool(d, class->is_thread_local);
    bin_dump_bool(d, class->is_auto);
    bin_dump_bool(d, class->is_register);
}

static void bin_dump_declaration_specs(struct ast_bin_dumper* d,
                                       const struct declaration_specs* specs) {
    bin_dump_ast_node_info(d, &specs->info);
    bin_dump_func_specs(d, &specs->func_specs);
    bin_dump_storage_class(d, &specs->storage_class);
    bin_dump_type_quals(d, &specs->type_quals);
    bin_dump_uint(d, specs->num_align_specs);
    for (size_t i = 0; i < specs->num_align_specs; ++i) {
        bin_dump_align_spec(d, &specs->align_specs[i]);
    }

    bin_dump_type_specs(d, &specs->type_specs);
}

static void bin_dump_declarator(struct ast_bin_dumper* d,
                                const struct declarator* decl) {
    (void)d;
    (void)decl;
    // TODO:
}

static void bin_dump_declaration_list(struct ast_bin_dumper* d,
                                      const struct declaration_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_compound_statement(struct ast_bin_dumper* d,
                                        const struct compound_statement* stat) {
    (void)d;
    (void)stat;
    // TODO:
}

static void bin_dump_func_def(struct ast_bin_dumper* d,
                              const struct func_def* def) {
    bin_dump_declaration_specs(d, def->specs);
    bin_dump_declarator(d, def->decl);
    bin_dump_declaration_list(d, &def->decl_list);
    bin_dump_compound_statement(d, def->comp);
}

static void bin_dump_init_declarator_list(
    struct ast_bin_dumper* d,
    const struct init_declarator_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_declaration(struct ast_bin_dumper* d,
                                 const struct declaration* decl) {
    bin_dump_bool(d, decl->is_normal_decl);
    if (decl->is_normal_decl) {
        bin_dump_declaration_specs(d, decl->decl_specs);
        bin_dump_init_declarator_list(d, &decl->init_decls);
    } else {
        bin_dump_static_assert_declaration(d, decl->static_assert_decl);
    }
}

static void bin_dump_external_declaration(
    struct ast_bin_dumper* d,
    const struct external_declaration* decl) {
    bin_dump_bool(d, decl->is_func_def);
    if (decl->is_func_def) {
        bin_dump_func_def(d, &decl->func_def);
    } else {
        bin_dump_declaration(d, &decl->decl);
    }
}

static void bin_dump_translation_unit(struct ast_bin_dumper* d,
                                      const struct translation_unit* tl) {
    bin_dump_uint(d, tl->len);
    for (size_t i = 0; i < tl->len; ++i) {
        bin_dump_external_declaration(d, &tl->external_decls[i]);
    }
}
