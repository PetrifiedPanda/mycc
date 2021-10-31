#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"

Token* tokenize(const char* str, const char* filename);

void free_tokenizer_result(Token* tokens);

#endif
