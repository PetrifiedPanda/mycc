#ifndef MYCC_FRONTEND_STR_LIT_H
#define MYCC_FRONTEND_STR_LIT_H

#include "util/StrBuf.h"

typedef enum {
    STR_LIT_DEFAULT,
    STR_LIT_INCLUDE,
    STR_LIT_U8,
    STR_LIT_LOWER_U,
    STR_LIT_UPPER_U,
    STR_LIT_L,
} StrLitKind;

// TODO: handling of non-default string literals
typedef struct StrLit {
    StrLitKind kind;
    StrBuf contents;
} StrLit;

StrLit convert_to_str_lit(Str spell);

void StrLit_free(const StrLit* lit);

Str StrLitKind_str(StrLitKind kind);

#endif

