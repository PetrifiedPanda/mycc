#ifndef MYCC_FRONTEND_PREPROC_TOKEN_ARR_H
#define MYCC_FRONTEND_PREPROC_TOKEN_ARR_H

#include "frontend/Token.h"

typedef struct {
    uint32_t len, cap;
    Token* tokens;
} TokenArr;

void TokenArr_free(TokenArr* arr);

#endif

