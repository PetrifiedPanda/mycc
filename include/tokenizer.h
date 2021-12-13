#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"

/**
 * @brief Tokenizes the given source code
 * 
 * @param str The source code to tokenize
 * @param filename The file the source code is in
 * @return struct token* A token array, terminated by a token with type INVALID
 */
struct token* tokenize(const char* str, const char* filename);

/**
 * @brief Frees a token array and contents created by tokenize()
 * 
 * @param tokens A token array that must be terminated by a token with type INVALID
 */
void free_tokenizer_result(struct token* tokens);

#endif

