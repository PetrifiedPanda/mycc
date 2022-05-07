#ifndef PREPROC_MACRO_H
#define PREPROC_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "token.h"

#include "preproc/preproc_state.h"

struct token_or_arg {
    bool is_arg;
    union {
        struct {
            bool is_va_args;
            size_t arg_num;
        };
        struct token token;
    };
};

struct preproc_macro {
    char* spelling;

    bool is_func_macro;
    size_t num_args;
    bool is_variadic;

    size_t expansion_len;
    struct token_or_arg* expansion;
};

bool expand_preproc_macro(struct preproc_state* state,
                          const struct preproc_macro* macro,
                          size_t macro_idx,
                          const struct token* macro_end);

#endif

