#ifndef EXPECTED_TOKENS_ERR
#define EXPECTED_TOKENS_ERR

#include <stddef.h>
#include <stdio.h>

#include "frontend/token_type.h"
#include "frontend/file_info.h"

struct expected_tokens_err {
    enum token_type got;
    size_t num_expected;
    // size must be increased when throwing error with more expected tokens
    enum token_type expected[21];
};

struct expected_tokens_err create_expected_token_err(enum token_type got, enum token_type ex);

struct expected_tokens_err create_expected_tokens_err(
    enum token_type got,
    const enum token_type* expected,
    size_t num_expected);

void print_expected_tokens_err(FILE* f, const struct expected_tokens_err* err);

#endif

