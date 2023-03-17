#include "frontend/ast/expr/cast_expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_cast_expr_rest(struct parser_state* s,
                                 struct cast_expr* res) {
    size_t alloc_len = res->len;
    struct source_loc last_lbracket_loc = {
        .file_idx = (size_t)-1,
        .file_loc = {0, 0},
    };
    while (s->it->kind == TOKEN_LBRACKET && next_is_type_name(s)) {
        last_lbracket_loc = s->it->loc;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->type_names,
                       &alloc_len,
                       sizeof *res->type_names);
        }

        if (!parse_type_name_inplace(s, &res->type_names[res->len])) {
            goto fail;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            goto fail;
        }
        ++res->len;
    }

    if (s->it->kind == TOKEN_LBRACE) {
        assert(res->len > 0);
        struct type_name* type_name = mycc_alloc(sizeof *type_name);

        --res->len;
        *type_name = res->type_names[res->len];

        if (res->type_names) {
            res->type_names = mycc_realloc(res->type_names,
                                       sizeof *res->type_names * res->len);
        }

        res->rhs = parse_unary_expr_type_name(s,
                                              NULL,
                                              0,
                                              type_name,
                                              last_lbracket_loc);
    } else {
        if (res->type_names) {
            res->type_names = mycc_realloc(res->type_names,
                                       sizeof *res->type_names * res->len);
        }
        res->rhs = parse_unary_expr(s);
    }
    if (!res->rhs) {
        goto fail;
    }

    return true;

fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_type_name_children(&res->type_names[i]);
    }
    mycc_free(res->type_names);
    return false;
}

struct cast_expr* parse_cast_expr(struct parser_state* s) {
    struct cast_expr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    res->type_names = NULL;
    res->len = 0;

    if (!parse_cast_expr_rest(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

struct cast_expr* parse_cast_expr_type_name(
    struct parser_state* s,
    struct type_name* type_name,
    struct source_loc start_bracket_loc) {
    assert(type_name);

    struct cast_expr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(start_bracket_loc);
    res->type_names = type_name;
    res->len = 1;

    if (!parse_cast_expr_rest(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

struct cast_expr* create_cast_expr_unary(struct unary_expr* start) {
    assert(start);

    struct cast_expr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(start->info.loc);
    res->type_names = NULL;
    res->len = 0;
    res->rhs = start;
    return res;
}

static void free_children(struct cast_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_type_name_children(&e->type_names[i]);
    }
    mycc_free(e->type_names);
    free_unary_expr(e->rhs);
}

void free_cast_expr(struct cast_expr* e) {
    free_children(e);
    mycc_free(e);
}
