#include "frontend/ast/compare_asts.h"

#include <string.h>

#include "util/macro_util.h"

// TODO: maybe assert is not the best name (especially because there already is
// an assert in testing)
#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            return false;                                                      \
        }                                                                      \
    } while (0)

#define COMPARE_NULLABLE(item1, item2, comp)                                   \
    do {                                                                       \
        if (item1 == NULL) {                                                   \
            ASSERT(item2 == NULL);                                             \
        } else if (item2 == NULL) {                                            \
            return false;                                                      \
        } else {                                                               \
            ASSERT(comp(item1, item2));                                        \
        }                                                                      \
    } while (0)

static bool compare_translation_units(const TranslationUnit* tl1,
                                      const TranslationUnit* tl2);

static bool compare_file_infos(const FileInfo* i1, const FileInfo* i2);

bool compare_asts(const TranslationUnit* tl1,
                  const FileInfo* i1,
                  const TranslationUnit* tl2,
                  const FileInfo* i2) {
    return compare_file_infos(i1, i2) && compare_translation_units(tl1, tl2);
}

static bool compare_ast_node_infos(const AstNodeInfo* i1,
                                   const AstNodeInfo* i2) {
    ASSERT(i1->loc.file_idx == i2->loc.file_idx);
    ASSERT(i1->loc.file_loc.line == i2->loc.file_loc.line);
    return i1->loc.file_loc.index == i2->loc.file_loc.index;
}

static bool compare_strs(const Str* s1, const Str* s2) {
    ASSERT(Str_len(s1) == Str_len(s2));
    return strcmp(Str_get_data(s1), Str_get_data(s2)) == 0;
}

static bool compare_file_infos(const FileInfo* i1, const FileInfo* i2) {
    ASSERT(i1->len == i2->len);
    for (size_t i = 0; i < i1->len; ++i) {
        ASSERT(compare_strs(&i1->paths[i], &i2->paths[i]));
    }
    return true;
}

static bool compare_type_quals(const TypeQuals* q1, const TypeQuals* q2) {
    ASSERT(q1->is_const == q2->is_const);
    ASSERT(q1->is_restrict == q2->is_restrict);
    ASSERT(q1->is_volatile == q2->is_volatile);
    return q1->is_atomic == q2->is_atomic;
}

static bool compare_type_names(const TypeName* t1, const TypeName* t2);

static bool compare_atomic_type_specs(const AtomicTypeSpec* s1,
                                      const AtomicTypeSpec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    return compare_type_names(s1->type_name, s2->type_name);
}

static bool compare_identifiers(const Identifier* i1, const Identifier* i2) {
    ASSERT(compare_ast_node_infos(&i1->info, &i2->info));
    return compare_strs(&i1->spelling, &i2->spelling);
}

static bool compare_values(const Value* v1, const Value* v2) {
    ASSERT(v1->kind == v2->kind);
    if (ValueKind_is_sint(v1->kind)) {
        return v1->sint_val == v2->sint_val;
    } else if (ValueKind_is_uint(v1->kind)) {
        return v1->uint_val == v2->uint_val; 
    } else {
        return v1->float_val == v2->float_val;
    }
}

static bool compare_constants(const Constant* c1, const Constant* c2) {
    ASSERT(compare_ast_node_infos(&c1->info, &c2->info));
    ASSERT(c1->kind == c2->kind);
    switch (c1->kind) {
        case CONSTANT_ENUM:
            return compare_strs(&c1->spelling, &c2->spelling);
        case CONSTANT_VAL:
            return compare_values(&c1->val, &c2->val);
    }
    UNREACHABLE();
}

static bool compare_str_lits(const StrLit* l1, const StrLit* l2) {
    ASSERT(l1->kind == l2->kind);
    return compare_strs(&l1->contents, &l2->contents);
}

static bool compare_string_literal_nodes(const StringLiteralNode* l1,
                                         const StringLiteralNode* l2) {
    ASSERT(compare_ast_node_infos(&l1->info, &l2->info));
    return compare_str_lits(&l1->lit, &l2->lit);
}

static bool compare_string_constants(const StringConstant* c1,
                                     const StringConstant* c2) {
    ASSERT(c1->is_func == c2->is_func);
    if (c1->is_func) {
        return compare_ast_node_infos(&c1->info, &c2->info);
    } else {
        return compare_string_literal_nodes(&c1->lit, &c2->lit);
    }
}

static bool compare_unary_exprs(const UnaryExpr* e1, const UnaryExpr* e2);
static bool compare_cond_exprs(const CondExpr* e1, const CondExpr* e2);

static bool compare_assign_exprs(const AssignExpr* e1, const AssignExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        UnaryAndOp* item1 = &e1->assign_chain[i];
        UnaryAndOp* item2 = &e2->assign_chain[i];
        ASSERT(compare_unary_exprs(item1->unary, item2->unary));
        ASSERT(item1->op == item2->op);
    }
    return compare_cond_exprs(e1->value, e2->value);
}

static bool compare_generic_assocs(const GenericAssoc* a1,
                                   const GenericAssoc* a2) {
    ASSERT(compare_ast_node_infos(&a1->info, &a2->info));
    COMPARE_NULLABLE(a1->type_name, a2->type_name, compare_type_names);
    return compare_assign_exprs(a1->assign, a2->assign);
}

static bool compare_generic_assoc_lists(const GenericAssocList* l1,
                                        const GenericAssocList* l2) {
    ASSERT(compare_ast_node_infos(&l1->info, &l2->info));
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_generic_assocs(&l1->assocs[i], &l2->assocs[i]));
    }
    return true;
}

static bool compare_generic_sel(const GenericSel* s1, const GenericSel* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(compare_assign_exprs(s1->assign, s2->assign));
    return compare_generic_assoc_lists(&s1->assocs, &s2->assocs);
}

static bool compare_exprs(const Expr* e1, const Expr* e2);

static bool compare_primary_exprs(const PrimaryExpr* e1,
                                  const PrimaryExpr* e2) {
    ASSERT(e1->kind == e2->kind);
    switch (e1->kind) {
        case PRIMARY_EXPR_IDENTIFIER:
            return compare_identifiers(e1->identifier, e2->identifier);
        case PRIMARY_EXPR_CONSTANT:
            return compare_constants(&e1->constant, &e2->constant);
        case PRIMARY_EXPR_STRING_LITERAL:
            return compare_string_constants(&e1->string, &e2->string);
        case PRIMARY_EXPR_BRACKET:
            return compare_exprs(e1->bracket_expr, e2->bracket_expr);
        case PRIMARY_EXPR_GENERIC:
            return compare_generic_sel(e1->generic, e2->generic);
    }
    UNREACHABLE();
}

static bool compare_const_exprs(const ConstExpr* e1, const ConstExpr* e2);

static bool compare_designator(const struct Designator* d1,
                               const struct Designator* d2) {
    ASSERT(compare_ast_node_infos(&d1->info, &d2->info));
    ASSERT(d1->is_index == d2->is_index);
    if (d1->is_index) {
        return compare_const_exprs(d1->arr_index, d2->arr_index);
    } else {
        return compare_identifiers(d1->identifier, d2->identifier);
    }
}

static bool compare_designator_list(const DesignatorList* l1,
                                    const DesignatorList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_designator(&l1->designators[i], &l2->designators[i]));
    }
    return true;
}

static bool compare_designations(const Designation* d1, const Designation* d2) {
    return compare_designator_list(&d1->designators, &d2->designators);
}

static bool compare_init_list(const InitList* l1, const InitList* l2);

static bool compare_initializers(const Initializer* i1, const Initializer* i2) {
    ASSERT(compare_ast_node_infos(&i1->info, &i2->info));
    ASSERT(i1->is_assign == i2->is_assign);
    if (i1->is_assign) {
        return compare_assign_exprs(i1->assign, i2->assign);
    } else {
        return compare_init_list(&i1->init_list, &i2->init_list);
    }
}

static bool compare_designation_inits(const DesignationInit* i1,
                                      const DesignationInit* i2) {
    ASSERT(compare_designations(&i1->designation, &i2->designation));
    return compare_initializers(&i1->init, &i2->init);
}

static bool compare_init_list(const InitList* l1, const InitList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_designation_inits(&l1->inits[i], &l2->inits[i]));
    }
    return true;
}

static bool compare_arg_expr_lists(const ArgExprList* l1,
                                   const ArgExprList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(
            compare_assign_exprs(&l1->assign_exprs[i], &l2->assign_exprs[i]));
    }
    return true;
}

static bool compare_postfix_suffixes(const PostfixSuffix* s1,
                                     const PostfixSuffix* s2) {
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case POSTFIX_INDEX:
            return compare_exprs(s1->index_expr, s2->index_expr);
        case POSTFIX_BRACKET:
            return compare_arg_expr_lists(&s1->bracket_list, &s2->bracket_list);
        case POSTFIX_ACCESS:
        case POSTFIX_PTR_ACCESS:
            return compare_identifiers(s1->identifier, s2->identifier);
        case POSTFIX_INC:
        case POSTFIX_DEC:
            return true;
    }
    UNREACHABLE();
}

static bool compare_postfix_exprs(const PostfixExpr* e1,
                                  const PostfixExpr* e2) {
    ASSERT(e1->is_primary == e2->is_primary);
    if (e1->is_primary) {
        ASSERT(compare_primary_exprs(e1->primary, e2->primary));
    } else {
        ASSERT(compare_ast_node_infos(&e1->info, &e2->info));
        ASSERT(compare_type_names(e1->type_name, e2->type_name));
        ASSERT(compare_init_list(&e1->init_list, &e2->init_list));
    }
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_postfix_suffixes(&e1->suffixes[i], &e2->suffixes[i]));
    }
    return true;
}

static bool compare_cast_exprs(const CastExpr* e1, const CastExpr* e2);

static bool compare_unary_exprs(const UnaryExpr* e1, const UnaryExpr* e2) {
    ASSERT(compare_ast_node_infos(&e1->info, &e2->info));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(e1->ops_before[i] == e2->ops_before[i]);
    }
    ASSERT(e1->kind == e2->kind);
    switch (e1->kind) {
        case UNARY_POSTFIX:
            return compare_postfix_exprs(e1->postfix, e2->postfix);
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            return compare_cast_exprs(e1->cast_expr, e2->cast_expr);
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            return compare_type_names(e1->type_name, e2->type_name);
    }
    UNREACHABLE();
}

static bool compare_cast_exprs(const CastExpr* e1, const CastExpr* e2) {
    ASSERT(compare_ast_node_infos(&e1->info, &e2->info));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_type_names(&e1->type_names[i], &e2->type_names[i]));
    }
    return compare_unary_exprs(e1->rhs, e2->rhs);
}

static bool compare_mul_exprs(const MulExpr* e1, const MulExpr* e2) {
    ASSERT(compare_cast_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const CastExprAndOp* item1 = &e1->mul_chain[i];
        const CastExprAndOp* item2 = &e2->mul_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_cast_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_add_exprs(const AddExpr* e1, const AddExpr* e2) {
    ASSERT(compare_mul_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const MulExprAndOp* item1 = &e1->add_chain[i];
        const MulExprAndOp* item2 = &e2->add_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_mul_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_shift_exprs(const ShiftExpr* e1, const ShiftExpr* e2) {
    ASSERT(compare_add_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const AddExprAndOp* item1 = &e1->shift_chain[i];
        const AddExprAndOp* item2 = &e2->shift_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_add_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_rel_exprs(const RelExpr* e1, const RelExpr* e2) {
    ASSERT(compare_shift_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const ShiftExprAndOp* item1 = &e1->rel_chain[i];
        const ShiftExprAndOp* item2 = &e2->rel_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_shift_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_eq_exprs(const EqExpr* e1, const EqExpr* e2) {
    ASSERT(compare_rel_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const RelExprAndOp* item1 = &e1->eq_chain[i];
        const RelExprAndOp* item2 = &e2->eq_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_rel_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_and_exprs(const AndExpr* e1, const AndExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_eq_exprs(&e1->eq_exprs[i], &e2->eq_exprs[i]));
    }
    return true;
}

static bool compare_xor_exprs(const XorExpr* e1, const XorExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_and_exprs(&e1->and_exprs[i], &e2->and_exprs[i]));
    }
    return true;
}

static bool compare_or_exprs(const OrExpr* e1, const OrExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_xor_exprs(&e1->xor_exprs[i], &e2->xor_exprs[i]));
    }
    return true;
}

static bool compare_log_and_exprs(const LogAndExpr* e1, const LogAndExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_or_exprs(&e1->or_exprs[i], &e2->or_exprs[i]));
    }
    return true;
}

static bool compare_log_or_exprs(const LogOrExpr* e1, const LogOrExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_log_and_exprs(&e1->log_ands[i], &e2->log_ands[i]));
    }
    return true;
}

static bool compare_exprs(const Expr* e1, const Expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(
            compare_assign_exprs(&e1->assign_exprs[i], &e2->assign_exprs[i]));
    }
    return true;
}

static bool compare_cond_exprs(const struct CondExpr* e1,
                               const struct CondExpr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const LogOrAndExpr* item1 = &e1->conditionals[i];
        const LogOrAndExpr* item2 = &e2->conditionals[i];
        ASSERT(compare_log_or_exprs(item1->log_or, item2->log_or));
        ASSERT(compare_exprs(item1->expr, item2->expr));
    }
    return compare_log_or_exprs(e1->last_else, e2->last_else);
}

static bool compare_const_exprs(const ConstExpr* e1, const ConstExpr* e2) {
    return compare_cond_exprs(&e1->expr, &e2->expr);
}

static bool compare_static_assert_declarations(
    const StaticAssertDeclaration* d1,
    const StaticAssertDeclaration* d2) {
    ASSERT(compare_const_exprs(d1->const_expr, d2->const_expr));
    return compare_string_literal_nodes(&d1->err_msg, &d2->err_msg);
}

static bool compare_pointers(const Pointer* p1, const Pointer* p2) {
    ASSERT(compare_ast_node_infos(&p1->info, &p2->info));
    ASSERT(p1->num_indirs == p2->num_indirs);
    for (size_t i = 0; i < p1->num_indirs; ++i) {
        ASSERT(compare_type_quals(&p1->quals_after_ptr[i],
                                  &p1->quals_after_ptr[i]));
    }
    return true;
}

static bool compare_param_type_lists(const ParamTypeList* l1,
                                     const ParamTypeList* l2);

static bool compare_abs_arr_or_func_suffix(const AbsArrOrFuncSuffix* s1,
                                           const AbsArrOrFuncSuffix* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            return s1->has_asterisk == s2->has_asterisk;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            ASSERT(s1->is_static == s2->is_static);
            ASSERT(compare_type_quals(&s1->type_quals, &s2->type_quals));
            COMPARE_NULLABLE(s1->assign, s2->assign, compare_assign_exprs);
            return true;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            return compare_param_type_lists(&s1->func_types, &s2->func_types);
    }
    UNREACHABLE();
}

static bool compare_abs_declarators(const AbsDeclarator* d1,
                                    const AbsDeclarator* d2);

static bool compare_direct_abs_declarators(
    const struct DirectAbsDeclarator* d1,
    const struct DirectAbsDeclarator* d2) {
    ASSERT(compare_ast_node_infos(&d1->info, &d2->info));
    COMPARE_NULLABLE(d1->bracket_decl,
                     d2->bracket_decl,
                     compare_abs_declarators);
    ASSERT(d1->len == d2->len);
    for (size_t i = 0; i < d1->len; ++i) {
        ASSERT(compare_abs_arr_or_func_suffix(&d1->following_suffixes[i],
                                              &d2->following_suffixes[i]));
    }
    return true;
}

static bool compare_abs_declarators(const AbsDeclarator* d1,
                                    const AbsDeclarator* d2) {
    COMPARE_NULLABLE(d1->ptr, d2->ptr, compare_pointers);
    COMPARE_NULLABLE(d1->direct_abs_decl,
                     d2->direct_abs_decl,
                     compare_direct_abs_declarators);
    return true;
}

static bool compare_declaration_specs(const DeclarationSpecs* s1,
                                      const DeclarationSpecs* s2);

static bool compare_declarators(const Declarator* d1, const Declarator* d2);

static bool compare_param_declarations(const ParamDeclaration* d1,
                                       const ParamDeclaration* d2) {
    ASSERT(compare_declaration_specs(d1->decl_specs, d2->decl_specs));
    ASSERT(d1->kind == d2->kind);
    switch (d1->kind) {
        case PARAM_DECL_DECL:
            return compare_declarators(d1->decl, d2->decl);
        case PARAM_DECL_ABSTRACT_DECL:
            return compare_abs_declarators(d1->abstract_decl,
                                           d2->abstract_decl);
        case PARAM_DECL_NONE:
            return true;
    }
    UNREACHABLE();
}

static bool compare_param_lists(const ParamList* l1, const ParamList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_param_declarations(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_param_type_lists(const ParamTypeList* l1,
                                     const ParamTypeList* l2) {
    ASSERT(l1->is_variadic == l2->is_variadic);
    return compare_param_lists(&l1->param_list, &l2->param_list);
}

static bool compare_identifier_lists(const IdentifierList* l1,
                                     const IdentifierList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_identifiers(&l1->identifiers[i], &l2->identifiers[i]));
    }
    return true;
}

static bool compare_arr_suffixes(const ArrSuffix* s1, const ArrSuffix* s2) {
    ASSERT(s1->is_static == s2->is_static);
    ASSERT(compare_type_quals(&s1->type_quals, &s2->type_quals));
    ASSERT(s1->is_asterisk == s2->is_asterisk);
    COMPARE_NULLABLE(s1->arr_len, s2->arr_len, compare_assign_exprs);
    return true;
}

static bool compare_arr_or_func_suffix(const ArrOrFuncSuffix* s1,
                                       const ArrOrFuncSuffix* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case ARR_OR_FUNC_ARRAY:
            return compare_arr_suffixes(&s1->arr_suffix, &s2->arr_suffix);
        case ARR_OR_FUNC_FUN_PARAMS:
            return compare_param_type_lists(&s1->fun_types, &s2->fun_types);
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            return compare_identifier_lists(&s1->fun_params, &s2->fun_params);
        case ARR_OR_FUNC_FUN_EMPTY:
            return true;
    }
    UNREACHABLE();
}

static bool compare_direct_declarators(const DirectDeclarator* d1,
                                       const DirectDeclarator* d2) {
    ASSERT(compare_ast_node_infos(&d1->info, &d2->info));
    ASSERT(d1->is_id == d2->is_id);
    if (d1->is_id) {
        ASSERT(compare_identifiers(d1->id, d2->id));
    } else {
        ASSERT(compare_declarators(d1->bracket_decl, d2->bracket_decl));
    }
    ASSERT(d1->len == d2->len);
    for (size_t i = 0; i < d1->len; ++i) {
        ASSERT(compare_arr_or_func_suffix(&d1->suffixes[i], &d2->suffixes[i]));
    }
    return true;
}

static bool compare_declarators(const Declarator* d1, const Declarator* d2) {
    COMPARE_NULLABLE(d1->ptr, d2->ptr, compare_pointers);
    return compare_direct_declarators(d1->direct_decl, d2->direct_decl);
}

static bool compare_struct_declarator(const StructDeclarator* d1,
                                      const StructDeclarator* d2) {
    COMPARE_NULLABLE(d1->decl, d2->decl, compare_declarators);
    COMPARE_NULLABLE(d1->bit_field, d2->bit_field, compare_const_exprs);
    return true;
}

static bool compare_struct_declarator_list(const StructDeclaratorList* l1,
                                           const StructDeclaratorList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_struct_declarator(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_struct_declarations(const StructDeclaration* d1,
                                        const StructDeclaration* d2) {
    ASSERT(d1->is_static_assert == d2->is_static_assert);
    if (d1->is_static_assert) {
        return compare_static_assert_declarations(d1->assert, d2->assert);
    } else {
        ASSERT(compare_declaration_specs(d1->decl_specs, d2->decl_specs));
        return compare_struct_declarator_list(&d1->decls, &d2->decls);
    }
}

static bool compare_struct_declaration_lists(const StructDeclarationList* l1,
                                             const StructDeclarationList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_struct_declarations(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_struct_union_specs(const StructUnionSpec* s1,
                                       const StructUnionSpec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->is_struct == s2->is_struct);
    COMPARE_NULLABLE(s1->identifier, s2->identifier, compare_identifiers);
    return compare_struct_declaration_lists(&s1->decl_list, &s2->decl_list);
}

static bool compare_enumerators(const Enumerator* e1, const Enumerator* e2) {
    ASSERT(compare_identifiers(e1->identifier, e2->identifier));
    COMPARE_NULLABLE(e1->enum_val, e2->enum_val, compare_const_exprs);
    return true;
}

static bool compare_enum_list(const EnumList* l1, const EnumList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_enumerators(&l1->enums[i], &l2->enums[i]));
    }
    return true;
}

static bool compare_enum_spec(const EnumSpec* s1, const EnumSpec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    COMPARE_NULLABLE(s1->identifier, s2->identifier, compare_identifiers);
    return compare_enum_list(&s1->enum_list, &s1->enum_list);
}

static bool compare_type_modifiers(const TypeModifiers* m1,
                                   const TypeModifiers* m2) {
    ASSERT(m1->is_unsigned == m2->is_unsigned);
    ASSERT(m1->is_signed == m2->is_signed);
    ASSERT(m1->is_short == m2->is_short);
    ASSERT(m1->num_long == m2->num_long);
    ASSERT(m1->is_complex == m2->is_complex);
    return m1->is_imaginary == m2->is_imaginary;
}

static bool compare_type_specs(const TypeSpecs* s1, const TypeSpecs* s2) {
    ASSERT(compare_type_modifiers(&s1->mods, &s2->mods));
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            return true;
        case TYPE_SPEC_ATOMIC:
            return compare_atomic_type_specs(s1->atomic_spec, s2->atomic_spec);
        case TYPE_SPEC_STRUCT:
            return compare_struct_union_specs(s1->struct_union_spec,
                                              s2->struct_union_spec);
        case TYPE_SPEC_ENUM:
            return compare_enum_spec(s1->enum_spec, s2->enum_spec);
        case TYPE_SPEC_TYPENAME:
            return compare_identifiers(s1->typedef_name, s2->typedef_name);
    }
    UNREACHABLE();
}

static bool compare_spec_qual_list(const SpecQualList* l1,
                                   const SpecQualList* l2) {
    ASSERT(compare_ast_node_infos(&l1->info, &l2->info));
    ASSERT(compare_type_quals(&l1->quals, &l2->quals));
    return compare_type_specs(&l1->specs, &l2->specs);
}

static bool compare_type_names(const TypeName* t1, const TypeName* t2) {
    ASSERT(compare_spec_qual_list(t1->spec_qual_list, t2->spec_qual_list));
    COMPARE_NULLABLE(t1->abstract_decl,
                     t2->abstract_decl,
                     compare_abs_declarators);
    return true;
}

static bool compare_align_spec(const AlignSpec* s1, const AlignSpec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->is_type_name == s2->is_type_name);
    if (s1->is_type_name) {
        return compare_type_names(s1->type_name, s2->type_name);
    } else {
        return compare_const_exprs(s1->const_expr, s2->const_expr);
    }
}

static bool compare_func_specs(const FuncSpecs* s1, const FuncSpecs* s2) {
    ASSERT(s1->is_inline == s2->is_inline);
    return s1->is_noreturn == s2->is_noreturn;
}

static bool compare_storage_class(const StorageClass* c1,
                                  const StorageClass* c2) {
    ASSERT(c1->is_typedef == c2->is_typedef);
    ASSERT(c1->is_extern == c2->is_extern);
    ASSERT(c1->is_static == c2->is_static);
    ASSERT(c1->is_thread_local == c2->is_thread_local);
    ASSERT(c1->is_auto == c2->is_auto);
    return c1->is_register == c2->is_register;
}

static bool compare_declaration_specs(const DeclarationSpecs* s1,
                                      const DeclarationSpecs* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(compare_func_specs(&s1->func_specs, &s2->func_specs));
    ASSERT(compare_storage_class(&s1->storage_class, &s2->storage_class));
    ASSERT(compare_type_quals(&s1->type_quals, &s2->type_quals));

    ASSERT(s1->num_align_specs == s2->num_align_specs);
    for (size_t i = 0; i < s1->num_align_specs; ++i) {
        ASSERT(compare_align_spec(&s1->align_specs[i], &s1->align_specs[i]));
    }
    return compare_type_specs(&s1->type_specs, &s2->type_specs);
}

static bool compare_init_declarators(const InitDeclarator* d1,
                                     const InitDeclarator* d2) {
    ASSERT(compare_declarators(d1->decl, d2->decl));
    COMPARE_NULLABLE(d1->init, d2->init, compare_initializers);
    return true;
}

static bool compare_init_declarator_lists(const InitDeclaratorList* l1,
                                          const InitDeclaratorList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_init_declarators(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_declarations(const Declaration* d1, const Declaration* d2) {
    ASSERT(d1->is_normal_decl == d2->is_normal_decl);
    if (d1->is_normal_decl) {
        ASSERT(compare_declaration_specs(d1->decl_specs, d2->decl_specs));
        return compare_init_declarator_lists(&d1->init_decls, &d2->init_decls);
    } else {
        return compare_static_assert_declarations(d1->static_assert_decl,
                                                  d2->static_assert_decl);
    }
}

static bool compare_declaration_lists(const DeclarationList* l1,
                                      const DeclarationList* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_declarations(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_statements(const Statement* s1, const Statement* s2);

static bool compare_labeled_statements(const LabeledStatement* s1,
                                       const LabeledStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case LABELED_STATEMENT_CASE:
            ASSERT(compare_const_exprs(s1->case_expr, s2->case_expr));
            break;
        case LABELED_STATEMENT_LABEL:
            ASSERT(compare_identifiers(s1->label, s2->label));
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
    }

    return compare_statements(s1->stat, s2->stat);
}

static bool compare_block_items(const BlockItem* i1, const BlockItem* i2) {
    ASSERT(i1->is_decl == i2->is_decl);
    if (i1->is_decl) {
        return compare_declarations(&i1->decl, &i2->decl);
    } else {
        return compare_statements(&i1->stat, &i2->stat);
    }
}

static bool compare_compound_statements(const CompoundStatement* s1,
                                        const CompoundStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->len == s2->len);
    for (size_t i = 0; i < s1->len; ++i) {
        ASSERT(compare_block_items(&s1->items[i], &s2->items[i]));
    }
    return true;
}

static bool compare_expr_statements(const ExprStatement* s1,
                                    const ExprStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    return compare_exprs(&s1->expr, &s2->expr);
}

static bool compare_selection_statements(const SelectionStatement* s1,
                                         const SelectionStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->is_if == s2->is_if);
    ASSERT(compare_exprs(s1->sel_expr, s2->sel_expr));
    ASSERT(compare_statements(s1->sel_stat, s2->sel_stat));
    COMPARE_NULLABLE(s1->else_stat, s2->else_stat, compare_statements);
    return true;
}

static bool compare_for_loops(const ForLoop* l1, const ForLoop* l2) {
    ASSERT(l1->is_decl == l2->is_decl);
    if (l1->is_decl) {
        ASSERT(compare_declarations(&l1->init_decl, &l2->init_decl));
    } else {
        ASSERT(compare_expr_statements(l1->init_expr, l2->init_expr));
    }
    ASSERT(compare_expr_statements(l1->cond, l2->cond));
    return compare_exprs(l1->incr_expr, l2->incr_expr);
}

static bool compare_iteration_statements(const struct IterationStatement* s1,
                                         const struct IterationStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->kind == s2->kind);
    ASSERT(compare_statements(s1->loop_body, s2->loop_body));
    switch (s1->kind) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            return compare_exprs(s1->while_cond, s2->while_cond);
        case ITERATION_STATEMENT_FOR:
            return compare_for_loops(&s1->for_loop, &s2->for_loop);
    }
    UNREACHABLE();
}

static bool compare_jump_statements(const struct JumpStatement* s1,
                                    const struct JumpStatement* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case JUMP_STATEMENT_GOTO:
            return compare_identifiers(s1->goto_label, s2->goto_label);
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            return true;
        case JUMP_STATEMENT_RETURN:
            COMPARE_NULLABLE(s1->ret_val, s2->ret_val, compare_exprs);
            return true;
    }
    UNREACHABLE();
}

static bool compare_statements(const Statement* s1, const Statement* s2) {
    ASSERT(s1->kind == s2->kind);
    switch (s1->kind) {
        case STATEMENT_LABELED:
            return compare_labeled_statements(s1->labeled, s2->labeled);
        case STATEMENT_COMPOUND:
            return compare_compound_statements(s1->comp, s2->comp);
        case STATEMENT_EXPRESSION:
            return compare_expr_statements(s1->expr, s2->expr);
        case STATEMENT_SELECTION:
            return compare_selection_statements(s1->sel, s2->sel);
        case STATEMENT_ITERATION:
            return compare_iteration_statements(s1->it, s2->it);
        case STATEMENT_JUMP:
            return compare_jump_statements(s1->jmp, s2->jmp);
    }
    UNREACHABLE();
}

static bool compare_func_defs(const FuncDef* d1, const FuncDef* d2) {
    ASSERT(compare_declaration_specs(d1->specs, d2->specs));
    ASSERT(compare_declarators(d1->decl, d2->decl));
    ASSERT(compare_declaration_lists(&d1->decl_list, &d2->decl_list));
    return compare_compound_statements(&d1->comp, &d2->comp);
}

static bool compare_external_declarations(const ExternalDeclaration* d1,
                                          const ExternalDeclaration* d2) {
    ASSERT(d1->is_func_def == d2->is_func_def);

    if (d1->is_func_def) {
        return compare_func_defs(&d1->func_def, &d2->func_def);
    } else {
        return compare_declarations(&d1->decl, &d2->decl);
    }
}

static bool compare_translation_units(const TranslationUnit* tl1,
                                      const TranslationUnit* tl2) {
    ASSERT(tl1->len == tl2->len);

    for (size_t i = 0; i < tl1->len; ++i) {
        const ExternalDeclaration* d1 = &tl1->external_decls[i];
        const ExternalDeclaration* d2 = &tl2->external_decls[i];
        ASSERT(compare_external_declarations(d1, d2));
    }
    return true;
}
