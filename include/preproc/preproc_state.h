#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

struct preproc_state {
    size_t len, cap;
    struct token* tokens;
};

#endif

