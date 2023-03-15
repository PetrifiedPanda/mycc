#include "frontend/expected_tokens_err.h"

#include <string.h>

#include "util/macro_util.h"

struct expected_tokens_err create_expected_token_err(enum token_kind got, enum token_kind ex) {
    return create_expected_tokens_err(got, &ex, 1);
}
struct expected_tokens_err create_expected_tokens_err(
    enum token_kind got,
    const enum token_kind* expected,
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
    fprintf(out, "Expected token of kind %s", get_kind_str(err->expected[0]));
    for (size_t i = 1; i < err->num_expected; ++i) {
        printf(", %s", get_kind_str(err->expected[i]));
    }

    if (err->got == INVALID) {
        fprintf(out, " but got to end of file");
    } else {
        fprintf(out, " but got token of kind %s", get_kind_str(err->got));
    }
}

