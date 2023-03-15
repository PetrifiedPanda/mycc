#ifndef STR_LIT_H
#define STR_LIT_H

#include "util/str.h"

enum str_lit_kind {
    STR_LIT_DEFAULT,
    STR_LIT_U8,
    STR_LIT_LOWER_U,
    STR_LIT_UPPER_U,
    STR_LIT_L,
};

// TODO: handling of non-default string literals
struct str_lit {
    enum str_lit_kind kind;
    struct str contents;
};

struct str_lit create_str_lit(enum str_lit_kind kind,
                              const struct str* contents);

void free_str_lit(const struct str_lit* lit);

const char* get_str_lit_kind_str(enum str_lit_kind kind);

#endif

