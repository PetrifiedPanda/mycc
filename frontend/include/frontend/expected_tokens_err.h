#ifndef EXPECTED_TOKENS_ERR
#define EXPECTED_TOKENS_ERR

#include <stddef.h>
#include <stdio.h>

#include "frontend/token_kind.h"
#include "frontend/file_info.h"

struct expected_tokens_err {
    enum token_kind got;
    size_t num_expected;
    // size must be increased when throwing error with more expected tokens
    enum token_kind expected[21];
};

struct expected_tokens_err create_expected_token_err(enum token_kind got, enum token_kind ex);

struct expected_tokens_err create_expected_tokens_err(
    enum token_kind got,
    const enum token_kind* expected,
    size_t num_expected);

void print_expected_tokens_err(FILE* f, const struct expected_tokens_err* err);

#endif

