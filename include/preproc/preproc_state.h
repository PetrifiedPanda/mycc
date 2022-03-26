#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

struct preproc_state {
    size_t len, alloc_len;
    struct token* tokens;
};

#endif

