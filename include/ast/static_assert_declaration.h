#ifndef STATIC_ASSERT_DECLARATION_H
#define STATIC_ASSERT_DECLARATION_H

#include "ast/string_literal.h"

struct const_expr;

struct static_assert_declaration {
    struct const_expr* const_expr;
    struct string_literal err_msg;
};

void free_static_assert_declaration(struct static_assert_declaration* d);

#include "ast/const_expr.h"

#endif
