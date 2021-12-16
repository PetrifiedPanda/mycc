#ifndef TEST_ASSERTS_H
#define TEST_ASSERTS_H

#include <stdlib.h>

#include "token_type.h"
#include "error.h"

#define PRINT_ASSERT_ERR() \
    printf("Assertion failure in %s, %d\n\t", __FILE__, __LINE__)


#define assert_size_t(got, expected)                        \
do {                                                        \
    if ((got) != (expected)) {                              \
        PRINT_ASSERT_ERR();                                 \
        printf("Expected %zu but got %zu", expected, got);  \
        exit(EXIT_FAILURE);                                 \
    }                                                       \
} while (0)

#define assert_str(got, expected)                                                           \
do {                                                                                        \
    if (((expected) == NULL && (got) != NULL) || ((got) == NULL && (expected) != NULL)) {   \
        PRINT_ASSERT_ERR();                                                                 \
        if ((expected) == NULL) {                                                           \
            printf("Expected NULL but got %s", got);                                        \
        } else {                                                                            \
            printf("Expected %s but got NULL", expected);                                   \
        }                                                                                   \
        exit(EXIT_FAILURE);                                                                 \
    } else if ((got) != NULL && (expected) != NULL) {                                       \
        if (strcmp(got, expected) != 0) {                                                   \
            PRINT_ASSERT_ERR();                                                             \
            printf("Expected %s but got %s", expected, got);                                \
            exit(EXIT_FAILURE);                                                             \
        }                                                                                   \
    }                                                                                       \
} while (0)

#define assert_token_type(got, expected)                                                \
do {                                                                                    \
    if ((got) != (expected)) {                                                          \
        PRINT_ASSERT_ERR();                                                             \
        printf("Expected %s but got %s", get_type_str(expected), get_type_str(got));    \
        exit(EXIT_FAILURE);                                                             \
    }                                                                                   \
} while (0)

#define assert_not_null(got)                    \
do {                                            \
    if ((got) == NULL) {                        \
        PRINT_ASSERT_ERR();                     \
        printf("Expected non null pointer");    \
        exit(EXIT_FAILURE);                     \
    }                                           \
} while (0)

#define assert_error(got, expected)                                                                 \
do {                                                                                                \
    if ((got) != (expected)) {                                                                      \
        PRINT_ASSERT_ERR();                                                                         \
        printf("Expected %s but got %s", get_error_type_str(expected), get_error_type_str(got));    \
        exit(EXIT_FAILURE);                                                                         \
    }                                                                                               \
} while (0)
#endif
