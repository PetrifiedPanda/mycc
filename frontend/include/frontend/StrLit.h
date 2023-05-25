#ifndef STR_LIT_H
#define STR_LIT_H

#include "util/Str.h"

typedef enum {
    STR_LIT_DEFAULT,
    STR_LIT_INCLUDE,
    STR_LIT_U8,
    STR_LIT_LOWER_U,
    STR_LIT_UPPER_U,
    STR_LIT_L,
} StrLitKind;

// TODO: handling of non-default string literals
typedef struct {
    StrLitKind kind;
    Str contents;
} StrLit;

StrLit convert_to_str_lit(Str* spell);
StrLit create_str_lit(StrLitKind kind, const Str* contents);

void free_str_lit(const StrLit* lit);

const char* get_str_lit_kind_str(StrLitKind kind);

#endif

