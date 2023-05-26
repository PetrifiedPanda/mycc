#include "frontend/ExpectedTokensErr.h"

#include <string.h>

#include "util/macro_util.h"

ExpectedTokensErr ExpectedTokensErr_create_single_token(TokenKind got, TokenKind ex) {
    return ExpectedTokensErr_create(got, &ex, 1);
}

ExpectedTokensErr ExpectedTokensErr_create(TokenKind got, const TokenKind* expected, size_t num_expected) {
    assert(expected);
    assert(num_expected >= 1);
    ExpectedTokensErr res = {
        .got = got,
        .num_expected = num_expected,
    };
    assert(num_expected <= ARR_LEN(res.expected));
    const size_t bytes = sizeof *res.expected * num_expected;
    memcpy(res.expected, expected, bytes);
    return res;
}

void ExpectedTokensErr_print(FILE* out, const ExpectedTokensErr* err) {
    fprintf(out, "Expected token of kind %s", TokenKind_str(err->expected[0]));
    for (size_t i = 1; i < err->num_expected; ++i) {
        printf(", %s", TokenKind_str(err->expected[i]));
    }

    if (err->got == TOKEN_INVALID) {
        fprintf(out, " but got to end of file");
    } else {
        fprintf(out, " but got token of kind %s", TokenKind_str(err->got));
    }
}

