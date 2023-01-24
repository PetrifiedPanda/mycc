#ifndef READ_AND_TOKENIZE_LINE_H
#define READ_AND_TOKENIZE_LINE_H

#include <stdbool.h>

#include "preproc_state.h"
#include "code_source.h"

bool read_and_tokenize_line(struct preproc_state* state,
                            struct code_source* src);
#endif

