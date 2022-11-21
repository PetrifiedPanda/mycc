#include "frontend/ast/compare_asts.h"

#include <string.h>

#include "util/annotations.h"

// TODO: maybe assert is not the best name (especially because there already is
// an assert in testing)
#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            return false;                                                      \
        }                                                                      \
    } while (0)

static bool compare_translation_units(const struct translation_unit* tl1,
                                      const struct translation_unit* tl2);

bool compare_asts(const struct translation_unit* tl1,
                  const struct translation_unit* tl2) {
    return compare_translation_units(tl1, tl2);
}

static bool compare_ast_node_infos(const struct ast_node_info* i1,
                                   const struct ast_node_info* i2) {
    ASSERT(i1->loc.file_idx == i2->loc.file_idx);
    ASSERT(i1->loc.file_loc.line == i2->loc.file_loc.line);
    return i1->loc.file_loc.index == i2->loc.file_loc.index;
}

static bool compare_strs(const struct str* s1, const struct str* s2) {
    ASSERT(str_len(s1) == str_len(s2));
    return strcmp(str_get_data(s1), str_get_data(s2));
}

static bool compare_type_quals(const struct type_quals* q1,
                               const struct type_quals* q2) {
    ASSERT(q1->is_const == q2->is_const);
    ASSERT(q1->is_restrict == q2->is_restrict);
    ASSERT(q1->is_volatile == q2->is_volatile);
    return q1->is_atomic == q2->is_atomic;
}

static bool compare_type_names(const struct type_name* t1,
                               const struct type_name* t2);

static bool compare_atomic_type_specs(const struct atomic_type_spec* s1,
                                      const struct atomic_type_spec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    return compare_type_names(s1->type_name, s2->type_name);
}

static bool compare_identifiers(const struct identifier* i1,
                                const struct identifier* i2) {
    ASSERT(compare_ast_node_infos(&i1->info, &i2->info));
    return compare_strs(&i1->spelling, &i2->spelling);
}

static bool compare_int_value(const struct int_value* v1,
                              const struct int_value* v2) {
    ASSERT(v1->type == v2->type);
    if (int_value_is_signed(v1->type)) {
        return v1->int_val == v2->int_val;
    } else {
        return v1->uint_val == v2->uint_val;
    }
}

static bool compare_float_value(const struct float_value* v1,
                                const struct float_value* v2) {
    ASSERT(v1->type == v2->type);
    return v1->val == v2->val;
}

static bool compare_constants(const struct constant* c1,
                              const struct constant* c2) {
    ASSERT(compare_ast_node_infos(&c1->info, &c2->info));
    ASSERT(c1->type == c2->type);
    switch (c1->type) {
        case CONSTANT_ENUM:
            return compare_strs(&c1->spelling, &c2->spelling);
        case CONSTANT_INT:
            return compare_int_value(&c1->int_val, &c2->int_val);
        case CONSTANT_FLOAT:
            return compare_float_value(&c1->float_val, &c2->float_val);
    }
    UNREACHABLE();
}

static bool compare_string_literals(const struct string_literal* l1,
                                    const struct string_literal* l2) {
    ASSERT(compare_ast_node_infos(&l1->info, &l2->info));
    return compare_strs(&l1->spelling, &l2->spelling);
}

static bool compare_string_constants(const struct string_constant* c1,
                                     const struct string_constant* c2) {
    ASSERT(c1->is_func == c2->is_func);
    if (c1->is_func) {
        return compare_ast_node_infos(&c1->info, &c2->info); 
    } else {
        return compare_string_literals(&c1->lit, &c2->lit);
    }
}

static bool compare_assign_exprs(const struct assign_expr* e1,
                                 const struct assign_expr* e2) {
    (void)e1, (void)e2;
    // TODO:
    return false;
}

static bool compare_generic_assoc_lists(const struct generic_assoc_list* l1,
                                        const struct generic_assoc_list* l2) {
    (void)l1, (void)l2;
    // TODO:
    return false;
}

static bool compare_generic_sel(const struct generic_sel* s1,
                                const struct generic_sel* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(compare_assign_exprs(s1->assign, s2->assign));
    return compare_generic_assoc_lists(&s1->assocs, &s2->assocs);
}

static bool compare_exprs(const struct expr* e1, const struct expr* e2);

static bool compare_primary_exprs(const struct primary_expr* e1,
                                  const struct primary_expr* e2) {
    ASSERT(e1->type == e2->type);
    switch (e1->type) {
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

static bool compare_init_list(const struct init_list* l1, const struct init_list* l2) {
    (void)l1, (void)l2;
    // TODO:
    return false;
}

static bool compare_arg_expr_lists(const struct arg_expr_list* l1,
                                   const struct arg_expr_list* l2) {
    (void)l1, (void)l2;
    // TODO:
    return false;
}

static bool compare_postfix_suffixes(const struct postfix_suffix* s1,
                                     const struct postfix_suffix* s2) {
    ASSERT(s1->type == s2->type);
    switch (s1->type) {
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

static bool compare_postfix_exprs(const struct postfix_expr* e1,
                                  const struct postfix_expr* e2) {
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

static bool compare_cast_exprs(const struct cast_expr* e1,
                               const struct cast_expr* e2);

static bool compare_unary_exprs(const struct unary_expr* e1,
                                const struct unary_expr* e2) {
    ASSERT(compare_ast_node_infos(&e1->info, &e2->info));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(e1->ops_before[i] == e2->ops_before[i]);
    }
    ASSERT(e1->type == e2->type);
    switch (e1->type) {
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

static bool compare_cast_exprs(const struct cast_expr* e1,
                               const struct cast_expr* e2) {
    ASSERT(compare_ast_node_infos(&e1->info, &e2->info));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_type_names(&e1->type_names[i], &e2->type_names[i]));
    }
    return compare_unary_exprs(e1->rhs, e2->rhs);
}

static bool compare_mul_exprs(const struct mul_expr* e1,
                              const struct mul_expr* e2) {
    ASSERT(compare_cast_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct cast_expr_and_op* item1 = &e1->mul_chain[i];
        const struct cast_expr_and_op* item2 = &e2->mul_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_cast_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_add_exprs(const struct add_expr* e1,
                              const struct add_expr* e2) {
    ASSERT(compare_mul_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct mul_expr_and_op* item1 = &e1->add_chain[i];
        const struct mul_expr_and_op* item2 = &e2->add_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_mul_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_shift_exprs(const struct shift_expr* e1,
                                const struct shift_expr* e2) {
    ASSERT(compare_add_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct add_expr_and_op* item1 = &e1->shift_chain[i];
        const struct add_expr_and_op* item2 = &e2->shift_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_add_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_rel_exprs(const struct rel_expr* e1,
                              const struct rel_expr* e2) {
    ASSERT(compare_shift_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct shift_expr_and_op* item1 = &e1->rel_chain[i];
        const struct shift_expr_and_op* item2 = &e2->rel_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_shift_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_eq_exprs(const struct eq_expr* e1,
                             const struct eq_expr* e2) {
    ASSERT(compare_rel_exprs(e1->lhs, e2->lhs));
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct rel_expr_and_op* item1 = &e1->eq_chain[i];
        const struct rel_expr_and_op* item2 = &e2->eq_chain[i];
        ASSERT(item1->op == item2->op);
        ASSERT(compare_rel_exprs(item1->rhs, item2->rhs));
    }
    return true;
}

static bool compare_and_exprs(const struct and_expr* e1,
                              const struct and_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_eq_exprs(&e1->eq_exprs[i], &e2->eq_exprs[i]));
    }
    return true;
}

static bool compare_xor_exprs(const struct xor_expr* e1,
                              const struct xor_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_and_exprs(&e1->and_exprs[i], &e2->and_exprs[i]));
    }
    return true;
}

static bool compare_or_exprs(const struct or_expr* e1,
                             const struct or_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_xor_exprs(&e1->xor_exprs[i], &e2->xor_exprs[i]));
    }
    return true;
}

static bool compare_log_and_exprs(const struct log_and_expr* e1,
                                  const struct log_and_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_or_exprs(&e1->or_exprs[i], &e2->or_exprs[i]));
    }
    return true;
}

static bool compare_log_or_exprs(const struct log_or_expr* e1,
                                 const struct log_or_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        ASSERT(compare_log_and_exprs(&e1->log_ands[i], &e2->log_ands[i]));
    }
    return true;
}

static bool compare_exprs(const struct expr* e1, const struct expr* e2) {
    (void)e1, (void)e2;
    // TODO:
    return false;
}

static bool compare_cond_exprs(const struct cond_expr* e1,
                               const struct cond_expr* e2) {
    ASSERT(e1->len == e2->len);
    for (size_t i = 0; i < e1->len; ++i) {
        const struct log_or_and_expr* item1 = &e1->conditionals[i];
        const struct log_or_and_expr* item2 = &e2->conditionals[i];
        ASSERT(compare_log_or_exprs(item1->log_or, item2->log_or));
        ASSERT(compare_exprs(item1->expr, item2->expr));
    }
    return compare_log_or_exprs(e1->last_else, e2->last_else);
}

static bool compare_const_exprs(const struct const_expr* e1,
                                const struct const_expr* e2) {
    return compare_cond_exprs(&e1->expr, &e2->expr);
}

static bool compare_static_assert_declarations(
    const struct static_assert_declaration* d1,
    const struct static_assert_declaration* d2) {
    ASSERT(compare_const_exprs(d1->const_expr, d2->const_expr));
    return compare_string_literals(&d1->err_msg, &d2->err_msg);
}

static bool compare_struct_declarator_list(
    const struct struct_declarator_list* l1,
    const struct struct_declarator_list* l2) {
    (void)l1, (void)l2;
    // TODO:
    return false;
}

static bool compare_declaration_specs(const struct declaration_specs* s1,
                                      const struct declaration_specs* s2);

static bool compare_struct_declarations(const struct struct_declaration* d1,
                                        const struct struct_declaration* d2) {
    ASSERT(d1->is_static_assert == d2->is_static_assert);
    if (d1->is_static_assert) {
        return compare_static_assert_declarations(d1->assert, d2->assert);
    } else {
        ASSERT(compare_declaration_specs(d1->decl_specs, d2->decl_specs));
        return compare_struct_declarator_list(&d1->decls, &d2->decls);
    }
}

static bool compare_struct_declaration_lists(
    const struct struct_declaration_list* l1,
    const struct struct_declaration_list* l2) {
    ASSERT(l1->len == l2->len);
    for (size_t i = 0; i < l1->len; ++i) {
        ASSERT(compare_struct_declarations(&l1->decls[i], &l2->decls[i]));
    }
    return true;
}

static bool compare_struct_union_specs(const struct struct_union_spec* s1,
                                       const struct struct_union_spec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->is_struct == s2->is_struct);
    if (s1->identifier == NULL) {
        ASSERT(s2->identifier == NULL);
    } else if (s2->identifier == NULL) {
        return false;
    } else {
        ASSERT(compare_identifiers(s1->identifier, s2->identifier));
    }
    return compare_struct_declaration_lists(&s1->decl_list, &s2->decl_list);
}

static bool compare_enum_spec(const struct enum_spec* s1,
                              const struct enum_spec* s2) {
    (void)s1, (void)s2;
    // TODO:
    return false;
}

static bool compare_type_modifiers(const struct type_modifiers* m1,
                                   const struct type_modifiers* m2) {
    ASSERT(m1->is_unsigned == m2->is_unsigned);
    ASSERT(m1->is_signed == m2->is_signed);
    ASSERT(m1->is_short == m2->is_short);
    ASSERT(m1->num_long == m2->num_long);
    ASSERT(m1->is_complex == m2->is_complex);
    return m1->is_imaginary == m2->is_imaginary;
}

static bool compare_type_specs(const struct type_specs* s1,
                               const struct type_specs* s2) {
    ASSERT(compare_type_modifiers(&s1->mods, &s2->mods));
    ASSERT(s1->type == s2->type);
    switch (s1->type) {
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

static bool compare_spec_qual_list(const struct spec_qual_list* l1,
                                   const struct spec_qual_list* l2) {
    ASSERT(compare_ast_node_infos(&l1->info, &l2->info));
    ASSERT(compare_type_quals(&l1->quals, &l2->quals));
    return compare_type_specs(&l1->specs, &l2->specs);
}

static bool compare_abs_declarator(const struct abs_declarator* d1,
                                   const struct abs_declarator* d2) {
    (void)d1, (void)d2;
    // TODO:
    return false;
}

static bool compare_type_names(const struct type_name* t1,
                               const struct type_name* t2) {
    ASSERT(compare_spec_qual_list(t1->spec_qual_list, t2->spec_qual_list));
    if (t1->abstract_decl == NULL) {
        return t2->abstract_decl == NULL;
    } else if (t2->abstract_decl == NULL) {
        return false;
    } else {
        return compare_abs_declarator(t1->abstract_decl, t2->abstract_decl);
    }
}

static bool compare_align_spec(const struct align_spec* s1,
                               const struct align_spec* s2) {
    ASSERT(compare_ast_node_infos(&s1->info, &s2->info));
    ASSERT(s1->is_type_name == s2->is_type_name);
    if (s1->is_type_name) {
        return compare_type_names(s1->type_name, s2->type_name);
    } else {
        return compare_const_exprs(s1->const_expr, s2->const_expr);
    }
}

static bool compare_func_specs(const struct func_specs* s1,
                               const struct func_specs* s2) {
    ASSERT(s1->is_inline == s2->is_inline);
    return s1->is_noreturn == s2->is_noreturn;
}

static bool compare_storage_class(const struct storage_class* c1,
                                  const struct storage_class* c2) {
    ASSERT(c1->is_typedef == c2->is_typedef);
    ASSERT(c1->is_extern == c2->is_extern);
    ASSERT(c1->is_static == c2->is_static);
    ASSERT(c1->is_thread_local == c2->is_thread_local);
    ASSERT(c1->is_auto == c2->is_auto);
    return c1->is_register == c2->is_register;
}

static bool compare_declaration_specs(const struct declaration_specs* s1,
                                      const struct declaration_specs* s2) {
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

static bool compare_declarator(const struct declarator* d1,
                               const struct declarator* d2) {
    (void)d1, (void)d2;
    // TODO:
    return false;
}

static bool compare_declaration_list(const struct declaration_list* l1,
                                     const struct declaration_list* l2) {
    (void)l1, (void)l2;
    // TODO:
    return false;
}

static bool compare_compound_statement(const struct compound_statement* s1,
                                       const struct compound_statement* s2) {
    (void)s1, (void)s2;
    // TODO:
    return false;
}

static bool compare_func_defs(const struct func_def* d1,
                              const struct func_def* d2) {
    ASSERT(compare_declaration_specs(d1->specs, d2->specs));
    ASSERT(compare_declarator(d1->decl, d2->decl));
    ASSERT(compare_declaration_list(&d1->decl_list, &d2->decl_list));
    return compare_compound_statement(d1->comp, d2->comp);
}

static bool compare_declarations(const struct declaration* d1,
                                 const struct declaration* d2) {
    (void)d1, (void)d2;
    // TODO:
    return false;
}

static bool compare_external_declarations(
    const struct external_declaration* d1,
    const struct external_declaration* d2) {
    ASSERT(d1->is_func_def == d2->is_func_def);

    if (d1->is_func_def) {
        return compare_func_defs(&d1->func_def, &d2->func_def);
    } else {
        return compare_declarations(&d1->decl, &d2->decl);
    }
}

static bool compare_translation_units(const struct translation_unit* tl1,
                                      const struct translation_unit* tl2) {
    ASSERT(tl1->len == tl2->len);

    for (size_t i = 0; i < tl1->len; ++i) {
        const struct external_declaration* d1 = &tl1->external_decls[i];
        const struct external_declaration* d2 = &tl2->external_decls[i];
        ASSERT(compare_external_declarations(d1, d2));
    }
    return true;
}
