#ifndef PREPROC_H
#define PREPROC_H

#include "frontend/file_info.h"
#include "frontend/token.h"

#include "preproc_err.h"

struct preproc_res {
    struct token* toks;
    struct file_info file_info;
};

/**
 * @param path path to file
 *
 * @return preprocessed tokens from this file, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct preproc_res preproc(const char* path, struct preproc_err* err);

#ifdef MYCC_TEST_FUNCTIONALITY
/**
 * @param str a string containing source code
 * @param path path to be entered into the resulting tokens
 *
 * @return preprocessed tokens from this string, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct preproc_res preproc_string(const char* str,
                                  const char* path,
                                  struct preproc_err* err);
#endif

/**
 * Converts the given preprocessor tokens to parser tokens
 */
bool convert_preproc_tokens(struct token* tokens,
                            const struct arch_type_info* info,
                            struct preproc_err* err);

/**
 * Frees tokens before calling convert_preproc_tokens
 */
void free_preproc_res_preproc_tokens(struct preproc_res* res);

/**
 * Frees tokens after calling convert_preproc_tokens
 */
void free_preproc_res(struct preproc_res* res);

#endif

