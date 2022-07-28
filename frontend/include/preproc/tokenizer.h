#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#include "preproc/preproc_state.h"

/**
 * Tokenizes a line of source code for the preprocessor, meaning that
 * keywords are treated as identifiers
 *
 * @param line_num The number of the current line in the file
 * @param comment_not_terminated A pointer to a boolean, that signifies whether
 *        a multi-line comment was started in a previous line
 */
bool tokenize_line(struct token_arr* res,
                   struct preproc_err* err,
                   const char* line,
                   size_t line_num,
                   const char* file,
                   bool* comment_not_terminated);

#endif
