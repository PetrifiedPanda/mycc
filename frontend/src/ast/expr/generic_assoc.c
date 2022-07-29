#include "frontend/ast/expr/generic_assoc.h"

#include <assert.h>

#include "frontend/parser/parser_util.h"

bool parse_generic_assoc_inplace(struct parser_state* s,
                                 struct generic_assoc* res) {
    assert(res);

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
