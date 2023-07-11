#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "testing.h"

#include "util/Str.h"

#ifdef _WIN32
#pragma warning (push)
// warning for comparing string literal addresses
#pragma warning(disable : 4130)
#endif

#define ASSERT(expr)                                                           \
    do {                                                                       \
        if (!(expr)) {                                                         \
            PRINT_ASSERT_ERR("Assertion failure: {Str}", STR_LIT(#expr));      \
        }                                                                      \
    } while (0)

#define ASSERT_BOOL(got, expected)                                             \
    do {                                                                       \
        const bool assert_bool_got = (got);                                    \
        const bool assert_bool_expected = (expected);                          \
        if (assert_bool_got != assert_bool_expected) {                         \
            PRINT_ASSERT_ERR("Expected {bool} but got {bool}",                 \
                             assert_bool_expected,                             \
                             assert_bool_got);                                 \
        }                                                                      \
    } while (0)

#define ASSERT_CHAR(got, expected)                                             \
    do {                                                                       \
        const char assert_char_got = (got);                                    \
        const char assert_char_expected = (expected);                          \
        if (assert_char_got != assert_char_expected) {                         \
            PRINT_ASSERT_ERR("Expected '{char}' but got '{char}'",             \
                             assert_char_expected,                             \
                             assert_char_got);                                 \
        }                                                                      \
    } while (0)

#define ASSERT_INT(got, expected)                                              \
    do {                                                                       \
        const int assert_int_got = (got);                                      \
        const int assert_int_expected = (expected);                            \
        if (assert_int_got != assert_int_expected) {                           \
            PRINT_ASSERT_ERR("Expected {int} but got {int}",                   \
                             assert_int_expected,                              \
                             assert_int_got);                                  \
        }                                                                      \
    } while (0)

#define ASSERT_I64(got, expected)                                              \
    do {                                                                       \
        const int64_t assert_intmax_t_got = (got);                             \
        const int64_t assert_intmax_t_expected = (expected);                   \
        if (assert_intmax_t_got != assert_intmax_t_expected) {                 \
            PRINT_ASSERT_ERR("Expected {i64} but got {i64}",                   \
                             assert_intmax_t_expected,                         \
                             assert_intmax_t_got);                             \
        }                                                                      \
    } while (0)

#define ASSERT_U64(got, expected)                                              \
    do {                                                                       \
        const uint64_t assert_uintmax_t_got = (got);                           \
        const uint64_t assert_uintmax_t_expected = (expected);                 \
        if (assert_uintmax_t_got != assert_uintmax_t_expected) {               \
            PRINT_ASSERT_ERR("Expected {u64} but got {u64}",                   \
                             assert_uintmax_t_expected,                        \
                             assert_uintmax_t_got);                            \
        }                                                                      \
    } while (0)

#define ASSERT_SIZE_T(got_expr, expected_expr)                                 \
    do {                                                                       \
        const size_t assert_size_t_got = (got_expr);                           \
        const size_t assert_size_t_ex = (expected_expr);                       \
        if (assert_size_t_got != assert_size_t_ex) {                           \
            PRINT_ASSERT_ERR("Expected {size_t} but got {size_t}",             \
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
            PRINT_ASSERT_ERR("Expected {float.20} but got {float.20}",         \
                             assert_double_expected,                           \
                             assert_double_got);                               \
        }                                                                      \
    } while (0)

#define ASSERT_STR(got, expected)                                              \
    do {                                                                       \
        const Str assert_str_got = (got);                                      \
        const Str assert_str_expected = (expected);                            \
        if (!Str_eq(assert_str_got, assert_str_expected)) {                    \
            PRINT_ASSERT_ERR("Expected \"{Str}\" but got \"{Str}\"",           \
                             assert_str_expected,                              \
                             assert_str_got);                                  \
        }                                                                      \
    } while (0)

#define ASSERT_NULL(got)                                                       \
    do {                                                                       \
        const void* const assert_null_got = (got);                             \
        if (assert_null_got != NULL) {                                         \
            PRINT_ASSERT_ERR("Expected NULL, but got {ptr}", assert_null_got); \
        }                                                                      \
    } while (0)

#define ASSERT_NOT_NULL(got)                                                   \
    do {                                                                       \
        const void* const assert_not_null_got = (got);                         \
        if (assert_not_null_got == NULL) {                                     \
            PRINT_ASSERT_ERR("Expected non null pointer, but got {ptr}",       \
                             assert_not_null_got);                             \
        }                                                                      \
    } while (0)

#ifdef _WIN32
#pragma warning (pop)
#endif

#endif

