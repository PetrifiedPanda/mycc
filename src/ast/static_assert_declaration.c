#include "ast/static_assert_declaration.h"

#include "util.h"

#include "parser/parser_util.h"

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

    if (s->it->type != STRING_LITERAL) {
        expected_token_error(STRING_LITERAL, s->it);
        free_const_expr(const_expr);
        return NULL;
    }

    char* spell = take_spelling(s->it);
    accept_it(s);

    if (!(accept(s, RBRACKET) && accept(s, SEMICOLON))) {
        free_const_expr(const_expr);
        free(spell);
        return NULL;
    }

    struct static_assert_declaration* res = xmalloc(
        sizeof(struct static_assert_declaration));
    res->const_expr = const_expr;
    res->err_msg = (struct string_literal){.spelling = spell};

    return res;
}

static void free_children(struct static_assert_declaration* d) {
    free_const_expr(d->const_expr);
    free_string_literal(&d->err_msg);
}

void free_static_assert_declaration(struct static_assert_declaration* d) {
    free_children(d);

    free(d);
}
