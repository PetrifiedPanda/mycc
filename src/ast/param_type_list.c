#include "ast/param_type_list.h"

#include <stdlib.h>
#include <assert.h>

struct param_type_list create_param_type_list(bool is_variadic, struct param_list* param_list) {
    assert(param_list);
    struct param_type_list res;

    res.is_variadic = is_variadic;
    res.param_list = param_list;
    return res;
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}

