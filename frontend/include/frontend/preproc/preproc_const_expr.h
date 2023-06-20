#ifndef PREPROC_CONST_EXPR_H
#define PREPROC_CONST_EXPR_H

#include <stdbool.h>

#include "frontend/preproc/PreprocState.h"

typedef struct {
    bool valid;
    bool res;
} PreprocConstExprRes;

PreprocConstExprRes evaluate_preproc_const_expr(PreprocState* state, TokenArr* arr);

#endif

