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

bool create_token(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename);
void create_token_move(Token* t, TokenType type, char* spelling, SourceLocation loc);

void free_token(Token* t);

#endif
