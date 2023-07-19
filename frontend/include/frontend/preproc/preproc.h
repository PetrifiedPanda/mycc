#ifndef MYCC_FRONTEND_PREPROC_PREPROC_H
#define MYCC_FRONTEND_PREPROC_PREPROC_H

#include "frontend/FileInfo.h"
#include "frontend/Token.h"

#include "PreprocErr.h"

typedef struct {
    Token* toks;
    FileInfo file_info;
} PreprocRes;

/**
 * @param path path to file
 *
 * @return preprocessed tokens from this file, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
PreprocRes preproc(CStr path, const ArchTypeInfo* info, PreprocErr* err);

#ifdef MYCC_TEST_FUNCTIONALITY
/**
 * @param str a string containing source code
 * @param path path to be entered into the resulting tokens
 *
 * @return preprocessed tokens from this string, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
PreprocRes preproc_string(Str str,
                          Str path,
                          const ArchTypeInfo* info,
                          PreprocErr* err);

#endif

/**
 * Converts the given preprocessor tokens to parser tokens
 */
bool convert_preproc_tokens(Token* tokens,
                            const ArchTypeInfo* info,
                            PreprocErr* err);

/**
 * Frees tokens before calling convert_preproc_tokens
 */
void PreprocRes_free_preproc_tokens(PreprocRes* res);

/**
 * Frees tokens after calling convert_preproc_tokens
 */
void PreprocRes_free(PreprocRes* res);

#endif

