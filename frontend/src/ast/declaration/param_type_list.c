#include "frontend/ast/declaration/param_type_list.h"

#include <assert.h>

bool parse_param_type_list(struct parser_state* s,
                           struct param_type_list* res) {
    res->param_list = parse_param_list(s);
    if (!res->param_list) {
        return false;
    }

    res->is_variadic = false;
    if (s->it->type == COMMA) {
        accept_it(s);
        if (!accept(s, ELLIPSIS)) {
            free_param_list(res->param_list);
            return false;
        }
        res->is_variadic = true;
    }

    return true;
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}
