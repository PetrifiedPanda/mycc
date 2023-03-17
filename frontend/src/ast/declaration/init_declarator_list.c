#include "frontend/ast/declaration/init_declarator_list.h"

#include <assert.h>

#include "util/mem.h"

static bool parse_init_declarator_list_first_base(
    struct parser_state* s,
    struct init_declarator_list* res,
    bool (*inplace_parse_func)(struct parser_state*, struct init_declarator*),
    struct init_declarator* first_decl) {
    assert(first_decl);
    res->len = 1;
    res->decls = first_decl;

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!inplace_parse_func(s, &res->decls[res->len])) {
            free_init_declarator_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return true;
}

static bool parse_init_declarator_list_base(
    struct parser_state* s,
    struct init_declarator_list* res,
    bool (*inplace_parse_func)(struct parser_state*, struct init_declarator*)) {
    struct init_declarator* first_decl = mycc_alloc(sizeof *first_decl);

    if (!inplace_parse_func(s, first_decl)) {
        mycc_free(first_decl);
        return false;
    }

    return parse_init_declarator_list_first_base(s,
                                                 res,
                                                 inplace_parse_func,
                                                 first_decl);
}

bool parse_init_declarator_list_first(struct parser_state* s,
                                      struct init_declarator_list* res,
                                      struct init_declarator* first_decl) {
    return parse_init_declarator_list_first_base(s,
                                                 res,
                                                 parse_init_declarator_inplace,
                                                 first_decl);
}

bool parse_init_declarator_list(struct parser_state* s,
                                struct init_declarator_list* res) {
    return parse_init_declarator_list_base(s,
                                           res,
                                           parse_init_declarator_inplace);
}

bool parse_init_declarator_list_typedef_first(
    struct parser_state* s,
    struct init_declarator_list* res,
    struct init_declarator* first_decl) {
    return parse_init_declarator_list_first_base(
        s,
        res,
        parse_init_declarator_typedef_inplace,
        first_decl);
}

bool parse_init_declarator_list_typedef(struct parser_state* s,
                                        struct init_declarator_list* res) {
    return parse_init_declarator_list_base(
        s,
        res,
        parse_init_declarator_typedef_inplace);
}

void free_init_declarator_list(struct init_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}
