#include "frontend/ast/ast_serializer.h"

#include <setjmp.h>
#include <string.h>

typedef struct {
    jmp_buf err_buf;
    FILE* file;
} AstSerializer;

static void serializer_write(AstSerializer* d,
                             const void* buffer,
                             size_t size,
                             size_t count) {
    if (fwrite(buffer, size, count, d->file) < count) {
        longjmp(d->err_buf, 0);
    }
}

static void serialize_file_info(AstSerializer* d,
                                const FileInfo* info);

static void serialize_translation_unit(AstSerializer* d, const TranslationUnit* tl);

bool serialize_ast(const TranslationUnit* tl, const FileInfo* file_info, FILE* f) {
    AstSerializer d = {
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

static void serialize_bool(AstSerializer* d, bool b) {
    serializer_write(d, &b, sizeof b, 1);
}

static void serialize_uint(AstSerializer* d, uint64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_int(AstSerializer* d, int64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_float(AstSerializer* d, double f) {
    serializer_write(d, &f, sizeof f, 1);
}

static void serialize_str(AstSerializer* d, const Str* str) {
    const size_t len = str_len(str);
    serialize_uint(d, len);
    const char* data = str_get_data(str);
    serializer_write(d, data, sizeof *data, len);
}

static void serialize_file_info(AstSerializer* d,
                                const FileInfo* info) {
    serialize_uint(d, info->len);
    for (size_t i = 0; i < info->len; ++i) {
        serialize_str(d, &info->paths[i]);
    }
}

static void serialize_ast_node_info(AstSerializer* d, const AstNodeInfo* info) {
    serialize_uint(d, info->loc.file_idx);
    serialize_uint(d, info->loc.file_loc.line);
    serialize_uint(d, info->loc.file_loc.index);
}

static void serialize_type_quals(AstSerializer* d, const TypeQuals* quals) {
    serialize_bool(d, quals->is_const);
    serialize_bool(d, quals->is_restrict);
    serialize_bool(d, quals->is_volatile);
    serialize_bool(d, quals->is_atomic);
}

static void serialize_type_name(AstSerializer* d, const TypeName* name);

static void serialize_atomic_type_spec(AstSerializer* d, const AtomicTypeSpec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_type_name(d, spec->type_name);
}

static void serialize_identifier(AstSerializer* d, const Identifier* id) {
    serialize_ast_node_info(d, &id->info);
    serialize_str(d, &id->spelling);
}

static void serialize_int_value(AstSerializer* d,
                                const IntValue* val) {
    serialize_uint(d, val->kind);
    if (int_value_is_signed(val->kind)) {
        serialize_int(d, val->int_val);
    } else {
        serialize_uint(d, val->uint_val);
    }
}

static void serialize_float_value(AstSerializer* d,
                                  const FloatValue* val) {
    serialize_uint(d, val->kind);
    serialize_float(d, val->val);
}

static void serialize_constant(AstSerializer* d, const Constant* constant) {
    serialize_ast_node_info(d, &constant->info);
    const uint64_t kind = constant->kind;
    assert((ConstantKind)kind == constant->kind);
    serialize_uint(d, kind);
    switch (constant->kind) {
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

static void serialize_str_lit(AstSerializer* d,
                              const StrLit* lit) {
    const uint64_t kind = lit->kind;
    assert((StrLitKind)kind == lit->kind);
    serialize_uint(d, kind);
    serialize_str(d, &lit->contents);
}

static void serialize_string_literal_node(
    AstSerializer* d,
    const StringLiteralNode* lit) {
    serialize_ast_node_info(d, &lit->info);
    serialize_str_lit(d, &lit->lit);
}

static void serialize_string_constant(AstSerializer* d, const StringConstant* constant) {
    serialize_bool(d, constant->is_func);
    if (constant->is_func) {
        serialize_ast_node_info(d, &constant->info);
    } else {
        serialize_string_literal_node(d, &constant->lit);
    }
}

static void serialize_unary_expr(AstSerializer* d, const UnaryExpr* expr);

static void serialize_cond_expr(AstSerializer* d,
                                const CondExpr* expr);

static void serialize_assign_expr(AstSerializer* d, const AssignExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const UnaryAndOp* item = &expr->assign_chain[i];
        serialize_unary_expr(d, item->unary);
        const uint64_t op = item->op;
        assert((AssignExprOp)op == item->op);
        serialize_uint(d, op);
    }
    serialize_cond_expr(d, expr->value);
}

static void serialize_expr(AstSerializer* d, const Expr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_assign_expr(d, &expr->assign_exprs[i]);
    }
}

static void serialize_generic_assoc(AstSerializer* d, const GenericAssoc* assoc) {
    serialize_ast_node_info(d, &assoc->info);
    const bool has_type_name = assoc->type_name != NULL;
    serialize_bool(d, has_type_name);
    if (has_type_name) {
        serialize_type_name(d, assoc->type_name);
    }
    serialize_assign_expr(d, assoc->assign);
}

static void serialize_generic_assoc_list(AstSerializer* d, const GenericAssocList* lst) {
    serialize_ast_node_info(d, &lst->info);
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_generic_assoc(d, &lst->assocs[i]);
    }
}

static void serialize_generic_sel(AstSerializer* d, const GenericSel* sel) {
    serialize_ast_node_info(d, &sel->info);
    serialize_assign_expr(d, sel->assign);
    serialize_generic_assoc_list(d, &sel->assocs);
}

static void serialize_primary_expr(AstSerializer* d, const PrimaryExpr* expr) {
    const uint64_t kind = expr->kind;
    serialize_uint(d, kind);
    assert((PrimaryExprKind)kind == expr->kind);
    switch (expr->kind) {
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

static void serialize_const_expr(AstSerializer* d, const ConstExpr* expr);

static void serialize_designator(AstSerializer* d,
                                 const struct Designator* des) {
    serialize_ast_node_info(d, &des->info);
    serialize_bool(d, des->is_index);
    if (des->is_index) {
        serialize_const_expr(d, des->arr_index);
    } else {
        serialize_identifier(d, des->identifier);
    }
}

static void serialize_designator_list(AstSerializer* d,
                                      const DesignatorList* des) {
    serialize_uint(d, des->len);
    for (size_t i = 0; i < des->len; ++i) {
        serialize_designator(d, &des->designators[i]);
    }
}

static void serialize_designation(AstSerializer* d,
                                  const Designation* des) {
    serialize_designator_list(d, &des->designators);
}

static void serialize_init_list(AstSerializer* d,
                                const InitList* lst);

static void serialize_initializer(AstSerializer* d,
                                  const Initializer* init) {
    serialize_ast_node_info(d, &init->info);
    serialize_bool(d, init->is_assign);
    if (init->is_assign) {
        serialize_assign_expr(d, init->assign);
    } else {
        serialize_init_list(d, &init->init_list);
    }
}

static void serialize_designation_init(AstSerializer* d,
                                       const DesignationInit* init) {
    const bool has_designation = is_valid_designation(&init->designation);
    serialize_bool(d, has_designation);
    if (has_designation) {
        serialize_designation(d, &init->designation);
    }
    serialize_initializer(d, &init->init);
}

static void serialize_init_list(AstSerializer* d,
                                const InitList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_designation_init(d, &lst->inits[i]);
    }
}

static void serialize_arg_expr_list(AstSerializer* d, const ArgExprList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_assign_expr(d, &lst->assign_exprs[i]);
    }
}

static void serialize_postfix_suffix(AstSerializer* d, const PostfixSuffix* suffix) {
    const uint64_t kind = suffix->kind;
    serialize_uint(d, kind);
    assert((PostfixSuffixKind)kind == suffix->kind);

    switch (suffix->kind) {
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

static void serialize_postfix_expr(AstSerializer* d, const PostfixExpr* expr) {
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

static void serialize_cast_expr(AstSerializer* d, const CastExpr* expr);

static void serialize_unary_expr(AstSerializer* d, const UnaryExpr* expr) {
    serialize_ast_node_info(d, &expr->info);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const uint64_t unary_op = expr->ops_before[i];
        assert((UnaryExprOp)unary_op == expr->ops_before[i]);
        serialize_uint(d, unary_op);
    }
    const uint64_t kind = expr->kind;
    serialize_uint(d, kind);
    assert((UnaryExprKind)kind == expr->kind);
    switch (expr->kind) {
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

static void serialize_cast_expr(AstSerializer* d, const CastExpr* expr) {
    serialize_ast_node_info(d, &expr->info);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_type_name(d, &expr->type_names[i]);
    }
    serialize_unary_expr(d, expr->rhs);
}

static void serialize_mul_expr(AstSerializer* d, const MulExpr* expr) {
    serialize_cast_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const CastExprAndOp* item = &expr->mul_chain[i];
        const uint64_t mul_op = item->op;
        assert((MulExprOp)mul_op == item->op);
        serialize_uint(d, mul_op);
        serialize_cast_expr(d, item->rhs);
    }
}

static void serialize_add_expr(AstSerializer* d, const AddExpr* expr) {
    serialize_mul_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const MulExprAndOp* item = &expr->add_chain[i];
        const uint64_t add_op = item->op;
        assert((AddExprOp)add_op == item->op);
        serialize_uint(d, add_op);
        serialize_mul_expr(d, item->rhs);
    }
}

static void serialize_shift_expr(AstSerializer* d, const ShiftExpr* expr) {
    serialize_add_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const AddExprAndOp* item = &expr->shift_chain[i];
        const uint64_t shift_op = item->op;
        assert((ShiftExprOp)shift_op == item->op);
        serialize_uint(d, shift_op);
        serialize_add_expr(d, item->rhs);
    }
}

static void serialize_rel_expr(AstSerializer* d, const RelExpr* expr) {
    serialize_shift_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const ShiftExprAndOp* item = &expr->rel_chain[i];
        const uint64_t rel_op = item->op;
        assert((RelExprOp)rel_op == item->op);
        serialize_uint(d, rel_op);
        serialize_shift_expr(d, item->rhs);
    }
}

static void serialize_eq_expr(AstSerializer* d, const EqExpr* expr) {
    serialize_rel_expr(d, expr->lhs);
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const RelExprAndOp* item = &expr->eq_chain[i];
        const uint64_t eq_op = item->op;
        assert((EqExprOp)eq_op == item->op);
        serialize_uint(d, eq_op);
        serialize_rel_expr(d, item->rhs);
    }
}

static void serialize_and_expr(AstSerializer* d, const AndExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_eq_expr(d, &expr->eq_exprs[i]);
    }
}

static void serialize_xor_expr(AstSerializer* d, const XorExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_and_expr(d, &expr->and_exprs[i]);
    }
}

static void serialize_or_expr(AstSerializer* d, const OrExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_xor_expr(d, &expr->xor_exprs[i]);
    }
}

static void serialize_log_and_expr(AstSerializer* d, const LogAndExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_or_expr(d, &expr->or_exprs[i]);
    }
}

static void serialize_log_or_expr(AstSerializer* d, const LogOrExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        serialize_log_and_expr(d, &expr->log_ands[i]);
    }
}

static void serialize_cond_expr(AstSerializer* d,
                                const CondExpr* expr) {
    serialize_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const LogOrAndExpr* item = &expr->conditionals[i];
        serialize_log_or_expr(d, item->log_or);
        serialize_expr(d, item->expr);
    }
    serialize_log_or_expr(d, expr->last_else);
}

static void serialize_const_expr(AstSerializer* d, const ConstExpr* expr) {
    serialize_cond_expr(d, &expr->expr);
}

static void serialize_static_assert_declaration(AstSerializer* d, const StaticAssertDeclaration* decl) {
    serialize_const_expr(d, decl->const_expr);
    serialize_string_literal_node(d, &decl->err_msg);
}

static void serialize_pointer(AstSerializer* d, const Pointer* ptr) {
    serialize_ast_node_info(d, &ptr->info);
    serialize_uint(d, ptr->num_indirs);
    for (size_t i = 0; i < ptr->num_indirs; ++i) {
        serialize_type_quals(d, &ptr->quals_after_ptr[i]);
    }
}

static void serialize_param_type_list(AstSerializer* d, const ParamTypeList* lst);

static void serialize_abs_arr_or_func_suffix(AstSerializer* d, const AbsArrOrFuncSuffix* suffix) {
    serialize_ast_node_info(d, &suffix->info);
    const uint64_t kind = suffix->kind;
    assert((AbsArrOrFuncSuffixKind)kind == suffix->kind);
    serialize_uint(d, kind);
    switch (suffix->kind) {
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

static void serialize_abs_declarator(AstSerializer* d, const AbsDeclarator* decl);

static void serialize_direct_abs_declarator(AstSerializer* d, const DirectAbsDeclarator* decl) {
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

static void serialize_abs_declarator(AstSerializer* d, const AbsDeclarator* decl) {
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

static void serialize_declaration_specs(AstSerializer* d,
                                        const DeclarationSpecs* specs);

static void serialize_declarator(AstSerializer* d, const Declarator* decl);

static void serialize_param_declaration(AstSerializer* d, const ParamDeclaration* decl) {
    serialize_declaration_specs(d, decl->decl_specs);
    const uint64_t kind = decl->kind;
    assert((ParamDeclKind)kind == decl->kind);
    serialize_uint(d, kind);
    switch (decl->kind) {
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

static void serialize_param_list(AstSerializer* d,
                                 const ParamList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_param_declaration(d, &lst->decls[i]);
    }
}

static void serialize_param_type_list(AstSerializer* d, const ParamTypeList* lst) {
    serialize_bool(d, lst->is_variadic);
    serialize_param_list(d, &lst->param_list);
}

static void serialize_identifier_list(AstSerializer* d, const IdentifierList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_identifier(d, &lst->identifiers[i]);
    }
}

static void serialize_arr_suffix(AstSerializer* d, const ArrSuffix* suffix) {
    serialize_bool(d, suffix->is_static);
    serialize_type_quals(d, &suffix->type_quals);
    serialize_bool(d, suffix->is_asterisk);
    const bool has_assign_expr = suffix->arr_len != NULL;
    serialize_bool(d, has_assign_expr);
    if (has_assign_expr) {
        serialize_assign_expr(d, suffix->arr_len);
    }
}

static void serialize_arr_or_func_suffix(AstSerializer* d, const ArrOrFuncSuffix* suffix) {
    serialize_ast_node_info(d, &suffix->info);
    const uint64_t kind = suffix->kind;
    assert((ArrOrFuncSuffixKind)kind == suffix->kind);
    serialize_uint(d, kind);
    switch (suffix->kind) {
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

static void serialize_direct_declarator(AstSerializer* d, const DirectDeclarator* decl) {
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

static void serialize_declarator(AstSerializer* d, const Declarator* decl) {
    const bool has_ptr = decl->ptr != NULL;
    serialize_bool(d, has_ptr);
    if (has_ptr) {
        serialize_pointer(d, decl->ptr);
    }
    serialize_direct_declarator(d, decl->direct_decl);
}

static void serialize_struct_declarator(AstSerializer* d, const StructDeclarator* decl) {
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

static void serialize_struct_declarator_list(AstSerializer* d, const StructDeclaratorList* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_struct_declarator(d, &decls->decls[i]);
    }
}

static void serialize_struct_declaration(AstSerializer* d, const StructDeclaration* decl) {
    serialize_bool(d, decl->is_static_assert);
    if (decl->is_static_assert) {
        serialize_static_assert_declaration(d, decl->assert);
    } else {
        serialize_declaration_specs(d, decl->decl_specs);
        serialize_struct_declarator_list(d, &decl->decls);
    }
}

static void serialize_struct_declaration_list(AstSerializer* d, const StructDeclarationList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_struct_declaration(d, &lst->decls[i]);
    }
}

static void serialize_struct_union_spec(AstSerializer* d, const StructUnionSpec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_bool(d, spec->is_struct);
    const bool has_identifier = spec->identifier != NULL;
    serialize_bool(d, has_identifier);
    if (has_identifier) {
        serialize_identifier(d, spec->identifier);
    }
    serialize_struct_declaration_list(d, &spec->decl_list);
}

static void serialize_enumerator(AstSerializer* d, const Enumerator* enumerator) {
    serialize_identifier(d, enumerator->identifier);
    const bool has_enum_val = enumerator->enum_val != NULL;
    serialize_bool(d, has_enum_val);
    if (has_enum_val) {
        serialize_const_expr(d, enumerator->enum_val);
    }
}

static void serialize_enum_list(AstSerializer* d, const EnumList* lst) {
    serialize_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        serialize_enumerator(d, &lst->enums[i]);
    }
}

static void serialize_enum_spec(AstSerializer* d, const EnumSpec* spec) {
    serialize_ast_node_info(d, &spec->info);
    const bool has_identifier = spec->identifier != NULL;
    serialize_bool(d, has_identifier);
    if (has_identifier) {
        serialize_identifier(d, spec->identifier);
    }
    serialize_enum_list(d, &spec->enum_list);
}

static void serialize_type_modifiers(AstSerializer* d, const TypeModifiers* mods) {
    serialize_bool(d, mods->is_unsigned);
    serialize_bool(d, mods->is_signed);
    serialize_bool(d, mods->is_short);
    serialize_uint(d, mods->num_long);
    serialize_bool(d, mods->is_complex);
    serialize_bool(d, mods->is_imaginary);
}

static void serialize_type_specs(AstSerializer* d, const TypeSpecs* specs) {
    serialize_type_modifiers(d, &specs->mods);
    const uint64_t kind = specs->kind;
    assert((TypeSpecKind)kind == specs->kind);
    serialize_uint(d, kind);
    switch (specs->kind) {
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

static void serialize_spec_qual_list(AstSerializer* d, const SpecQualList* lst) {
    serialize_ast_node_info(d, &lst->info);
    serialize_type_quals(d, &lst->quals);
    serialize_type_specs(d, &lst->specs);
}

static void serialize_type_name(AstSerializer* d, const TypeName* name) {
    serialize_spec_qual_list(d, name->spec_qual_list);
    const bool has_abs_decl = name->abstract_decl != NULL;
    serialize_bool(d, has_abs_decl);
    if (has_abs_decl) {
        serialize_abs_declarator(d, name->abstract_decl);
    }
}

static void serialize_align_spec(AstSerializer* d, const AlignSpec* spec) {
    serialize_ast_node_info(d, &spec->info);
    serialize_bool(d, spec->is_type_name);
    if (spec->is_type_name) {
        serialize_type_name(d, spec->type_name);
    } else {
        serialize_const_expr(d, spec->const_expr);
    }
}

static void serialize_func_specs(AstSerializer* d, const FuncSpecs* specs) {
    serialize_bool(d, specs->is_inline);
    serialize_bool(d, specs->is_noreturn);
}

static void serialize_storage_class(AstSerializer* d, const StorageClass* class) {
    serialize_bool(d, class->is_typedef);
    serialize_bool(d, class->is_extern);
    serialize_bool(d, class->is_static);
    serialize_bool(d, class->is_thread_local);
    serialize_bool(d, class->is_auto);
    serialize_bool(d, class->is_register);
}

static void serialize_declaration_specs(AstSerializer* d, const DeclarationSpecs* specs) {
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

static void serialize_declaration(AstSerializer* d, const Declaration* decl);

static void serialize_declaration_list(AstSerializer* d, const DeclarationList* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_declaration(d, &decls->decls[i]);
    }
}

static void serialize_statement(AstSerializer* d, const Statement* stat);

static void serialize_labeled_statement(AstSerializer* d, const LabeledStatement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t kind = stat->kind;
    assert((LabeledStatementKind)kind == stat->kind);
    serialize_uint(d, kind);
    switch (stat->kind) {
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

static void serialize_expr_statement(AstSerializer* d, const ExprStatement* stat) {
    serialize_ast_node_info(d, &stat->info);
    serialize_expr(d, &stat->expr);
}

static void serialize_selection_statement(AstSerializer* d, const SelectionStatement* stat) {
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

static void serialize_for_loop(AstSerializer* d, const ForLoop* loop) {
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
    AstSerializer* d,
    const struct IterationStatement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t kind = stat->kind;
    assert((IterationStatementKind)kind == stat->kind);
    serialize_uint(d, kind);
    serialize_statement(d, stat->loop_body);
    switch (stat->kind) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            serialize_expr(d, stat->while_cond);
            break;
        case ITERATION_STATEMENT_FOR:
            serialize_for_loop(d, &stat->for_loop);
            break;
    }
}

static void serialize_jump_statement(AstSerializer* d, const JumpStatement* stat) {
    serialize_ast_node_info(d, &stat->info);
    const uint64_t kind = stat->kind;
    assert((JumpStatementKind)kind == stat->kind);
    serialize_uint(d, kind);
    switch (stat->kind) {
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

static void serialize_compound_statement(AstSerializer* d, const CompoundStatement* stat);

static void serialize_statement(AstSerializer* d, const Statement* stat) {
    const uint64_t kind = stat->kind;
    assert((StatementKind)kind == stat->kind);
    serialize_uint(d, kind);
    switch (stat->kind) {
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

static void serialize_block_item(AstSerializer* d, const BlockItem* item) {
    serialize_bool(d, item->is_decl);
    if (item->is_decl) {
        serialize_declaration(d, &item->decl);
    } else {
        serialize_statement(d, &item->stat);
    }
}

static void serialize_compound_statement(AstSerializer* d, const CompoundStatement* stat) {
    serialize_ast_node_info(d, &stat->info);
    serialize_uint(d, stat->len);
    for (size_t i = 0; i < stat->len; ++i) {
        serialize_block_item(d, &stat->items[i]);
    }
}

static void serialize_func_def(AstSerializer* d, const FuncDef* def) {
    serialize_declaration_specs(d, def->specs);
    serialize_declarator(d, def->decl);
    serialize_declaration_list(d, &def->decl_list);
    serialize_compound_statement(d, &def->comp);
}

static void serialize_init_declarator(AstSerializer* d, const InitDeclarator* decl) {
    serialize_declarator(d, decl->decl);
    const bool has_init = decl->init != NULL;
    serialize_bool(d, has_init);
    if (has_init) {
        serialize_initializer(d, decl->init);
    }
}

static void serialize_init_declarator_list(AstSerializer* d, const InitDeclaratorList* decls) {
    serialize_uint(d, decls->len);
    for (size_t i = 0; i < decls->len; ++i) {
        serialize_init_declarator(d, &decls->decls[i]);
    }
}

static void serialize_declaration(AstSerializer* d, const Declaration* decl) {
    serialize_bool(d, decl->is_normal_decl);
    if (decl->is_normal_decl) {
        serialize_declaration_specs(d, decl->decl_specs);
        serialize_init_declarator_list(d, &decl->init_decls);
    } else {
        serialize_static_assert_declaration(d, decl->static_assert_decl);
    }
}

static void serialize_external_declaration(AstSerializer* d, const ExternalDeclaration* decl) {
    serialize_bool(d, decl->is_func_def);
    if (decl->is_func_def) {
        serialize_func_def(d, &decl->func_def);
    } else {
        serialize_declaration(d, &decl->decl);
    }
}

static void serialize_translation_unit(AstSerializer* d, const TranslationUnit* tl) {
    serialize_uint(d, tl->len);
    for (size_t i = 0; i < tl->len; ++i) {
        serialize_external_declaration(d, &tl->external_decls[i]);
    }
}
