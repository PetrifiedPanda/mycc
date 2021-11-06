#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "token_type.h"

typedef struct {
    size_t line, index;
} SourceLocation;

typedef struct {
    TokenType type;
    char* spelling;
    char* file;
    SourceLocation source_loc;
} Token;

bool init_token(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename);

void free_token(Token* t);

#endif

