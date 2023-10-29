#ifndef MYCC_FRONTEND_PREPROC_PREPROC_CONST_EXPR_H
#define MYCC_FRONTEND_PREPROC_PREPROC_CONST_EXPR_H

#include <stdbool.h>

#include "frontend/preproc/PreprocState.h"

typedef struct PreprocConstExprRes {
    bool valid;
    bool res;
} PreprocConstExprRes;

PreprocConstExprRes evaluate_preproc_const_expr(PreprocState* state,
                                                TokenArr* arr,
                                                const ArchTypeInfo* info,
                                                PreprocErr* err);

#endif

