#include "ast/ast_dumper.h"

#include <stdarg.h>
#include <assert.h>

#include "util/annotations.h"

struct ast_dumper {
    FILE* file;
    size_t num_indents;
};

static void add_indent(struct ast_dumper* d) {
    d->num_indents += 1;
}

static void remove_indent(struct ast_dumper* d) {
    d->num_indents -= 1;
}

static void print_indents(struct ast_dumper* d) {
    for (size_t i = 0; i < d->num_indents; ++i) {
        fprintf(d->file, "    ");
    }
}

static void dumper_println(struct ast_dumper* d, const char* format, ...) {
    print_indents(d);

    va_list args;
    va_start(args, format);
    vfprintf(d->file, format, args);
    va_end(args);

    fprintf(d->file, "\n");
}

static void dump_translation_unit(struct ast_dumper* d,
                                  const struct translation_unit* tl);

void dump_ast(const struct translation_unit* tl, FILE* f) {
    struct ast_dumper d = {
        .file = f,
        .num_indents = 0,
    };

    dump_translation_unit(&d, tl);
}

static const char* bool_to_str(bool b) {
    return b ? "true" : "false";
}

static void dump_func_specs(struct ast_dumper* d, const struct func_specs* s) {
    assert(s);

    dumper_println(d, "func_specs:");

    add_indent(d);

    dumper_println(d, "is_inline: %s", bool_to_str(s->is_inline));
    dumper_println(d, "is_noreturn: %s", bool_to_str(s->is_noreturn));

    remove_indent(d);
}

static void dump_storage_class(struct ast_dumper* d,
                               const struct storage_class* c) {
    assert(c);

    dumper_println(d, "storage_class:");

    add_indent(d);

    dumper_println(d, "is_typedef: %s", bool_to_str(c->is_typedef));
    dumper_println(d, "is_extern: %s", bool_to_str(c->is_extern));
    dumper_println(d, "is_static: %s", bool_to_str(c->is_static));
    dumper_println(d, "is_thread_local: %s", bool_to_str(c->is_thread_local));
    dumper_println(d, "is_auto: %s", bool_to_str(c->is_auto));
    dumper_println(d, "is_register: %s", bool_to_str(c->is_register));

    remove_indent(d);
}

static void dump_type_quals(struct ast_dumper* d, const struct type_quals* q) {
    assert(q);

    dumper_println(d, "type_quals:");

    add_indent(d);

    dumper_println(d, "is_const: %s", bool_to_str(q->is_const));
    dumper_println(d, "is_restrict: %s", bool_to_str(q->is_restrict));
    dumper_println(d, "is_volatile: %s", bool_to_str(q->is_volatile));
    dumper_println(d, "is_atomic: %s", bool_to_str(q->is_atomic));

    remove_indent(d);
}

static void dump_declaration_specs(struct ast_dumper* d,
                                   const struct declaration_specs* s) {
    assert(s);

    dumper_println(d, "declaration_specs:");

    add_indent(d);

    dump_func_specs(d, &s->func_specs);
    dump_storage_class(d, &s->storage_class);

    dump_type_quals(d, &s->type_quals);

    remove_indent(d);
}

static void dump_pointer(struct ast_dumper* d, const struct pointer* p) {
    assert(p);

    dumper_println(d, "pointer:");

    add_indent(d);

    dumper_println(d, "num_indirs: %zu", p->num_indirs);
    for (size_t i = 0; i < p->num_indirs; ++i) {
        dump_type_quals(d, &p->quals_after_ptr[i]);
    }

    remove_indent(d);
}

static void dump_identifier(struct ast_dumper* d, struct identifier* i) {
    assert(i);

    dumper_println(d, "identifier: %s", i->spelling);
}

static void dump_constant(struct ast_dumper* d, const struct constant* c) {
    assert(c);
    
    dumper_println(d, "constant:");

    add_indent(d);
    
    switch (c->type) {
        case ENUM:
            dumper_println(d, "enum: %s", c->spelling);
            break;
        case F_CONSTANT:
            dumper_println(d, "float_constant: %s", c->spelling);
            break;
        case I_CONSTANT:
            dumper_println(d, "int_constant: %s", c->spelling);
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_string_literal(struct ast_dumper* d,
                                const struct string_literal* l) {
    assert(l);
    dumper_println(d, "string_literal: %s", l->spelling);
}

static void dump_string_constant(struct ast_dumper* d,
                                 const struct string_constant* c) {
    assert(c);
    
    dumper_println(d, "string_constant:");

    add_indent(d);
    
    if (c->is_func) {
        dumper_println(d, "%s", get_spelling(FUNC_NAME));
    } else {
        dump_string_literal(d, &c->lit);
    }

    remove_indent(d);
}

static void dump_assign_expr(struct ast_dumper* d, const struct assign_expr* e);
static void dump_type_name(struct ast_dumper* d, const struct type_name* n);

static void dump_generic_assoc(struct ast_dumper* d,
                               const struct generic_assoc* a) {
    assert(a);

    dumper_println(d, "generic_assoc:");

    add_indent(d);

    if (a->type_name) {
        dump_type_name(d, a->type_name);
    } else {
        dumper_println(d, "default");
    }

    dump_assign_expr(d, a->assign);

    remove_indent(d);
}

static void dump_generic_assoc_list(struct ast_dumper* d,
                                    const struct generic_assoc_list* l) {
    assert(l);

    dumper_println(d, "generic_assoc_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);

    for (size_t i = 0; i < l->len; ++i) {
        dump_generic_assoc(d, &l->assocs[i]);
    }

    remove_indent(d);
}

static void dump_generic_sel(struct ast_dumper* d,
                             const struct generic_sel* s) {
    assert(d);

    dumper_println(d, "generic_sel:");

    add_indent(d);

    dump_assign_expr(d, s->assign);

    dump_generic_assoc_list(d, &s->assocs);

    remove_indent(d);
}

static void dump_expr(struct ast_dumper* d, const struct expr* e);

static void dump_primary_expr(struct ast_dumper* d,
                              const struct primary_expr* e) {
    assert(e);

    dumper_println(d, "primary_expr:");

    add_indent(d);

    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            dump_identifier(d, e->identifier);
            break;
        case PRIMARY_EXPR_CONSTANT:
            dump_constant(d, &e->constant);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            dump_string_constant(d, &e->string);
            break;
        case PRIMARY_EXPR_BRACKET:
            dump_expr(d, e->bracket_expr);
            break;
        case PRIMARY_EXPR_GENERIC:
            dump_generic_sel(d, e->generic);
            break;
    }

    remove_indent(d);
}

static void dump_type_name(struct ast_dumper* d, const struct type_name* n) {
    assert(n);
    (void)d;
    (void)n;
    // TODO:
}

static void dump_arg_expr_list(struct ast_dumper* d,
                               const struct arg_expr_list* l) {
    assert(l);
    (void)d;
    (void)l;
    // TODO:
}

static void dump_postfix_suffix(struct ast_dumper* d,
                                const struct postfix_suffix* s) {
    assert(s);

    dumper_println(d, "postfix_suffix:");

    add_indent(d);

    switch (s->type) {
        case POSTFIX_INDEX:
            dump_expr(d, s->index_expr);
            break;
        case POSTFIX_BRACKET:
            dump_arg_expr_list(d, &s->bracket_list);
            break;
        case POSTFIX_ACCESS:
            dumper_println(d, "access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_PTR_ACCESS:
            dumper_println(d, "pointer_access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_INC_DEC:
            dumper_println(d, "%s", get_spelling(s->inc_dec));
            break;
    }

    remove_indent(d);
}

static void dump_init_list(struct ast_dumper* d, const struct init_list* l);

static void dump_postfix_expr(struct ast_dumper* d, struct postfix_expr* e) {
    assert(e);

    dumper_println(d, "postfix_expr:");

    add_indent(d);

    if (e->is_primary) {
        dump_primary_expr(d, e->primary);
    } else {
        dump_type_name(d, e->type_name);
        dump_init_list(d, &e->init_list);
    }

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_postfix_suffix(d, &e->suffixes[i]);
    }

    remove_indent(d);
}

static void dump_cast_expr(struct ast_dumper* d, const struct cast_expr* e) {
    assert(e);
    (void)d;
    (void)e;
    // TODO:
}

static void dump_unary_expr(struct ast_dumper* d, struct unary_expr* e) {
    assert(e);

    dumper_println(d, "unary_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dumper_println(d, "%s", get_spelling(e->operators_before[i]));
    }

    switch (e->type) {
        case UNARY_POSTFIX:
            dump_postfix_expr(d, e->postfix);
            break;
        case UNARY_UNARY_OP:
            dumper_println(d, "%s", get_spelling(e->unary_op));
            dump_cast_expr(d, e->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
            dumper_println(d, "sizeof");
            dump_type_name(d, e->type_name);
            break;
        case UNARY_ALIGNOF_TYPE:
            dumper_println(d, "_Alignof");
            dump_type_name(d, e->type_name);
            break;
    }

    remove_indent(d);
}

static void dump_cond_expr(struct ast_dumper* d, const struct cond_expr* e);

static void dump_assign_expr(struct ast_dumper* d,
                             const struct assign_expr* e) {
    assert(e);

    dumper_println(d, "assign_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dumper_println(d, "unary_and_op:");
        add_indent(d);

        struct unary_and_op* item = &e->assign_chain[i];

        dump_unary_expr(d, item->unary);

        dumper_println(d, "%s", get_spelling(item->assign_op));

        remove_indent(d);
    }
    dump_cond_expr(d, e->value);

    remove_indent(d);
}
static void dump_param_type_list(struct ast_dumper* d,
                                 const struct param_type_list* l);

static void dump_abs_arr_or_func_suffix(
    struct ast_dumper* d,
    const struct abs_arr_or_func_suffix* s) {
    assert(s);

    dumper_println(d, "abs_arr_or_func_suffix:");

    add_indent(d);

    switch (s->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            dumper_println(d, "has_asterisk: %s", bool_to_str(s->has_asterisk));
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            dumper_println(d, "is_static: %s", bool_to_str(s->is_static));
            dump_type_quals(d, &s->type_quals);
            if (s->assign) {
                dump_assign_expr(d, s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            dump_param_type_list(d, &s->func_types);
            break;
    }

    remove_indent(d);
}

static void dump_abs_declarator(struct ast_dumper* d,
                                const struct abs_declarator* decl);

static void dump_direct_abs_declarator(
    struct ast_dumper* d,
    const struct direct_abs_declarator* decl) {
    assert(decl);

    dumper_println(d, "direct_abs_declarator:");

    add_indent(d);

    if (decl->bracket_decl) {
        dump_abs_declarator(d, decl->bracket_decl);
    }
    for (size_t i = 0; i < decl->len; ++i) {
        dump_abs_arr_or_func_suffix(d, &decl->following_suffixes[i]);
    }

    remove_indent(d);
}

static void dump_abs_declarator(struct ast_dumper* d,
                                const struct abs_declarator* decl) {
    assert(decl);

    dumper_println(d, "abs_declarator:");

    add_indent(d);
    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    if (decl->direct_abs_decl) {
        dump_direct_abs_declarator(d, decl->direct_abs_decl);
    }

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl);

static void dump_param_declaration(struct ast_dumper* d,
                                   const struct param_declaration* decl) {
    assert(decl);

    dumper_println(d, "param_declaration:");

    add_indent(d);

    dump_declaration_specs(d, decl->decl_specs);
    switch (decl->type) {
        case PARAM_DECL_DECL:
            dump_declarator(d, decl->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            dump_abs_declarator(d, decl->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            dumper_println(d, "no_decl");
            break;
    }

    remove_indent(d);
}

static void dump_param_list(struct ast_dumper* d, const struct param_list* l) {
    assert(l);

    dumper_println(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_param_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_param_type_list(struct ast_dumper* d,
                                 const struct param_type_list* l) {
    assert(l);

    dumper_println(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "is_variadic: %s", bool_to_str(l->is_variadic));
    dump_param_list(d, l->param_list);

    remove_indent(d);
}

static void dump_identifier_list(struct ast_dumper* d,
                                 const struct identifier_list* l) {
    assert(l);
    dumper_println(d, "identifier_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_identifier(d, &l->identifiers[i]);
    }

    remove_indent(d);
}

static void dump_arr_suffix(struct ast_dumper* d, const struct arr_suffix* s) {
    assert(s);

    dumper_println(d, "arr_suffix:");

    add_indent(d);

    dumper_println(d, "is_static: %s", bool_to_str(s->is_static));
    dump_type_quals(d, &s->type_quals);
    dumper_println(d, "is_asterisk: %s", bool_to_str(s->is_asterisk));
    if (s->arr_len) {
        dump_assign_expr(d, s->arr_len);
    }

    remove_indent(d);
}

static void dump_direct_declarator(struct ast_dumper* d,
                                   struct direct_declarator* decl) {
    assert(decl);

    dumper_println(d, "direct_declarator:");

    add_indent(d);

    if (decl->is_id) {
        dump_identifier(d, decl->id);
    } else {
        dump_declarator(d, decl->decl);
    }

    for (size_t i = 0; i < decl->len; ++i) {
        struct arr_or_func_suffix* item = &decl->suffixes[i];
        switch (item->type) {
            case ARR_OR_FUNC_ARRAY:
                dump_arr_suffix(d, &item->arr_suffix);
                break;
            case ARR_OR_FUNC_FUN_PARAMS:
                dump_param_type_list(d, &item->fun_types);
                break;
            case ARR_OR_FUNC_FUN_OLD_PARAMS:
                dump_identifier_list(d, &item->fun_params);
                break;
            case ARR_OR_FUNC_FUN_EMPTY:
                dumper_println(d, "empty_func_suffix");
                break;
        }
    }

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl) {
    assert(decl);

    dumper_println(d, "declarator:");

    add_indent(d);

    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    dump_direct_declarator(d, decl->direct_decl);

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* d,
                             const struct declaration* decl);

static void dump_declaration_list(struct ast_dumper* d,
                                  const struct declaration_list* l) {
    assert(l);

    dumper_println(d, "declaration_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_statement(struct ast_dumper* d, const struct statement* s);

static void dump_const_expr(struct ast_dumper* d, const struct const_expr* e);

static void dump_labeled_statement(struct ast_dumper* d,
                                   const struct labeled_statement* s) {
    assert(s);

    dumper_println(d, "labeled_statement:");

    add_indent(d);

    switch (s->type) {
        case CASE:
            dump_const_expr(d, s->case_expr);
            break;
        case IDENTIFIER:
            dump_identifier(d, s->identifier);
            break;
        case DEFAULT:
            dumper_println(d, "default");
            break;
        default:
            UNREACHABLE();
    }

    dump_statement(d, s->stat);

    remove_indent(d);
}

static void dump_expr(struct ast_dumper* d, const struct expr* e) {
    assert(e);

    dumper_println(d, "expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_assign_expr(d, &e->assign_exprs[i]);
    }

    remove_indent(d);
}

static void dump_compound_statement(struct ast_dumper* d,
                                    const struct compound_statement* s);

static void dump_expr_statement(struct ast_dumper* d,
                                const struct expr_statement* s) {
    assert(s);
    dumper_println(d, "expr_statement:");

    add_indent(d);

    if (s->expr.len != 0) {
        dump_expr(d, &s->expr);
    }

    remove_indent(d);
}

static void dump_selection_statement(struct ast_dumper* d,
                                     const struct selection_statement* s) {
    assert(s);

    dumper_println(d, "selection_statement:");

    add_indent(d);

    dumper_println(d, "is_if: %s", bool_to_str(s->is_if));

    dump_statement(d, s->sel_stat);

    if (s->else_stat) {
        assert(s->is_if);
        dump_statement(d, s->else_stat);
    }

    remove_indent(d);
}

static void dump_iteration_statement(struct ast_dumper* d,
                                     const struct iteration_statement* s) {
    assert(s);

    dumper_println(d, "iteration_statement:");

    add_indent(d);

    switch (s->type) {
        case WHILE:
            dumper_println(d, "type: while");
            dump_expr(d, s->while_cond);
            dump_statement(d, s->loop_body);
            break;
        case DO:
            dumper_println(d, "type: do");
            dump_statement(d, s->loop_body);
            dump_expr(d, s->while_cond);
            break;
        case FOR:
            dumper_println(d, "type: for");
            if (s->for_loop.is_decl) {
                dump_declaration(d, s->for_loop.init_decl);
            } else {
                dump_expr_statement(d, s->for_loop.init_expr);
            }
            dump_expr_statement(d, s->for_loop.cond);
            dump_expr(d, s->for_loop.incr_expr);
            dump_statement(d, s->loop_body);
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_jump_statement(struct ast_dumper* d,
                                const struct jump_statement* s) {
    assert(s);
    dumper_println(d, "jump_statement:");

    add_indent(d);

    switch (s->type) {
        case GOTO:
            dumper_println(d, "type: goto");
            dump_identifier(d, s->goto_label);
            break;
        case CONTINUE:
            dumper_println(d, "type: continue");
            break;
        case BREAK:
            dumper_println(d, "type: break");
            break;
        case RETURN:
            dumper_println(d, "type: return");
            if (s->ret_val) {
                dump_expr(d, s->ret_val);
            }
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_statement(struct ast_dumper* d, const struct statement* s) {
    assert(s);

    dumper_println(d, "statement:");

    add_indent(d);

    switch (s->type) {
        case STATEMENT_LABELED:
            dump_labeled_statement(d, s->labeled);
            break;
        case STATEMENT_COMPOUND:
            dump_compound_statement(d, s->comp);
            break;
        case STATEMENT_EXPRESSION:
            dump_expr_statement(d, s->expr);
            break;
        case STATEMENT_SELECTION:
            dump_selection_statement(d, s->sel);
            break;
        case STATEMENT_ITERATION:
            dump_iteration_statement(d, s->it);
            break;
        case STATEMENT_JUMP:
            dump_jump_statement(d, s->jmp);
            break;
    }

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* s,
                             const struct declaration* decl);

static void dump_block_item(struct ast_dumper* d, const struct block_item* i) {
    assert(i);

    dumper_println(d, "block_item:");

    add_indent(d);

    if (i->is_decl) {
        dump_declaration(d, &i->decl);
    } else {
        dump_statement(d, &i->stat);
    }

    remove_indent(d);
}

static void dump_compound_statement(struct ast_dumper* d,
                                    const struct compound_statement* s) {
    assert(s);

    dumper_println(d, "compound_statement:");

    add_indent(d);

    dumper_println(d, "len: %zu", s->len);
    for (size_t i = 0; i < s->len; ++i) {
        dump_block_item(d, &s->items[i]);
    }

    remove_indent(d);
}

static void dump_func_def(struct ast_dumper* d, const struct func_def* f) {
    assert(f);

    dumper_println(d, "func_def:");

    add_indent(d);

    dump_declaration_specs(d, f->specs);
    dump_declarator(d, f->decl);
    dump_declaration_list(d, &f->decl_list);
    dump_compound_statement(d, f->comp);

    remove_indent(d);
}

static void dump_designator(struct ast_dumper* d,
                            const struct designator* des) {
    assert(des);
    dumper_println(d, "designator:");

    add_indent(d);

    if (des->is_index) {
        dump_const_expr(d, des->arr_index);
    } else {
        dump_identifier(d, des->identifier);
    }

    remove_indent(d);
}

static void dump_designator_list(struct ast_dumper* d,
                                 const struct designator_list* l) {
    assert(l);

    dumper_println(d, "designator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_designator(d, &l->designators[i]);
    }

    remove_indent(d);
}

static void dump_designation(struct ast_dumper* d,
                             const struct designation* des) {
    assert(des);

    dumper_println(d, "designation:");

    add_indent(d);

    dump_designator_list(d, &des->designators);

    remove_indent(d);
}

static void dump_initializer(struct ast_dumper* d, const struct initializer* i);

static void dump_designation_init(struct ast_dumper* d,
                                  const struct designation_init* i) {
    assert(i);

    dumper_println(d, "designation_init:");

    add_indent(d);

    if (i->designation) {
        dump_designation(d, i->designation);
    }
    dump_initializer(d, i->init);

    remove_indent(d);
}

static void dump_init_list(struct ast_dumper* d, const struct init_list* l) {
    assert(l);

    dumper_println(d, "init_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_designation_init(d, &l->inits[i]);
    }

    remove_indent(d);
}

static void dump_initializer(struct ast_dumper* d,
                             const struct initializer* i) {
    assert(i);

    dumper_println(d, "initializer:");

    add_indent(d);

    if (i->is_assign) {
        dump_assign_expr(d, i->assign);
    } else {
        dump_init_list(d, &i->init_list);
    }

    remove_indent(d);
}

static void dump_init_declarator(struct ast_dumper* d,
                                 const struct init_declarator* decl) {
    assert(decl);

    dumper_println(d, "init_declarator:");

    add_indent(d);

    dump_declarator(d, decl->decl);
    if (decl->init) {
        dump_initializer(d, decl->init);
    }

    remove_indent(d);
}

static void dump_init_declarator_list(struct ast_dumper* d,
                                      const struct init_declarator_list* l) {
    assert(l);

    dumper_println(d, "init_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_init_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_cond_expr(struct ast_dumper* d, const struct cond_expr* e) {
    assert(e);
    (void)d;
    (void)e;
    // TODO:
}

static void dump_const_expr(struct ast_dumper* d, const struct const_expr* e) {
    assert(e);

    dumper_println(d, "const_expr:");

    add_indent(d);

    dump_cond_expr(d, &e->expr);

    remove_indent(d);
}

static void dump_static_assert_declaration(
    struct ast_dumper* d,
    const struct static_assert_declaration* decl) {
    assert(decl);

    dumper_println(d, "static_assert_declaration:");

    add_indent(d);

    dump_const_expr(d, decl->const_expr);
    dump_string_literal(d, &decl->err_msg);

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* d,
                             const struct declaration* decl) {
    assert(decl);

    dumper_println(d, "declaration:");

    add_indent(d);

    if (decl->is_normal_decl) {
        dump_declaration_specs(d, decl->decl_specs);
        dump_init_declarator_list(d, &decl->init_decls);
    } else {
        dump_static_assert_declaration(d, decl->static_assert_decl);
    }

    remove_indent(d);
}

static void dump_external_declaration(struct ast_dumper* d,
                                      const struct external_declaration* decl) {
    assert(decl);

    dumper_println(d, "external_declaration:");

    add_indent(d);

    if (decl->is_func_def) {
        dump_func_def(d, &decl->func_def);
    } else {
        dump_declaration(d, &decl->decl);
    }

    remove_indent(d);
}

static void dump_translation_unit(struct ast_dumper* d,
                                  const struct translation_unit* tl) {
    assert(tl);

    dumper_println(d, "translation_unit:");

    add_indent(d);

    dumper_println(d, "len: %zu", tl->len);

    for (size_t i = 0; i < tl->len; ++i) {
        dump_external_declaration(d, &tl->external_decls[i]);
    }

    remove_indent(d);
}

