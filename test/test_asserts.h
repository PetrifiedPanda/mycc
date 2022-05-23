#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token_type.h"

#include "test.h"

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

#define ASSERT_INT(got, expected)                                              \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %d but got %d", expected, got);         \
        }                                                                      \
    } while (0)

#define ASSERT_SIZE_T(got, expected)                                           \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %zu but got %zu", expected, got);       \
        }                                                                      \
    } while (0)

#define ASSERT_STR(got, expected)                                              \
    do {                                                                       \
        if (((expected) == NULL && (got) != NULL)                              \
            || ((got) == NULL && (expected) != NULL)) {                        \
            PRINT_ASSERT_ERR("Expected %s but got %s", expected, got);         \
        } else if ((got) != NULL && (expected) != NULL) {                      \
            if (strcmp(got, expected) != 0) {                                  \
                PRINT_ASSERT_ERR("Expected %s but got %s", expected, got);     \
            }                                                                  \
        }                                                                      \
    } while (0)

#define ASSERT_TOKEN_TYPE(got, expected)                                       \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_type_str(expected),                           \
                             get_type_str(got));                               \
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

