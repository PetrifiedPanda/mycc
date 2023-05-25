#ifndef EXPECTED_TOKENS_ERR
#define EXPECTED_TOKENS_ERR

#include <stddef.h>
#include <stdio.h>

#include "Token.h"
#include "FileInfo.h"

typedef struct {
    TokenKind got;
    size_t num_expected;
    // size must be increased when throwing error with more expected tokens
    TokenKind expected[21];
} ExpectedTokensErr;

ExpectedTokensErr create_expected_token_err(TokenKind got, TokenKind ex);

ExpectedTokensErr create_expected_tokens_err(TokenKind got, const TokenKind* expected, size_t num_expected);

void print_expected_tokens_err(FILE* f, const ExpectedTokensErr* err);

#endif

