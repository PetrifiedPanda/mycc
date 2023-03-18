#include "frontend/ast/initializer.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool parse_designator_inplace(struct parser_state* s,
                                     struct designator* res) {
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->kind) {
        case TOKEN_LINDEX: {
            parser_accept_it(s);
            struct const_expr* index = parse_const_expr(s);
            if (!index) {
                return false;
            }
            if (!parser_accept(s, TOKEN_RINDEX)) {
                free_const_expr(index);
                return false;
            }

            res->is_index = true;
            res->arr_index = index;
            return true;
        }
        case TOKEN_DOT: {
            parser_accept_it(s);
            if (s->it->kind == TOKEN_IDENTIFIER) {
                const struct str spell = token_take_spelling(s->it);
                struct source_loc loc = s->it->loc;
                parser_accept_it(s);
                res->is_index = false;
                res->identifier = create_identifier(&spell, loc);
                return true;
            } else {
                expected_token_error(s, TOKEN_IDENTIFIER);
                return false;
            }
        }
        default: {
            static const enum token_kind expected[] = {TOKEN_LINDEX, TOKEN_DOT};
            expected_tokens_error(s, expected, ARR_LEN(expected));
            return false;
        }
    }
}

void free_designator_children(struct designator* d) {
    if (d->is_index) {
        free_const_expr(d->arr_index);
    } else {
        free_identifier(d->identifier);
    }
}

void free_designator_list(struct designator_list* l);

static bool parse_designator_list(struct parser_state* s,
                                  struct designator_list* res) {
    *res = (struct designator_list){
        .len = 1,
        .designators = mycc_alloc(sizeof *res->designators),
    };

    if (!parse_designator_inplace(s, &res->designators[0])) {
        mycc_free(res->designators);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_LINDEX || s->it->kind == TOKEN_DOT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->designators,
                            &alloc_len,
                            sizeof *res->designators);
        }

        if (!parse_designator_inplace(s, &res->designators[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->designators = mycc_realloc(res->designators,
                                    res->len * sizeof *res->designators);

    return res;

fail:
    free_designator_list(res);
    return false;
}

void free_designator_list(struct designator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_designator_children(&l->designators[i]);
    }
    mycc_free(l->designators);
}

static bool parse_designation_inplace(struct parser_state* s,
                                      struct designation* res) {
    if (!parse_designator_list(s, &res->designators)) {
        return false;
    }

    if (!parser_accept(s, TOKEN_ASSIGN)) {
        free_designator_list(&res->designators);
        return false;
    }

    return true;
}

void free_designation_children(struct designation* d) {
    free_designator_list(&d->designators);
}

void free_designation(struct designation* d) {
    free_designation_children(d);
    mycc_free(d);
}

static bool parse_initializer_inplace(struct parser_state* s,
                                      struct initializer* res);
bool is_valid_designation(const struct designation* d);

struct designation create_invalid_designation(void) {
    return (struct designation){
        .designators =
            {
                .len = 0,
                .designators = NULL,
            },
    };
}

static bool parse_designation_init(struct parser_state* s,
                                   struct designation_init* res) {
    if (s->it->kind == TOKEN_LINDEX || s->it->kind == TOKEN_DOT) {
        if (!parse_designation_inplace(s, &res->designation)) {
            return false;
        }
    } else {
        res->designation = create_invalid_designation();
    }

    if (!parse_initializer_inplace(s, &res->init)) {
        if (is_valid_designation(&res->designation)) {
            free_designation_children(&res->designation);
        }
        return false;
    }
    return true;
}

void free_init_list_children(struct init_list* l);

bool parse_init_list(struct parser_state* s, struct init_list* res) {
    *res = (struct init_list){
        .len = 1,
        .inits = mycc_alloc(sizeof *res->inits),
    };
    if (!parse_designation_init(s, &res->inits[0])) {
        mycc_free(res->inits);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_COMMA && s->it[1].kind != TOKEN_RBRACE) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->inits,
                            &alloc_len,
                            sizeof *res->inits);
        }

        if (!parse_designation_init(s, &res->inits[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->inits = mycc_realloc(res->inits, sizeof *res->inits * res->len);

    return res;
fail:
    free_init_list_children(res);
    return false;
}

bool is_valid_designation(const struct designation* d) {
    return d->designators.len != 0;
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct designation_init* item = &l->inits[i];
        if (is_valid_designation(&item->designation)) {
            free_designation_children(&item->designation);
        }
        free_initializer_children(&item->init);
    }
    mycc_free(l->inits);
}

static bool parse_initializer_inplace(struct parser_state* s,
                                      struct initializer* res) {
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->kind == TOKEN_LBRACE) {
        res->is_assign = false;
        parser_accept_it(s);
        if (!parse_init_list(s, &res->init_list)) {
            return false;
        }

        if (s->it->kind == TOKEN_COMMA) {
            parser_accept_it(s);
        }

        if (!parser_accept(s, TOKEN_RBRACE)) {
            free_init_list_children(&res->init_list);
            return false;
        }
    } else {
        res->is_assign = true;
        res->assign = parse_assign_expr(s);
        if (!res->assign) {
            return false;
        }
    }
    return true;
}

struct initializer* parse_initializer(struct parser_state* s) {
    struct initializer* res = mycc_alloc(sizeof *res);
    if (!parse_initializer_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_initializer_children(struct initializer* i) {
    if (i->is_assign) {
        free_assign_expr(i->assign);
    } else {
        free_init_list_children(&i->init_list);
    }
}

void free_initializer(struct initializer* i) {
    free_initializer_children(i);
    mycc_free(i);
}
