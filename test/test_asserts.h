#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>

#include "token_type.h"
#include "error.h"

#define PRINT_ASSERT_ERR(format, ...)                                       \
    fprintf(stderr, "Assertion failure in %s, %d\n\t", __FILE__, __LINE__); \
    fprintf(stderr, format, __VA_ARGS__);                                   \
    exit(EXIT_FAILURE)


#define ASSERT_INT(got, expected)                                       \
    do {                                                                \
        if ((got) != (expected)) {                                      \
            PRINT_ASSERT_ERR("Expected %d but got %d", expected, got);  \
        }                                                               \
    } while (0)


#define ASSERT_SIZE_T(got, expected)                                        \
    do {                                                                    \
        if ((got) != (expected)) {                                          \
            PRINT_ASSERT_ERR("Expected %zu but got %zu", expected, got);    \
        }                                                                   \
    } while (0)


#define ASSERT_STR(got, expected)                                                               \
    do {                                                                                        \
        if (((expected) == NULL && (got) != NULL) || ((got) == NULL && (expected) != NULL)) {   \
            PRINT_ASSERT_ERR("Expected %s but got %s", expected, got);                          \
        } else if ((got) != NULL && (expected) != NULL) {                                       \
            if (strcmp(got, expected) != 0) {                                                   \
                PRINT_ASSERT_ERR("Expected %s but got %s", expected, got);                      \
            }                                                                                   \
        }                                                                                       \
    } while (0)


#define ASSERT_TOKEN_TYPE(got, expected)                                                            \
    do {                                                                                            \
        if ((got) != (expected)) {                                                                  \
            PRINT_ASSERT_ERR("Expected %s but got %s", get_type_str(expected), get_type_str(got));  \
        }                                                                                           \
    } while (0)


#define ASSERT_NOT_NULL(got)                                                        \
    do {                                                                            \
        if ((got) == NULL) {                                                        \
            PRINT_ASSERT_ERR("Expected non null pointer, but got %p", (void*)got);  \
        }                                                                           \
    } while (0)


#define ASSERT_ERROR(got, expected)                                                                             \
    do {                                                                                                        \
        if ((got) != (expected)) {                                                                              \
            PRINT_ASSERT_ERR("Expected %s but got %s", get_error_type_str(expected), get_error_type_str(got));  \
        }                                                                                                       \
    } while (0)

#endif
