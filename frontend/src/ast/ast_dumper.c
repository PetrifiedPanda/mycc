#include "frontend/ast/ast_dumper.h"

#include <stdarg.h>
#include <setjmp.h>
#include <assert.h>

#include "util/annotations.h"

struct ast_dumper {
    jmp_buf err_buf;
    FILE* file;
    size_t num_indents;
    const struct file_info* file_info;
};

static void add_indent(struct ast_dumper* d) {
    d->num_indents += 1;
}

static void remove_indent(struct ast_dumper* d) {
    d->num_indents -= 1;
}

static void print_indents(struct ast_dumper* d) {
    for (size_t i = 0; i < d->num_indents; ++i) {
        if (fprintf(d->file, "  ") < 0) {
            longjmp(d->err_buf, 0);
        }
    }
}

static void dumper_println(struct ast_dumper* d, const char* format, ...) {
    print_indents(d);

    va_list args;
    va_start(args, format);
    int res = vfprintf(d->file, format, args);
    va_end(args);

    if (res < 0) {
        longjmp(d->err_buf, 0);
    }

    if (fprintf(d->file, "\n") < 0) {
        longjmp(d->err_buf, 0);
    }
}

static void dumper_print_node_head(struct ast_dumper* d,
                                   const char* name,
                                   const struct ast_node_info* node) {
    const struct source_loc* loc = &node->loc;
    assert(loc->file_idx < d->file_info->len);
    const char* file_path = d->file_info->paths[loc->file_idx];
    dumper_println(d,
                   "%s: %s:%zu,%zu",
                   name,
                   file_path,
                   loc->file_loc.line,
                   loc->file_loc.index);
}

static void dump_translation_unit(struct ast_dumper* d,
                                  const struct translation_unit* tl);

bool dump_ast(const struct translation_unit* tl,
              const struct file_info* file_info,
              FILE* f) {
    struct ast_dumper d = {
        .file = f,
        .file_info = file_info,
        .num_indents = 0,
    };

    if (setjmp(d.err_buf) == 0) {
        dump_translation_unit(&d, tl);
    } else {
        return false;
    }

    return true;
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

static void dump_type_name(struct ast_dumper* d, const struct type_name* n);
static void dump_const_expr(struct ast_dumper* d, const struct const_expr* e);

static void dump_align_spec(struct ast_dumper* d, const struct align_spec* s) {
    assert(s);

    dumper_print_node_head(d, "align_spec", &s->info);

    add_indent(d);

    if (s->is_type_name) {
        dump_type_name(d, s->type_name);
    } else {
        dump_const_expr(d, s->const_expr);
    }

    remove_indent(d);
}

static void dump_type_specs(struct ast_dumper* d, const struct type_specs* s);

static void dump_declaration_specs(struct ast_dumper* d,
                                   const struct declaration_specs* s) {
    assert(s);

    dumper_print_node_head(d, "declaration_specs", &s->info);

    add_indent(d);

    dump_func_specs(d, &s->func_specs);
    dump_storage_class(d, &s->storage_class);

    dump_type_quals(d, &s->type_quals);

    dumper_println(d, "num_align_specs: %zu", s->num_align_specs);
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        dump_align_spec(d, &s->align_specs[i]);
    }

    dump_type_specs(d, &s->type_specs);

    remove_indent(d);
}

static void dump_pointer(struct ast_dumper* d, const struct pointer* p) {
    assert(p);

    dumper_print_node_head(d, "pointer", &p->info);

    add_indent(d);

    dumper_println(d, "num_indirs: %zu", p->num_indirs);
    for (size_t i = 0; i < p->num_indirs; ++i) {
        dump_type_quals(d, &p->quals_after_ptr[i]);
    }

    remove_indent(d);
}

static void dump_identifier(struct ast_dumper* d, struct identifier* i) {
    assert(i);

    dumper_print_node_head(d, "identifier", &i->info);

    add_indent(d);
    dumper_println(d, "spelling: %s", i->spelling);
    remove_indent(d);
}

static void dump_constant(struct ast_dumper* d, const struct constant* c) {
    assert(c);

    dumper_print_node_head(d, "constant", &c->info);

    add_indent(d);

    switch (c->type) {
        case CONSTANT_ENUM:
            dumper_println(d, "enum: %s", c->spelling);
            break;
        case CONSTANT_FLOAT:
            dumper_println(d, "float_constant: %s", c->spelling);
            break;
        case CONSTANT_INT:
            dumper_println(d, "int_constant: %s", c->spelling);
            break;
    }

    remove_indent(d);
}

static void dump_string_literal(struct ast_dumper* d,
                                const struct string_literal* l) {
    assert(l);
    dumper_print_node_head(d, "string_literal", &l->info);

    add_indent(d);
    dumper_println(d, "%s", l->spelling);
    remove_indent(d);
}

static void dump_string_constant(struct ast_dumper* d,
                                 const struct string_constant* c) {
    assert(c);

    add_indent(d);

    if (c->is_func) {
        dumper_print_node_head(d, "string_constant", &c->info);
        dumper_println(d, "%s", get_spelling(FUNC_NAME));
    } else {
        dumper_println(d, "string_constant:");
        dump_string_literal(d, &c->lit);
    }

    remove_indent(d);
}

static void dump_assign_expr(struct ast_dumper* d, const struct assign_expr* e);

static void dump_generic_assoc(struct ast_dumper* d,
                               const struct generic_assoc* a) {
    assert(a);

    dumper_print_node_head(d, "generic_assoc", &a->info);

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

    dumper_print_node_head(d, "generic_assoc_list", &l->info);

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

    dumper_print_node_head(d, "generic_sel", &s->info);

    add_indent(d);

    dump_assign_expr(d, s->assign);

    dump_generic_assoc_list(d, &s->assocs);

    remove_indent(d);
}

static void dump_expr(struct ast_dumper* d, const struct expr* e);

static void dump_primary_expr(struct ast_dumper* d,
                              const struct primary_expr* e) {
    assert(e);

    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_identifier(d, e->identifier);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_CONSTANT:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_constant(d, &e->constant);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_string_constant(d, &e->string);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_BRACKET:
            dumper_print_node_head(d, "primary_expr", &e->info);

            add_indent(d);

            dump_expr(d, e->bracket_expr);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_GENERIC:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_generic_sel(d, e->generic);

            remove_indent(d);
            break;
    }
}

static void dump_type_modifiers(struct ast_dumper* d,
                                const struct type_modifiers* m) {
    assert(m);

    dumper_println(d, "type_modifiers:");

    add_indent(d);

    dumper_println(d, "is_unsigned: %s", bool_to_str(m->is_unsigned));
    dumper_println(d, "is_signed: %s", bool_to_str(m->is_signed));
    dumper_println(d, "is_short: %s", bool_to_str(m->is_short));
    dumper_println(d, "num_long: %d", m->num_long);
    dumper_println(d, "is_complex: %s", bool_to_str(m->is_complex));
    dumper_println(d, "is_imaginary: %s", bool_to_str(m->is_imaginary));

    remove_indent(d);
}

static void dump_atomic_type_spec(struct ast_dumper* d,
                                  const struct atomic_type_spec* s) {
    assert(s);

    dumper_print_node_head(d, "atomic_type_spec", &s->info);

    add_indent(d);

    dump_type_name(d, s->type_name);

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl);

static void dump_struct_declarator(struct ast_dumper* d,
                                   struct struct_declarator* decl) {
    assert(decl);

    dumper_println(d, "struct_declarator:");

    add_indent(d);

    if (decl->decl) {
        dump_declarator(d, decl->decl);
    }
    if (decl->bit_field) {
        dump_const_expr(d, decl->bit_field);
    }

    remove_indent(d);
}

static void dump_struct_declarator_list(
    struct ast_dumper* d,
    const struct struct_declarator_list* l) {
    assert(l);

    dumper_println(d, "struct_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);

    for (size_t i = 0; i < l->len; ++i) {
        dump_struct_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_static_assert_declaration(
    struct ast_dumper* d,
    const struct static_assert_declaration* decl);

static void dump_struct_declaration(struct ast_dumper* d,
                                    const struct struct_declaration* decl) {
    assert(decl);

    dumper_println(d, "struct_declaration");

    add_indent(d);

    if (decl->is_static_assert) {
        dump_static_assert_declaration(d, decl->assert);
    } else {
        dump_declaration_specs(d, decl->decl_specs);
        dump_struct_declarator_list(d, &decl->decls);
    }

    remove_indent(d);
}

static void dump_struct_declaration_list(
    struct ast_dumper* d,
    const struct struct_declaration_list* l) {
    assert(l);

    dumper_println(d, "struct_declaration_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_struct_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_struct_union_spec(struct ast_dumper* d,
                                   const struct struct_union_spec* s) {
    assert(d);

    dumper_print_node_head(d, "struct_union_spec", &s->info);

    add_indent(d);

    if (s->is_struct) {
        dumper_println(d, "struct");
    } else {
        dumper_println(d, "union");
    }

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_struct_declaration_list(d, &s->decl_list);

    remove_indent(d);
}

static void dump_enumerator(struct ast_dumper* d, const struct enumerator* e) {
    assert(e);

    dumper_println(d, "enumerator:");

    add_indent(d);

    dump_identifier(d, e->identifier);
    if (e->enum_val) {
        dump_const_expr(d, e->enum_val);
    }

    remove_indent(d);
}

static void dump_enum_list(struct ast_dumper* d, const struct enum_list* l) {
    assert(l);

    dumper_println(d, "enum_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_enumerator(d, &l->enums[i]);
    }

    remove_indent(d);
}

static void dump_enum_spec(struct ast_dumper* d, const struct enum_spec* s) {
    assert(s);

    dumper_print_node_head(d, "enum_spec", &s->info);

    add_indent(d);

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_enum_list(d, &s->enum_list);

    remove_indent(d);
}

static const char* type_spec_type_str(enum type_spec_type t) {
    switch (t) {
        case TYPE_SPEC_NONE:
            return "NO_TYPE_SPEC";
        case TYPE_SPEC_VOID:
            return get_spelling(VOID);
        case TYPE_SPEC_CHAR:
            return get_spelling(CHAR);
        case TYPE_SPEC_INT:
            return get_spelling(INT);
        case TYPE_SPEC_FLOAT:
            return get_spelling(FLOAT);
        case TYPE_SPEC_DOUBLE:
            return get_spelling(DOUBLE);
        case TYPE_SPEC_BOOL:
            return get_spelling(BOOL);
        default:
            UNREACHABLE();
    }
}

static void dump_type_specs(struct ast_dumper* d, const struct type_specs* s) {
    assert(s);

    dumper_println(d, "type_specs:");

    add_indent(d);

    dump_type_modifiers(d, &s->mods);

    switch (s->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            dumper_println(d, "%s", type_spec_type_str(s->type));
            break;
        case TYPE_SPEC_ATOMIC:
            dump_atomic_type_spec(d, s->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            dump_struct_union_spec(d, s->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            dump_enum_spec(d, s->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            dump_identifier(d, s->typedef_name);
            break;
    }

    remove_indent(d);
}

static void dump_spec_qual_list(struct ast_dumper* d,
                                const struct spec_qual_list* l) {
    assert(l);

    dumper_print_node_head(d, "spec_qual_list", &l->info);

    add_indent(d);

    dump_type_quals(d, &l->quals);
    dump_type_specs(d, &l->specs);

    remove_indent(d);
}

static void dump_abs_declarator(struct ast_dumper* d,
                                const struct abs_declarator* decl);

static void dump_type_name(struct ast_dumper* d, const struct type_name* n) {
    assert(n);

    dumper_println(d, "type_name:");

    add_indent(d);

    dump_spec_qual_list(d, n->spec_qual_list);
    if (n->abstract_decl) {
        dump_abs_declarator(d, n->abstract_decl);
    }

    remove_indent(d);
}

static void dump_arg_expr_list(struct ast_dumper* d,
                               const struct arg_expr_list* l) {
    assert(l);

    dumper_println(d, "arg_expr_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_assign_expr(d, &l->assign_exprs[i]);
    }

    remove_indent(d);
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
            dumper_println(d, "%s", s->is_inc ? "++" : "--");
            break;
    }

    remove_indent(d);
}

static void dump_init_list(struct ast_dumper* d, const struct init_list* l);

static void dump_postfix_expr(struct ast_dumper* d, struct postfix_expr* e) {
    assert(e);

    if (e->is_primary) {
        dumper_println(d, "postfix_expr:");
    } else {
        dumper_print_node_head(d, "postfix_expr", &e->info);
    }

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

static void dump_unary_expr(struct ast_dumper* d, struct unary_expr* e);

static void dump_cast_expr(struct ast_dumper* d, const struct cast_expr* e) {
    assert(e);

    dumper_print_node_head(d, "cast_expr", &e->info);

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_type_name(d, &e->type_names[i]);
    }

    dump_unary_expr(d, e->rhs);

    remove_indent(d);
}

static void dump_unary_expr(struct ast_dumper* d, struct unary_expr* e) {
    assert(e);

    dumper_print_node_head(d, "unary_expr", &e->info);

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

static const char* assign_expr_op_str(enum assign_expr_op o) {
    switch (o) {
        case ASSIGN_EXPR_ASSIGN:
            return get_spelling(ASSIGN);
        case ASSIGN_EXPR_MUL:
            return get_spelling(MUL_ASSIGN);
        case ASSIGN_EXPR_DIV:
            return get_spelling(DIV_ASSIGN);
        case ASSIGN_EXPR_MOD:
            return get_spelling(MOD_ASSIGN);
        case ASSIGN_EXPR_ADD:
            return get_spelling(ADD_ASSIGN);
        case ASSIGN_EXPR_SUB:
            return get_spelling(SUB_ASSIGN);
        case ASSIGN_EXPR_LSHIFT:
            return get_spelling(LEFT_ASSIGN);
        case ASSIGN_EXPR_RSHIFT:
            return get_spelling(RIGHT_ASSIGN);
        case ASSIGN_EXPR_AND:
            return get_spelling(AND_ASSIGN);
        case ASSIGN_EXPR_XOR:
            return get_spelling(XOR_ASSIGN);
        case ASSIGN_EXPR_OR:
            return get_spelling(OR_ASSIGN);
    };
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

        dumper_println(d, "%s", assign_expr_op_str(item->op));

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

    dumper_print_node_head(d, "abs_arr_or_func_suffix", &s->info);

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

static void dump_direct_abs_declarator(
    struct ast_dumper* d,
    const struct direct_abs_declarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_abs_declarator", &decl->info);

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

static void dump_arr_or_func_suffix(struct ast_dumper* d,
                                    const struct arr_or_func_suffix* s) {
    dumper_print_node_head(d, "arr_or_func_suffix", &s->info);

    add_indent(d);

    switch (s->type) {
        case ARR_OR_FUNC_ARRAY:
            dump_arr_suffix(d, &s->arr_suffix);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            dump_param_type_list(d, &s->fun_types);
            break;
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            dump_identifier_list(d, &s->fun_params);
            break;
        case ARR_OR_FUNC_FUN_EMPTY:
            dumper_println(d, "empty_func_suffix");
            break;
    }

    remove_indent(d);
}

static void dump_direct_declarator(struct ast_dumper* d,
                                   struct direct_declarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_declarator", &decl->info);

    add_indent(d);

    if (decl->is_id) {
        dump_identifier(d, decl->id);
    } else {
        dump_declarator(d, decl->decl);
    }

    for (size_t i = 0; i < decl->len; ++i) {
        struct arr_or_func_suffix* item = &decl->suffixes[i];
        dump_arr_or_func_suffix(d, item);
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

static void dump_labeled_statement(struct ast_dumper* d,
                                   const struct labeled_statement* s) {
    assert(s);

    dumper_print_node_head(d, "labeled_statement", &s->info);

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
    dumper_print_node_head(d, "expr_statement", &s->info);

    add_indent(d);

    if (s->expr.len != 0) {
        dump_expr(d, &s->expr);
    }

    remove_indent(d);
}

static void dump_selection_statement(struct ast_dumper* d,
                                     const struct selection_statement* s) {
    assert(s);

    dumper_print_node_head(d, "selection_statement", &s->info);

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

    dumper_print_node_head(d, "iteration_statement", &s->info);

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
    dumper_print_node_head(d, "jump_statement", &s->info);

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

    dumper_print_node_head(d, "compound_statement", &s->info);

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
    dumper_print_node_head(d, "designator", &des->info);

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

    dumper_print_node_head(d, "initializer", &i->info);

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

static const char* mul_expr_op_str(enum mul_expr_op o) {
    switch (o) {
        case MUL_EXPR_MUL:
            return get_spelling(ASTERISK);
        case MUL_EXPR_DIV:
            return get_spelling(DIV);
        case MUL_EXPR_MOD:
            return get_spelling(MOD);
    }
}

static void dump_mul_expr(struct ast_dumper* d, const struct mul_expr* e) {
    assert(e);

    dumper_println(d, "mul_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_cast_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct cast_expr_and_op* item = &e->mul_chain[i];
        dumper_println(d, "mul_op: %s", mul_expr_op_str(item->op));
        dump_cast_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* add_expr_op_str(enum add_expr_op op) {
    switch (op) {
        case ADD_EXPR_ADD:
            return get_spelling(ADD);
        case ADD_EXPR_SUB:
            return get_spelling(SUB);
    }
}

static void dump_add_expr(struct ast_dumper* d, const struct add_expr* e) {
    assert(e);

    dumper_println(d, "add_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_mul_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct mul_expr_and_op* item = &e->add_chain[i];
        dumper_println(d, "add_op: %s", add_expr_op_str(item->op));
        dump_mul_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* shift_expr_op_str(enum shift_expr_op o) {
    switch (o) {
        case SHIFT_EXPR_LEFT:
            return get_spelling(LEFT_OP);
        case SHIFT_EXPR_RIGHT:
            return get_spelling(RIGHT_OP);
    }
}

static void dump_shift_expr(struct ast_dumper* d, const struct shift_expr* e) {
    assert(e);

    dumper_println(d, "shift_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_add_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct add_expr_and_op* item = &e->shift_chain[i];
        dumper_println(d, "shift_op: %s", shift_expr_op_str(item->op));
        dump_add_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* rel_expr_op_str(enum rel_expr_op o) {
    switch (o) {
        case REL_EXPR_LT:
            return get_spelling(LT);
        case REL_EXPR_GT:
            return get_spelling(GT);
        case REL_EXPR_LE:
            return get_spelling(LE_OP);
        case REL_EXPR_GE:
            return get_spelling(GE_OP);
    }
}

static void dump_rel_expr(struct ast_dumper* d, const struct rel_expr* e) {
    assert(e);

    dumper_println(d, "rel_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_shift_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct shift_expr_and_op* item = &e->rel_chain[i];
        dumper_println(d, "rel_op: %s", rel_expr_op_str(item->op));
        dump_shift_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* eq_expr_op_str(enum eq_expr_op o) {
    switch (o) {
        case EQ_EXPR_EQ:
            return get_spelling(EQ_OP);
        case EQ_EXPR_NE:
            return get_spelling(NE_OP);
    }
}

static void dump_eq_expr(struct ast_dumper* d, const struct eq_expr* e) {
    assert(e);

    dumper_println(d, "eq_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_rel_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct rel_expr_and_op* item = &e->eq_chain[i];
        dumper_println(d, "eq_op: %s", eq_expr_op_str(item->op));
        dump_rel_expr(d, item->rhs);
    }

    remove_indent(d);
}

static void dump_and_expr(struct ast_dumper* d, const struct and_expr* e) {
    assert(e);

    dumper_println(d, "and_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_eq_expr(d, &e->eq_exprs[i]);
    }

    remove_indent(d);
}

static void dump_xor_expr(struct ast_dumper* d, const struct xor_expr* e) {
    assert(e);

    dumper_println(d, "xor_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_and_expr(d, &e->and_exprs[i]);
    }

    remove_indent(d);
}

static void dump_or_expr(struct ast_dumper* d, const struct or_expr* e) {
    assert(e);

    dumper_println(d, "or_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_xor_expr(d, &e->xor_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_and_expr(struct ast_dumper* d,
                              const struct log_and_expr* e) {
    assert(e);

    dumper_println(d, "log_and_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_or_expr(d, &e->or_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_or_expr(struct ast_dumper* d,
                             const struct log_or_expr* e) {
    assert(e);

    dumper_println(d, "log_or_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_log_and_expr(d, &e->log_ands[i]);
    }

    remove_indent(d);
}

static void dump_cond_expr(struct ast_dumper* d, const struct cond_expr* e) {
    assert(e);

    dumper_println(d, "cond_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        dump_log_or_expr(d, item->log_or);
        dump_expr(d, item->expr);
    }
    dump_log_or_expr(d, e->last_else);

    remove_indent(d);
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

