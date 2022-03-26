#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#include "preproc/preproc_state.h"

bool tokenize_line(struct preproc_state* res, const char* line, size_t line_num, const char* file, bool* comment_not_terminated);

#endif

