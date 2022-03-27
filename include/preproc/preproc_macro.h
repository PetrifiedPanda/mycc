#ifndef PREPROC_MACRO_H
#define PREPROC_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "token.h"

#include "preproc/preproc_state.h"

struct token_or_num {
    bool is_arg_num;
    union {
        size_t arg_num;
        struct token token;
    };
};

struct preproc_macro {
    char* spelling;

    bool is_func_macro;
    size_t num_args;

    size_t expansion_len;
    struct token_or_num* expansion;
};

bool expand_preproc_macro(struct preproc_state* state, struct preproc_macro* macro, size_t macro_idx);

#endif

