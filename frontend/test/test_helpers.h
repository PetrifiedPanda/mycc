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
                             get_token_kind_str(expected),                     \
                             get_token_kind_str(got));                         \
        }                                                                      \
    } while (0)

#define ASSERT_INT_VALUE_KIND(got, expected)                                   \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_int_value_kind_str(expected),                 \
                             get_int_value_kind_str(got));                     \
        }                                                                      \
    } while (0)

#define ASSERT_FLOAT_VALUE_KIND(got, expected)                                 \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_float_value_kind_str(expected),               \
                             get_float_value_kind_str(got));                   \
        }                                                                      \
    } while (0)

#define ASSERT_STR_LIT_KIND(got, expected)                                     \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_str_lit_kind_str(expected),                   \
                             get_str_lit_kind_str(got));                       \
        }                                                                      \
    } while (0)

PreprocRes tokenize(const char* file);
PreprocRes tokenize_string(const char* str, const char* file);

void test_compare_files(const char* got_file, const char* ex_file);

#define STR_NON_HEAP(lit) str_non_heap(ARR_LEN(lit) - 1, lit)

Str str_non_heap(size_t len, const char* str);

#endif

