#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

#include "preproc/preproc_err.h"

struct preproc_state {
    size_t len, cap;
    struct token* tokens;
    struct preproc_err* err;
};

struct preproc_macro;

struct preproc_macro* find_preproc_macro(const char* spelling);

#include "preproc/preproc_macro.h"

#endif

