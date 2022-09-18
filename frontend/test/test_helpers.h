#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "frontend/token_type.h"

#define ASSERT_TOKEN_TYPE(got, expected)                                       \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_type_str(expected),                           \
                             get_type_str(got));                               \
        }                                                                      \
    } while (0)

#define ASSERT_VALUE_TYPE(got, expected)                                       \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_value_type_str(expected),                     \
                             get_value_type_str(got));                         \
        }                                                                      \
    } while (0)

struct preproc_res tokenize(const char* file);
struct preproc_res tokenize_string(const char* str, const char* file);

void test_compare_files(const char* got_file, const char* ex_file);

#endif

