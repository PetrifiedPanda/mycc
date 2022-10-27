#include "util/str.h"

#include "testing/testing.h"
#include "testing/asserts.h"

enum {
    INSERTION_NUM = 100
};

static void insert_test_helper(struct str* str,
                               char* expected_buf,
                               size_t initial_len) {
    ASSERT_STR(str_get_data(str), expected_buf);
    for (size_t i = 0; i < INSERTION_NUM; ++i) {
        ASSERT_STR(str_get_data(str), expected_buf);
        const size_t expected_len = i + initial_len;
        ASSERT_SIZE_T(str_len(str), expected_len);
        const char to_insert = 'a' + (char)i;
        str_push_back(str, to_insert);
        ASSERT_CHAR(str_char_at(str, expected_len), to_insert);
        expected_buf[expected_len] = to_insert;
    }
    ASSERT_STR(str_get_data(str), expected_buf);
    free_str(str);
}

TEST(push_back_to_empty) {
    struct str str = create_empty_str();

    char expected[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str, expected, 0);
}

TEST(push_back_to_empty_with_cap) {
    struct str str1 = create_empty_str_with_cap(200);
    
    char expected_buf[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str1, expected_buf, 0);

    struct str str2 = create_empty_str_with_cap(4);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str2, expected_buf, 0);
    
    struct str str3 = create_empty_str_with_cap(23);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str3, expected_buf, 0);

    struct str str4 = create_empty_str_with_cap(0);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str4, expected_buf, 0);
}

#define PUSH_BACK_TEST_HELPER(str_lit)                                         \
    do {                                                                       \
        const char raw_str[] = str_lit;                                        \
        enum {                                                                 \
            RAW_STR_STRLEN = sizeof raw_str - 1                                \
        };                                                                     \
        struct str str = create_str(RAW_STR_STRLEN, raw_str);                  \
        ASSERT_STR(str_get_data(&str), raw_str);                               \
                                                                               \
        char expected[RAW_STR_STRLEN + INSERTION_NUM + 1] = {0};               \
        memcpy(expected, raw_str, RAW_STR_STRLEN);                             \
        insert_test_helper(&str, expected, RAW_STR_STRLEN);                    \
    } while (0)

TEST(push_back_to_nonempty) {
    PUSH_BACK_TEST_HELPER("small str");
    PUSH_BACK_TEST_HELPER("looong string very long yes yes yes");
    PUSH_BACK_TEST_HELPER("");
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
        static_assert(sizeof expected / sizeof *expected - 1                   \
                          == (size_t)S1_LEN + S2_LEN,                          \
                      "Expected length does not match the length of "          \
                      "concatenated strings");                                 \
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

static void copy_test_helper(size_t len, const char* s) {
    struct str original = create_str(len, s);
    const char* orig_data = str_get_data(&original);
    ASSERT_STR(orig_data, s);
    struct str copy = str_copy(&original);
    ASSERT_STR(orig_data, s);
    const char* copy_data = str_get_data(&copy);
    ASSERT_STR(copy_data, s);
    ASSERT_STR(copy_data, orig_data);
    free_str(&original);
    free_str(&copy);
}

static void take_test_helper(size_t len, const char* s) {
    struct str original = create_str(len, s);
    const char* orig_data = str_get_data(&original);
    ASSERT_STR(orig_data, s);
    struct str taken = str_take(&original);
    ASSERT(!str_is_valid(&original));
    const char* taken_data = str_get_data(&taken);
    ASSERT_STR(taken_data, s);
    free_str(&original);
    free_str(&taken);
}

TEST(copy_take) {
    const char small_str[] = "Small string";
    const char long_str[] = "Not so small string, that is very long";
    enum {
        SMALL_LEN = sizeof small_str / sizeof *small_str - 1,
        LONG_LEN = sizeof long_str / sizeof *long_str - 1,
    };
    copy_test_helper(SMALL_LEN, small_str);
    copy_test_helper(LONG_LEN, long_str);
    take_test_helper(SMALL_LEN, small_str);
    take_test_helper(LONG_LEN, long_str);

    struct str null_str = create_null_str();
    ASSERT(!str_is_valid(&null_str));
    struct str copy = str_copy(&null_str);
    ASSERT(!str_is_valid(&copy));
}

TEST_SUITE_BEGIN(str, 5) {
    REGISTER_TEST(push_back_to_empty);
    REGISTER_TEST(push_back_to_empty_with_cap);
    REGISTER_TEST(push_back_to_nonempty);
    REGISTER_TEST(concat);
    REGISTER_TEST(copy_take);
}
TEST_SUITE_END()

