#include "ast/param_type_list.h"

#include <assert.h>

static struct param_type_list create_param_type_list(bool is_variadic, struct param_list* param_list) {
    assert(param_list);
    return (struct param_type_list){.is_variadic = is_variadic, .param_list = param_list};
}

struct param_type_list parse_param_type_list(struct parser_state* s) {
    (void)s;
    // TODO:
    return (struct param_type_list) {
        .is_variadic = false,
        .param_list = NULL
    };
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}

