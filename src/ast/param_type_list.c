#include "ast/param_type_list.h"

#include <assert.h>

static struct param_type_list create_param_type_list(
    bool is_variadic,
    struct param_list* param_list) {
    assert(param_list);
    return (struct param_type_list){.is_variadic = is_variadic,
                                    .param_list = param_list};
}

struct param_type_list parse_param_type_list(struct parser_state* s) {
    struct param_list* param_list = parse_param_list(s);
    if (!param_list) {
        return (struct param_type_list){.is_variadic = false,
                                        .param_list = NULL};
    }

    bool is_variadic = false;
    if (s->it->type == COMMA) {
        accept_it(s);
        if (!accept(s, ELLIPSIS)) {
            free_param_list(param_list);
            return (struct param_type_list){.is_variadic = false,
                                            .param_list = NULL};
        }
        is_variadic = true;
    }

    return create_param_type_list(is_variadic, param_list);
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}
