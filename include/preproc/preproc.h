#ifndef PREPROC_H
#define PREPROC_H

#include "token.h"

#include "preproc/preproc_err.h"

/**
 * @param path path to file
 *
 * @return preprocessed tokens from this file, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct token* preproc(const char* path, struct preproc_err* err);

/**
 * @param str a string containing source code
 * @param path path to be entered into the resulting tokens
 *
 * @return preprocessed tokens from this string, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct token* preproc_string(const char* str,
                             const char* path,
                             struct preproc_err* err);

/**
 * Converts the given preprocessor tokens to parser tokens
 */
void convert_preproc_tokens(struct token* tokens);

void free_tokens(struct token* tokens);

#endif

