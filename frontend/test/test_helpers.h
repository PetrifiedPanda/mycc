#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "token_type.h"

#define ASSERT_TOKEN_TYPE(got, expected)                                       \
    do {                                                                       \
        if ((got) != (expected)) {                                             \
            PRINT_ASSERT_ERR("Expected %s but got %s",                         \
                             get_type_str(expected),                           \
                             get_type_str(got));                               \
        }                                                                      \
    } while (0)


struct token* tokenize(const char* file);
struct token* tokenize_string(const char* str, const char* file);

void test_compare_files(const char* got_file, const char* ex_file);

#endif

