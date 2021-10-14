#ifndef PARAM_TYPE_LIST_H
#define PARAM_TYPE_LIST_H

#include <stdbool.h>

typedef struct ParamList ParamList;

typedef struct ParamTypeList {
    bool is_variadic;
    ParamList* param_list;
} ParamTypeList;

ParamTypeList* create_param_type_list(bool is_variadic, ParamList* param_list);

void free_param_type_list(ParamTypeList* l);

#include "ast/param_list.h"

#endif
