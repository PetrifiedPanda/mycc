#include "frontend/ast/declaration/static_assert_declaration.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

struct static_assert_declaration* parse_static_assert_declaration(
    struct parser_state* s) {
    if (!(accept(s, STATIC_ASSERT) && accept(s, LBRACKET))) {
        return NULL;
    }

    struct const_expr* const_expr = parse_const_expr(s);
    if (!const_expr) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        free_const_expr(const_expr);
        return NULL;
    }

    if (s->it->kind != STRING_LITERAL) {
        expected_token_error(s, STRING_LITERAL);
        free_const_expr(const_expr);
        return NULL;
    }

    const struct str_lit lit = take_str_lit(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);

    if (!(accept(s, RBRACKET) && accept(s, SEMICOLON))) {
        free_const_expr(const_expr);
        free_str_lit(&lit);
        return NULL;
    }

    struct static_assert_declaration* res = mycc_alloc(sizeof *res);
    res->const_expr = const_expr;
    res->err_msg = create_string_literal_node(&lit, loc); 

    return res;
}

static void free_children(struct static_assert_declaration* d) {
    free_const_expr(d->const_expr);
    free_string_literal(&d->err_msg);
}

void free_static_assert_declaration(struct static_assert_declaration* d) {
    free_children(d);

    mycc_free(d);
}

