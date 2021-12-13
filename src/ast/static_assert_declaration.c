#include "ast/static_assert_declaration.h"

void free_static_assert_declaration(struct static_assert_declaration* d) {
    free_const_expr(d->const_expr);
    free_string_literal(&d->err_msg);
}
