#include "ast/param_type_list.h"

#include <assert.h>

struct param_type_list create_param_type_list(bool is_variadic, struct param_list* param_list) {
    assert(param_list);
    return (struct param_type_list){.is_variadic = is_variadic, .param_list = param_list};
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}

