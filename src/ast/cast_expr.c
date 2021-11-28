#include "ast/cast_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

CastExpr* create_cast_expr(TypeName* type_names, size_t len, UnaryExpr* rhs) {
    assert(rhs);
    if (len > 0) {
        assert(type_names);
    } else {
        assert(type_names == NULL);
    }
    CastExpr* res = xmalloc(sizeof(CastExpr));
    res->type_names = type_names;
    res->len = len;
    res->rhs = rhs;
    return res;
}

static void free_children(CastExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_type_name_children(&e->type_names[i]);
    }
    free(e->type_names);
    free_unary_expr(e->rhs);
}

void free_cast_expr(CastExpr* e) {
    free_children(e);
    free(e);
}

