#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stddef.h>
#include <stdio.h>

#include "frontend/Token.h"

#include "frontend/preproc/preproc.h"

#include "util/macro_util.h"

#define ASSERT_TOKEN_KIND(got, expected)                                       \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             TokenKind_str(expected).data,                     \
                             TokenKind_str(got).data);                         \
        }                                                                      \
    } while (0)

#define ASSERT_VALUE_KIND(got, expected)                                       \
    do {                                                                       \
        ValueKind got_kind = got, expected_kind = expected;                    \
        if (got_kind != expected_kind) {                                       \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             ValueKind_str(expected_kind).data,                \
                             ValueKind_str(got_kind).data);                    \
        }                                                                      \
    } while (0)

#define ASSERT_STR_LIT_KIND(got, expected)                                     \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             StrLitKind_str(expected).data,                    \
                             StrLitKind_str(got).data);                        \
        }                                                                      \
    } while (0)

PreprocRes tokenize(Str file);
PreprocRes tokenize_string(Str str, Str file);

void test_compare_files(Str got_file, Str ex_file);

#define STR_BUF_NON_HEAP(lit) StrBuf_non_heap(ARR_LEN(lit) - 1, lit)

StrBuf StrBuf_non_heap(size_t len, const char* str);

#endif

