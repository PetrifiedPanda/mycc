#ifndef PREPROC_MACRO_H
#define PREPROC_MACRO_H

#include <stddef.h>

#include "preproc/preproc_token.h"

struct str_and_arg {
    char* spelling;
    size_t arg_num;
};

struct preproc_macro {
    char* spelling;
    size_t num_args;

    size_t len;
    struct str_and_arg* expansion;
};

void expand_macro(struct preproc_macro* m, const char** args, size_t num_args);

#endif

