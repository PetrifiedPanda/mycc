#ifndef STATIC_ASSERT_DECLARATION_H
#define STATIC_ASSERT_DECLARATION_H

#include "ast/string_literal.h"

#include "parser/parser_state.h"

struct const_expr;

struct static_assert_declaration {
    struct const_expr* const_expr;
    struct string_literal err_msg;
};

struct static_assert_declaration* parse_static_assert_declaration(
    struct parser_state* s);

void free_static_assert_declaration(struct static_assert_declaration* d);

struct ast_visitor;

bool visit_static_assert_declaration(struct ast_visitor* visitor,
                                     struct static_assert_declaration* d);

#include "ast/expr/const_expr.h"

#endif

