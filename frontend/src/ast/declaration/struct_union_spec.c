#include "frontend/ast/declaration/struct_union_spec.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct struct_union_spec* create_struct_union(
    struct source_loc loc,
    bool is_struct,
    struct identifier* identifier,
    struct struct_declaration_list decl_list) {
    struct struct_union_spec* res = xmalloc(sizeof(struct struct_union_spec));
    res->info = create_ast_node_info(loc);
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;

    return res;
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
        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
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
        list = parse_struct_declaration_list(s);
        if (list.len == 0) {
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

static void free_children(struct struct_union_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_struct_declaration_list(&s->decl_list);
}

void free_struct_union_spec(struct struct_union_spec* s) {
    free_children(s);
    free(s);
}

