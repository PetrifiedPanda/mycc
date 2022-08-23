#ifndef PREPROC_MACRO_H
#define PREPROC_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/token.h"

#include "preproc_state.h"

struct token_or_arg {
    bool is_arg;
    union {
        size_t arg_num;
        struct token token;
    };
};

struct preproc_macro {
    const char* spell;
    bool is_func_macro;
    size_t num_args;
    bool is_variadic;

    size_t expansion_len;
    struct token_or_arg* expansion;
};

bool expand_preproc_macro(struct preproc_state* state,
                          struct token_arr* res,
                          const struct preproc_macro* macro,
                          size_t macro_idx,
                          size_t macro_end);

struct preproc_macro parse_preproc_macro(struct token_arr* arr,
                                         const char* spell,
                                         struct preproc_err* err);

void free_preproc_macro(struct preproc_macro* m);

#endif

