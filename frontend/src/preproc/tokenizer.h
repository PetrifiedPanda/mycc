#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#include "frontend/preproc/PreprocState.h"

/**
 * Tokenizes a line of source code for the preprocessor, meaning that
 * keywords are treated as identifiers
 *
 * @param line_num The number of the current line in the file
 * @param comment_not_terminated A pointer to a boolean, that signifies whether
 *        a multi-line comment was started in a previous line
 */
bool tokenize_line(TokenArr* res, PreprocErr* err, LineInfo* info);

/**
 * @param res The token where the result is written in
 * @param info Will be written with the advanced source location and pointer
 * 
 * @returns false if an error occured
 */
bool next_preproc_token(Token* res, PreprocErr* err, LineInfo* info);

#endif
