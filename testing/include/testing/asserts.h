#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "testing.h"

#include "util/Str.h"

#ifdef WIN32
// warning for comparing string literal addresses
#pragma warning(disable : 4130)
#endif

#define ASSERT(expr)                                                           \
    do {                                                                       \
        if (!(expr)) {                                                         \
            PRINT_ASSERT_ERR("Assertion failure: %s", #expr);                  \
        }                                                                      \
    } while (0)

#define ASSERT_BOOL(got, expected)                                             \
    do {                                                                       \
        const bool assert_bool_got = (got);                                    \
        const bool assert_bool_expected = (expected);                          \
        if (assert_bool_got != assert_bool_expected) {                         \
            const char* assert_bool_expected_str = assert_bool_expected        \
                                                       ? "true"                \
                                                       : "false";              \
            const char* assert_bool_got_str = assert_bool_got ? "true"         \
                                                              : "false";       \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             assert_bool_expected_str,                         \
                             assert_bool_got_str);                             \
        }                                                                      \
    } while (0)

#define ASSERT_CHAR(got, expected)                                             \
    do {                                                                       \
        const char assert_char_got = (got);                                    \
        const char assert_char_expected = (expected);                          \
        if (assert_char_got != assert_char_expected) {                         \
            PRINT_ASSERT_ERR("Expected %c but got %c",                         \
                             assert_char_expected,                             \
                             assert_char_got);                                 \
        }                                                                      \
    } while (0)

#define ASSERT_INT(got, expected)                                              \
    do {                                                                       \
        const int assert_int_got = (got);                                      \
        const int assert_int_expected = (expected);                            \
        if (assert_int_got != assert_int_expected) {                           \
            PRINT_ASSERT_ERR("Expected %d but got %d",                         \
                             assert_int_expected,                              \
                             assert_int_got);                                  \
        }                                                                      \
    } while (0)

#define ASSERT_INTMAX_T(got, expected)                                         \
    do {                                                                       \
        const intmax_t assert_intmax_t_got = (got);                            \
        const intmax_t assert_intmax_t_expected = (expected);                  \
        if (assert_intmax_t_got != assert_intmax_t_expected) {                 \
            PRINT_ASSERT_ERR("Expected %jd but got %jd",                       \
                             assert_intmax_t_expected,                         \
                             assert_intmax_t_got);                             \
        }                                                                      \
    } while (0)

#define ASSERT_UINTMAX_T(got, expected)                                        \
    do {                                                                       \
        const intmax_t assert_uintmax_t_got = (got);                           \
        const intmax_t assert_uintmax_t_expected = (expected);                 \
        if (assert_uintmax_t_got != assert_uintmax_t_expected) {               \
            PRINT_ASSERT_ERR("Expected %ju but got %ju",                       \
                             assert_uintmax_t_expected,                        \
                             assert_uintmax_t_got);                            \
        }                                                                      \
    } while (0)

#define ASSERT_SIZE_T(got_expr, expected_expr)                                 \
    do {                                                                       \
        const size_t assert_size_t_got = (got_expr);                           \
        const size_t assert_size_t_ex = (expected_expr);                       \
        if (assert_size_t_got != assert_size_t_ex) {                           \
            PRINT_ASSERT_ERR("Expected %zu but got %zu",                       \
                             assert_size_t_ex,                                 \
                             assert_size_t_got);                               \
        }                                                                      \
    } while (0)

#define ASSERT_DOUBLE(got, expected, precision)                                \
    do {                                                                       \
        const double assert_double_got = (got);                                \
        const double assert_double_expected = (expected);                      \
        const double assert_double_precision = (precision);                    \
        if (fabsl(assert_double_got - assert_double_expected)                  \
            > assert_double_precision) {                                       \
            PRINT_ASSERT_ERR("Expected %.20f but got %.20f",                   \
                             assert_double_expected,                           \
                             assert_double_got);                               \
        }                                                                      \
    } while (0)

#define ASSERT_STR(got, expected)                                              \
    do {                                                                       \
        const Str assert_str_got = (got);                                      \
        const Str assert_str_expected = (expected);                            \
        if (!Str_eq(assert_str_got, assert_str_expected)) {                    \
            PRINT_ASSERT_ERR("Expected \"%s\" but got \"%s\"",                 \
                             assert_str_expected.data,                         \
                             assert_str_got.data);                             \
        }                                                                      \
    } while (0)

#define ASSERT_NULL(got)                                                       \
    do {                                                                       \
        const void* const assert_null_got = (got);                             \
        if (assert_null_got != NULL) {                                         \
            PRINT_ASSERT_ERR("Expected NULL, but got %p", assert_null_got);    \
        }                                                                      \
    } while (0)

#define ASSERT_NOT_NULL(got)                                                   \
    do {                                                                       \
        const void* const assert_not_null_got = (got);                         \
        if (assert_not_null_got == NULL) {                                     \
            PRINT_ASSERT_ERR("Expected non null pointer, but got %p",          \
                             assert_not_null_got);                             \
        }                                                                      \
    } while (0)

#endif

