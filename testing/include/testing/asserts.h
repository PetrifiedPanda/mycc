#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "testing/testing.h"

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
        if ((got) != (expected)) {                                             \
            const char* expected_str = expected ? "true" : "false";            \
            const char* got_str = got ? "true" : "false";                      \
            PRINT_ASSERT_ERR("Expected %s but got %s", expected_str, got_str); \
        }                                                                      \
    } while (0)

#define ASSERT_CHAR(got, expected)                                             \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %c but got %c", expected, got);         \
        }                                                                      \
    } while (0)

#define ASSERT_INT(got, expected)                                              \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %d but got %d", expected, got);         \
        }                                                                      \
    } while (0)

#define ASSERT_INTMAX_T(got, expected)                                         \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %jd but got %jd", expected, got);       \
        }                                                                      \
    } while (0)

#define ASSERT_UINTMAX_T(got, expected)                                        \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %ju but got %ju", expected, got);       \
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
        if (fabsl((got) - (expected)) > precision) {                           \
            PRINT_ASSERT_ERR("Expected %.20f but got %.20f", expected, got);   \
        }                                                                      \
    } while (0)

#define ASSERT_STR(got, expected)                                              \
    do {                                                                       \
        if ((expected) == NULL && (got) != NULL) {                             \
            PRINT_ASSERT_ERR("Expected NULL but got %s", got);                 \
        } else if ((expected) != NULL && (got) == NULL) {                      \
            PRINT_ASSERT_ERR("Expected %s but got NULL", expected);            \
        } else if ((got) != NULL && (expected) != NULL) {                      \
            if (strcmp(got, expected) != 0) {                                  \
                PRINT_ASSERT_ERR("Expected %s but got %s", expected, got);     \
            }                                                                  \
        }                                                                      \
    } while (0)

#define ASSERT_NULL(got)                                                       \
    do {                                                                       \
        if ((got) != NULL) {                                                   \
            PRINT_ASSERT_ERR("Expected NULL, but got %p", (void*)got);         \
        }                                                                      \
    } while (0)

#define ASSERT_NOT_NULL(got)                                                   \
    do {                                                                       \
        if ((got) == NULL) {                                                   \
            PRINT_ASSERT_ERR("Expected non null pointer, but got %p",          \
                             (void*)got);                                      \
        }                                                                      \
    } while (0)

#endif

