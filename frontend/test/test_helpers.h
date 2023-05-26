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
                             TokenKind_str(expected),                          \
                             TokenKind_str(got));                              \
        }                                                                      \
    } while (0)

#define ASSERT_VALUE_KIND(got, expected)                                       \
    do {                                                                       \
        ValueKind got_kind = got, expected_kind = expected;                    \
        if (got_kind != expected_kind) {                                       \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             ValueKind_str(expected_kind),                     \
                             ValueKind_str(got_kind));                         \
        }                                                                      \
    } while (0)

#define ASSERT_STR_LIT_KIND(got, expected)                                     \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             StrLitKind_str(expected),                         \
                             StrLitKind_str(got));                             \
        }                                                                      \
    } while (0)

PreprocRes tokenize(const char* file);
PreprocRes tokenize_string(const char* str, const char* file);

void test_compare_files(const char* got_file, const char* ex_file);

#define STR_NON_HEAP(lit) str_non_heap(ARR_LEN(lit) - 1, lit)

Str str_non_heap(size_t len, const char* str);

#endif

