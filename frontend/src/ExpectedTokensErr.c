#include "frontend/ExpectedTokensErr.h"

#include <string.h>

#include "util/macro_util.h"

ExpectedTokensErr ExpectedTokensErr_create_single_token(TokenKind got, TokenKind ex) {
    return ExpectedTokensErr_create(got, &ex, 1);
}

ExpectedTokensErr ExpectedTokensErr_create(TokenKind got, const TokenKind* expected, uint32_t num_expected) {
    assert(expected);
    assert(num_expected >= 1);
    ExpectedTokensErr res = {
        .got = got,
        .num_expected = num_expected,
    };
    assert(num_expected <= ARR_LEN(res.expected));
    const uint32_t bytes = sizeof *res.expected * num_expected;
    memcpy(res.expected, expected, bytes);
    return res;
}

void ExpectedTokensErr_print(File out, const ExpectedTokensErr* err) {
    File_printf(out, "Expected token of kind {Str}", TokenKind_str(err->expected[0]));
    for (uint32_t i = 1; i < err->num_expected; ++i) {
        File_printf(out, ", {Str}", TokenKind_str(err->expected[i]));
    }

    if (err->got == TOKEN_INVALID) {
        File_put_str_val(STR_LIT(" but got to end of file"), out);
    } else {
        File_printf(out, " but got token of kind {Str}", TokenKind_str(err->got));
    }
}

