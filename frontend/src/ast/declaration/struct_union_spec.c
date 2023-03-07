#include "frontend/ast/declaration/struct_union_spec.h"

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool parse_struct_declarator_inplace(struct parser_state* s,
                                     struct struct_declarator* res) {
    assert(res);

    if (s->it->type != COLON) {
        res->decl = parse_declarator(s);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    if (s->it->type == COLON) {
        accept_it(s);
        res->bit_field = parse_const_expr(s);
        if (!res->bit_field) {
            free_struct_declarator_children(res);
            return false;
        }
    } else {
        res->bit_field = NULL;
        if (!res->decl) {
            set_parser_err(s->err,
                           PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
                           s->it->loc);
            return false;
        }
    }

    return true;
}

void free_struct_declarator_children(struct struct_declarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

static bool parse_struct_declarator_list(struct parser_state* s,
                                  struct struct_declarator_list* res) {
    *res = (struct struct_declarator_list){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declarator_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!parse_struct_declarator_inplace(s, &res->decls[res->len])) {
            free_struct_declarator_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

void free_struct_declarator_list(struct struct_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

static bool parse_struct_declaration_inplace(struct parser_state* s,
                                      struct struct_declaration* res) {
    if (s->it->type == STATIC_ASSERT) {
        res->is_static_assert = true;
        res->assert = parse_static_assert_declaration(s);
        if (!res->assert) {
            return false;
        }
    } else {
        res->is_static_assert = false;
        bool found_typedef = false;
        res->decl_specs = parse_declaration_specs(s, &found_typedef);
        if (!res->decl_specs) {
            return false;
        }

        if (found_typedef) {
            set_parser_err(s->err, PARSER_ERR_TYPEDEF_STRUCT, s->it->loc);
        }

        if (s->it->type != SEMICOLON) {
            if (!parse_struct_declarator_list(s, &res->decls)) {
                free_declaration_specs(res->decl_specs);
                return false;
            }
        } else {
            res->decls = (struct struct_declarator_list){
                .len = 0,
                .decls = NULL,
            };
        }

        if (!accept(s, SEMICOLON)) {
            free_struct_declaration_children(res);
            return false;
        }
    }

    return true;
}

void free_struct_declaration_children(struct struct_declaration* d) {
    if (d->is_static_assert) {
        free_static_assert_declaration(d->assert);
    } else {
        free_declaration_specs(d->decl_specs);
        free_struct_declarator_list(&d->decls);
    }
}

static struct struct_union_spec* create_struct_union(
    struct source_loc loc,
    bool is_struct,
    struct identifier* identifier,
    struct struct_declaration_list decl_list) {
    struct struct_union_spec* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;

    return res;
}

static bool parse_struct_declaration_list(struct parser_state* s,
                                   struct struct_declaration_list* res) {
    *res = (struct struct_declaration_list){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declaration_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (is_declaration(s) || s->it->type == STATIC_ASSERT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!parse_struct_declaration_inplace(s, &res->decls[res->len])) {
            free_struct_declaration_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

void free_struct_declaration_list(struct struct_declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}


struct struct_union_spec* parse_struct_union_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    bool is_struct;
    if (s->it->type == STRUCT) {
        is_struct = true;
        accept_it(s);
    } else if (s->it->type == UNION) {
        is_struct = false;
        accept_it(s);
    } else {
        enum token_type expected[] = {STRUCT, UNION};
        expected_tokens_error(s, expected, ARR_LEN(expected));
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->type == IDENTIFIER) {
        const struct str spell = take_spelling(s->it);
        const struct source_loc id_loc = s->it->loc;
        accept_it(s);
        id = create_identifier(&spell, id_loc);
    }

    struct struct_declaration_list list = {.len = 0, .decls = NULL};
    if (s->it->type == LBRACE) {
        accept_it(s);
        if (!parse_struct_declaration_list(s, &list)) {
            goto fail;
        }

        if (!accept(s, RBRACE)) {
            free_struct_declaration_list(&list);
            goto fail;
        }
    }
    return create_struct_union(loc, is_struct, id, list);

fail:
    if (id) {
        free_identifier(id);
    }
    return NULL;
}

static void free_struct_union_spec_children(struct struct_union_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_struct_declaration_list(&s->decl_list);
}

void free_struct_union_spec(struct struct_union_spec* s) {
    free_struct_union_spec_children(s);
    mycc_free(s);
}

