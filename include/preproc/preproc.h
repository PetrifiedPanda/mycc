#ifndef PREPROC_H
#define PREPROC_H

#include "token.h"

struct token* preproc(const char* path);
struct token* preproc_string(const char* str, const char* path);

void free_tokens(struct token* tokens);

#endif

