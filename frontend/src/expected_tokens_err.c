#include "frontend/expected_tokens_err.h"

#include <string.h>

#include "util/macro_util.h"

struct expected_tokens_err create_expected_token_err(enum token_type got, enum token_type ex) {
    return create_expected_tokens_err(got, &ex, 1);
}
struct expected_tokens_err create_expected_tokens_err(
    enum token_type got,
    const enum token_type* expected,
    size_t num_expected) {
    assert(expected);
    assert(num_expected >= 1);
    struct expected_tokens_err res = {
        .got = got,
        .num_expected = num_expected,
    };
    assert(num_expected <= ARR_LEN(res.expected));
    const size_t bytes = sizeof *res.expected * num_expected;
    memcpy(res.expected, expected, bytes);
    return res;
}

void print_expected_tokens_err(FILE* out,
                               const struct expected_tokens_err* err) {
    fprintf(out, "Expected token of type %s", get_type_str(err->expected[0]));
    for (size_t i = 1; i < err->num_expected; ++i) {
        printf(", %s", get_type_str(err->expected[i]));
    }

    if (err->got == INVALID) {
        fprintf(out, " but got to enf of file");
    } else {
        fprintf(out, " but got token of type %s", get_type_str(err->got));
    }
}

