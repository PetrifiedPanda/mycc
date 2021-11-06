#include "ast/param_type_list.h"

#include <stdlib.h>
#include <assert.h>

ParamTypeList create_param_type_list(bool is_variadic, ParamList* param_list) {
    assert(param_list);
    ParamTypeList res;

    res.is_variadic = is_variadic;
    res.param_list = param_list;
    return res;
}

void free_param_type_list(ParamTypeList* l) {
    free_param_list(l->param_list);
}

