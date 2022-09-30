#include "util/str.h"

#include "testing/testing.h"
#include "testing/asserts.h"

enum {
    INSERTION_NUM = 100
};

#define PUSH_BACK_TEST_HELPER(str_lit, create_empty)                           \
    do {                                                                       \
        const char raw_str[] = str_lit;                                        \
        static_assert(!create_empty || sizeof raw_str == 1,                    \
                      "If create_empty is true str_lit must be empty");        \
        enum {                                                                 \
            RAW_STR_STRLEN = sizeof raw_str - 1                                \
        };                                                                     \
        struct str str = create_empty ? create_empty_str()                     \
                                      : create_str(RAW_STR_STRLEN, raw_str);   \
        ASSERT_STR(str_get_data(&str), raw_str);                               \
                                                                               \
        char expected[RAW_STR_STRLEN + INSERTION_NUM + 1] = {0};               \
        memcpy(expected, raw_str, RAW_STR_STRLEN);                             \
        for (size_t i = 0; i < INSERTION_NUM; ++i) {                           \
            ASSERT_STR(str_get_data(&str), expected);                          \
            const size_t expected_len = i + RAW_STR_STRLEN;                    \
            ASSERT_SIZE_T(str_len(&str), expected_len);                        \
            const char to_insert = 'a' + i;                                    \
            str_push_back(&str, to_insert);                                    \
            expected[i + RAW_STR_STRLEN] = to_insert;                          \
        }                                                                      \
        ASSERT_STR(str_get_data(&str), expected);                              \
        free_str(&str);                                                        \
    } while (0)

TEST(push_back_to_empty) {
    PUSH_BACK_TEST_HELPER("", true);
}

TEST(push_back_to_nonempty) {
    PUSH_BACK_TEST_HELPER("small str", false);
    PUSH_BACK_TEST_HELPER("looong string very long yes yes yes", false);
    PUSH_BACK_TEST_HELPER("", false);
}

#define CONCAT_TEST_HELPER(lit1, lit2, ex)                                     \
    do {                                                                       \
        const char s1[] = lit1;                                                \
        const char s2[] = lit2;                                                \
        enum {                                                                 \
            S1_LEN = sizeof s1 / sizeof *s1 - 1,                               \
            S2_LEN = sizeof s2 / sizeof *s2 - 1,                               \
        };                                                                     \
                                                                               \
        const char expected[S1_LEN + S2_LEN + 1] = ex;                         \
        ASSERT_SIZE_T(sizeof expected / sizeof *expected - 1,                  \
                      (size_t)S1_LEN + S2_LEN);                                \
                                                                               \
        struct str res = str_concat(S1_LEN, s1, S2_LEN, s2);                   \
        ASSERT_SIZE_T(str_len(&res), (size_t)S1_LEN + S2_LEN);                 \
        ASSERT_STR(str_get_data(&res), expected);                              \
                                                                               \
        free_str(&res);                                                        \
    } while (0)

TEST(concat) {
    CONCAT_TEST_HELPER("Hello, ", "World", "Hello, World");
    CONCAT_TEST_HELPER(
        "Hello, ",
        "World, and others, I don't know who but someone else",
        "Hello, World, and others, I don't know who but someone else");
    CONCAT_TEST_HELPER("aaaaaaa", "bbbbbbbb", "aaaaaaabbbbbbbb");
}

TEST_SUITE_BEGIN(str, 3) {
    REGISTER_TEST(push_back_to_empty);
    REGISTER_TEST(push_back_to_nonempty);
    REGISTER_TEST(concat);
}
TEST_SUITE_END()

