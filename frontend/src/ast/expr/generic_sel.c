#include "frontend/ast/expr/generic_sel.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_generic_assoc_inplace(struct parser_state* s,
                                        struct generic_assoc* res) {
    assert(res);

    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == DEFAULT) {
        accept_it(s);
        res->type_name = NULL;
    } else {
        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            return false;
        }
    }

    if (!accept(s, COLON)) {
        goto fail;
    }

    res->assign = parse_assign_expr(s);
    if (!res->assign) {
        goto fail;
    }

    return true;
fail:
    if (res->type_name) {
        free_type_name(res->type_name);
    }
    return false;
}

void free_generic_assoc_children(struct generic_assoc* a) {
    if (a->type_name) {
        free_type_name(a->type_name);
    }
    free_assign_expr(a->assign);
}

static bool parse_generic_assoc_list(struct parser_state* s,
                                     struct generic_assoc_list* res) {
    size_t alloc_len = 1;
    *res = (struct generic_assoc_list){
        .info = create_ast_node_info(s->it->loc),
        .len = 1,
        .assocs = mycc_alloc(sizeof *res->assocs * alloc_len),
    };

    if (!parse_generic_assoc_inplace(s, &res->assocs[0])) {
        mycc_free(res->assocs);
        return false;
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assocs,
                            &alloc_len,
                            sizeof *res->assocs);
        }

        if (!parse_generic_assoc_inplace(s, &res->assocs[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->assocs = mycc_realloc(res->assocs, sizeof *res->assocs * res->len);
    return res;

fail:
    free_generic_assoc_list(res);
    return false;
}

void free_generic_assoc_list(struct generic_assoc_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_generic_assoc_children(&l->assocs[i]);
    }
    mycc_free(l->assocs);
}

static struct generic_sel* create_generic_sel(struct assign_expr* assign,
                                              struct generic_assoc_list assocs,
                                              struct source_loc loc) {
    assert(assign);
    assert(assocs.len != 0);
    struct generic_sel* res = mycc_alloc(sizeof *res);

    res->info = create_ast_node_info(loc);
    res->assign = assign;
    res->assocs = assocs;
    return res;
}

struct generic_sel* parse_generic_sel(struct parser_state* s) {
    assert(s->it->type == GENERIC);
    struct source_loc loc = s->it->loc;
    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct assign_expr* assign = parse_assign_expr(s);
    if (!assign) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        goto fail;
    }

    struct generic_assoc_list assocs;
    if (!parse_generic_assoc_list(s, &assocs)) {
        goto fail;
    }

    if (!accept(s, RBRACKET)) {
        free_generic_assoc_list(&assocs);
        goto fail;
    }

    return create_generic_sel(assign, assocs, loc);

fail:
    free_assign_expr(assign);
    return NULL;
}

static void free_children(struct generic_sel* s) {
    free_assign_expr(s->assign);
    free_generic_assoc_list(&s->assocs);
}

void free_generic_sel(struct generic_sel* s) {
    free_children(s);
    mycc_free(s);
}
