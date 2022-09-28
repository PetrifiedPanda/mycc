#include "util/string.h"

#include "testing/testing.h"
#include "testing/asserts.h"

enum {
    INSERTION_NUM = 100
};

#define PUSH_BACK_TEST_HELPER(str_lit, create_empty)                           \
    {                                                                          \
        const char raw_str[] = str_lit;                                        \
        static_assert(!create_empty || sizeof raw_str == 1,                    \
                      "If create_empty is true str_lit must be empty");        \
        enum {                                                                 \
            RAW_STR_STRLEN = sizeof raw_str - 1                                \
        };                                                                     \
        struct string str = create_empty                                       \
                                ? create_empty_string()                        \
                                : create_string(RAW_STR_STRLEN, raw_str);      \
        ASSERT_STR(raw_str, string_get_data(&str));                            \
                                                                               \
        char expected[RAW_STR_STRLEN + INSERTION_NUM + 1] = {0};               \
        memcpy(expected, raw_str, RAW_STR_STRLEN);                             \
        for (size_t i = 0; i < INSERTION_NUM; ++i) {                           \
            ASSERT_STR(string_get_data(&str), expected);                       \
            const size_t expected_len = i + RAW_STR_STRLEN;                    \
            ASSERT_SIZE_T(str._len, expected_len);                             \
            const char to_insert = 'a' + i;                                    \
            string_push_back(&str, to_insert);                                 \
            expected[i + RAW_STR_STRLEN] = to_insert;                          \
        }                                                                      \
        ASSERT_STR(string_get_data(&str), expected);                           \
        free_string(&str);                                                     \
    }

TEST(push_back_to_empty) {
    PUSH_BACK_TEST_HELPER("", true);
}

TEST(push_back_to_nonempty) {
    PUSH_BACK_TEST_HELPER("small str", false);
    PUSH_BACK_TEST_HELPER("looong string very long yes yes yes", false);
    PUSH_BACK_TEST_HELPER("", false);
}

TEST_SUITE_BEGIN(string, 2) {
    REGISTER_TEST(push_back_to_empty);
    REGISTER_TEST(push_back_to_nonempty);
}
TEST_SUITE_END()

