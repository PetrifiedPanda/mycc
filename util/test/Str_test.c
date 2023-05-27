#include "util/Str.h"

#include "testing/testing.h"
#include "testing/asserts.h"

#include "util/macro_util.h"

enum {
    INSERTION_NUM = 100
};

static void insert_test_helper(Str* str,
                               char* expected_buf,
                               size_t initial_len) {
    ASSERT_STR(Str_get_data(str), expected_buf);
    for (size_t i = 0; i < INSERTION_NUM; ++i) {
        ASSERT_STR(Str_get_data(str), expected_buf);
        const size_t expected_len = i + initial_len;
        ASSERT_SIZE_T(Str_len(str), expected_len);
        const char to_insert = 'a' + (char)i;
        Str_push_back(str, to_insert);
        ASSERT_CHAR(Str_char_at(str, expected_len), to_insert);
        expected_buf[expected_len] = to_insert;
    }
    ASSERT_STR(Str_get_data(str), expected_buf);
    Str_free(str);
}

TEST(push_back_to_empty) {
    Str str = Str_create_empty();

    char expected[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str, expected, 0);
}

TEST(push_back_to_empty_with_cap) {
    enum {
        STATIC_BUF_LEN = ARR_LEN((Str){0}._static_buf) - 1
    };
    enum {
        STR1_CAP = 200
    };
    Str str1 = Str_create_empty_with_cap(STR1_CAP);
    ASSERT_SIZE_T(Str_cap(&str1), STR1_CAP);

    char expected_buf[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str1, expected_buf, 0);

    Str str2 = Str_create_empty_with_cap(4);
    ASSERT_SIZE_T(Str_cap(&str2), STATIC_BUF_LEN);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str2, expected_buf, 0);

    enum {
        STR3_CAP = STATIC_BUF_LEN + 1
    };
    Str str3 = Str_create_empty_with_cap(STR3_CAP);
    ASSERT_SIZE_T(Str_cap(&str3), STR3_CAP);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str3, expected_buf, 0);

    Str str4 = Str_create_empty_with_cap(0);
    ASSERT_SIZE_T(Str_cap(&str4), STATIC_BUF_LEN);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str4, expected_buf, 0);
}

#define PUSH_BACK_TEST_HELPER(str_lit)                                         \
    do {                                                                       \
        const char raw_str[] = str_lit;                                        \
        enum {                                                                 \
            RAW_STR_STRLEN = ARR_LEN(raw_str) - 1                              \
        };                                                                     \
        Str str = Str_create(RAW_STR_STRLEN, raw_str);                         \
        ASSERT_STR(Str_get_data(&str), raw_str);                               \
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
            S1_LEN = ARR_LEN(s1) - 1,                                          \
            S2_LEN = ARR_LEN(s2) - 1,                                          \
        };                                                                     \
                                                                               \
        const char expected[S1_LEN + S2_LEN + 1] = ex;                         \
        static_assert(ARR_LEN(expected) - 1 == (size_t)S1_LEN + S2_LEN,        \
                      "Expected length does not match the length of "          \
                      "concatenated strings");                                 \
                                                                               \
        Str res = Str_concat(S1_LEN, s1, S2_LEN, s2);                          \
        ASSERT_SIZE_T(Str_len(&res), (size_t)S1_LEN + S2_LEN);                 \
        ASSERT_STR(Str_get_data(&res), expected);                              \
                                                                               \
        Str_free(&res);                                                        \
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
    Str original = Str_create(len, s);
    const char* orig_data = Str_get_data(&original);
    ASSERT_STR(orig_data, s);
    Str copy = Str_copy(&original);
    ASSERT_STR(orig_data, s);
    const char* copy_data = Str_get_data(&copy);
    ASSERT_STR(copy_data, s);
    ASSERT_STR(copy_data, orig_data);
    Str_free(&original);
    Str_free(&copy);
}

static void take_test_helper(size_t len, const char* s) {
    Str original = Str_create(len, s);
    const char* orig_data = Str_get_data(&original);
    ASSERT_STR(orig_data, s);
    Str taken = Str_take(&original);
    ASSERT(!Str_is_valid(&original));
    const char* taken_data = Str_get_data(&taken);
    ASSERT_STR(taken_data, s);
    Str_free(&original);
    Str_free(&taken);
}

TEST(copy_take) {
    const char small_str[] = "Small string";
    const char long_str[] = "Not so small string, that is very long";
    enum {
        SMALL_LEN = ARR_LEN(small_str) - 1,
        LONG_LEN = ARR_LEN(long_str) - 1,
    };
    copy_test_helper(SMALL_LEN, small_str);
    copy_test_helper(LONG_LEN, long_str);
    take_test_helper(SMALL_LEN, small_str);
    take_test_helper(LONG_LEN, long_str);

    Str null_str = Str_create_null();
    ASSERT(!Str_is_valid(&null_str));
    Str copy = Str_copy(&null_str);
    ASSERT(!Str_is_valid(&copy));
}

TEST(str_append_c_str) {
    Str str = Str_create_empty();
#define SHORT_STR "test"
    const char short_str[] = SHORT_STR;
    enum {
        SHORT_STR_LEN = ARR_LEN(short_str) - 1
    };
    Str_append_c_str(&str, SHORT_STR_LEN, short_str);
    ASSERT_STR(Str_get_data(&str), short_str);
    Str_free(&str);

    str = Str_create_empty();
#define LONG_STR "string long enough to not fit into static buffer"
    const char long_str[] = LONG_STR;
    enum {
        LONG_STR_LEN = ARR_LEN(long_str) - 1
    };
    Str_append_c_str(&str, LONG_STR_LEN, long_str);
    ASSERT_STR(Str_get_data(&str), long_str);
    Str_free(&str);

    str = Str_create(SHORT_STR_LEN, short_str);
    Str_append_c_str(&str, SHORT_STR_LEN, short_str);
    ASSERT_STR(Str_get_data(&str), SHORT_STR SHORT_STR);

    Str_append_c_str(&str, LONG_STR_LEN, long_str);
    ASSERT_STR(Str_get_data(&str), SHORT_STR SHORT_STR LONG_STR);
    Str_free(&str);
#undef SHORT_STR
#undef LONG_STR
}

TEST(str_reserve) {
    Str str = Str_create_empty();
    enum {
        STATIC_BUF_SIZE = sizeof str._static_buf,
        TO_RESERVE_1 = STATIC_BUF_SIZE - 3,
        TO_RESERVE_2 = TO_RESERVE_1 + 12,
        TO_RESERVE_3 = TO_RESERVE_2 + 8
    };
    Str_reserve(&str, TO_RESERVE_1);
    ASSERT_SIZE_T(Str_cap(&str), STATIC_BUF_SIZE - 1);

    char expected[TO_RESERVE_3 + 1] = {0};
    for (size_t i = 0; i < TO_RESERVE_1; ++i) {
        Str_push_back(&str, 'a');
        expected[i] = 'a';
    }
    ASSERT_SIZE_T(Str_cap(&str), STATIC_BUF_SIZE - 1);

    ASSERT_STR(Str_get_data(&str), expected);
    Str_reserve(&str, TO_RESERVE_2);
    ASSERT_STR(Str_get_data(&str), expected);
    ASSERT_SIZE_T(Str_cap(&str), TO_RESERVE_2);

    for (size_t i = TO_RESERVE_1; i < TO_RESERVE_2; ++i) {
        Str_push_back(&str, 'b');
        expected[i] = 'b';
    }
    ASSERT_SIZE_T(Str_cap(&str), TO_RESERVE_2);

    ASSERT_STR(Str_get_data(&str), expected);
    Str_reserve(&str, TO_RESERVE_3);
    ASSERT_STR(Str_get_data(&str), expected);
    for (size_t i = TO_RESERVE_2; i < TO_RESERVE_3; ++i) {
        Str_push_back(&str, 'c');
        expected[i] = 'c';
    }
    ASSERT_SIZE_T(Str_cap(&str), TO_RESERVE_3);
    ASSERT_STR(Str_get_data(&str), expected);

    Str_free(&str);
}

TEST(str_concat) {
#define SMALL_A "ab"
#define SMALL_B "cd"
    enum {
        SMALL_A_LEN = sizeof SMALL_A - 1,
        SMALL_B_LEN = sizeof SMALL_B - 1
    };
    Str small_str = Str_concat(SMALL_A_LEN, SMALL_A, SMALL_B_LEN, SMALL_B);
    ASSERT_STR(Str_get_data(&small_str), SMALL_A SMALL_B);
    Str_free(&small_str);
#undef SMALL_A
#undef SMALL_B

#define LARGE_A "this is a longer string "
#define LARGE_B "that will be allocated in the dynamic buffer"
    enum {
        LARGE_A_LEN = sizeof LARGE_A - 1,
        LARGE_B_LEN = sizeof LARGE_B - 1
    };
    Str large_str = Str_concat(LARGE_A_LEN, LARGE_A, LARGE_B_LEN, LARGE_B);
    ASSERT_STR(Str_get_data(&large_str), LARGE_A LARGE_B);
    Str_free(&large_str);
#undef LARGE_A
#undef LARGE_B
}

TEST(str_shrink_to_fit) {
    {
        enum {
            STATIC_BUF_LEN = ARR_LEN((Str){0}._static_buf),
        };
        Str str = Str_create_empty_with_cap(STATIC_BUF_LEN);
        const size_t num_pushs = STATIC_BUF_LEN - 1;
        char ex[STATIC_BUF_LEN] = {0};
        for (size_t i = 0; i < num_pushs; ++i) {
            Str_push_back(&str, 'a');
            ex[i] = 'a';
        }
        ASSERT_STR(Str_get_data(&str), ex);
        ASSERT_SIZE_T(Str_cap(&str), STATIC_BUF_LEN);
        Str_shrink_to_fit(&str);
        ASSERT_SIZE_T(Str_cap(&str), STATIC_BUF_LEN - 1);
        ASSERT_STR(Str_get_data(&str), ex);

        Str_free(&str);
    }

    {
        const char c_str[] = "Some string that is meaningless, but is long "
                             "enough to not fit into the static buffer";
        enum {
            C_STR_LEN = ARR_LEN(c_str),
            INIT_CAP = C_STR_LEN * 3,
        };
        Str str = Str_create_empty_with_cap(INIT_CAP);
        ASSERT_SIZE_T(Str_cap(&str), INIT_CAP);
        Str_append_c_str(&str, C_STR_LEN, c_str);
        ASSERT_STR(Str_get_data(&str), c_str);
        ASSERT_SIZE_T(Str_len(&str), C_STR_LEN);

        Str_shrink_to_fit(&str);
        ASSERT_SIZE_T(Str_cap(&str), C_STR_LEN);
        ASSERT_STR(Str_get_data(&str), c_str);
        Str_free(&str);
    }
}

TEST(str_remove_front) {
#define STR_START "hello"
#define STR_END " there"
#define FULL_STR STR_START STR_END
    Str str = Str_create(ARR_LEN(FULL_STR) - 1, FULL_STR);
    ASSERT_STR(Str_get_data(&str), FULL_STR);
    Str_remove_front(&str, ARR_LEN(STR_START) - 1);
    ASSERT_STR(Str_get_data(&str), STR_END);
    Str_free(&str);

#undef STR_START
#undef STR_END
#undef FULL_STR

#define STR_START "and today we are brothers tonight we're all friends\n"
#define STR_END "a moment of peace in a war that never ends"
#define FULL_STR STR_START STR_END

    str = Str_create(ARR_LEN(FULL_STR) - 1, FULL_STR);
    ASSERT_STR(Str_get_data(&str), FULL_STR);
    Str_remove_front(&str, ARR_LEN(STR_START) - 1);
    ASSERT_STR(Str_get_data(&str), STR_END);
    Str_free(&str);

#undef STR_START
#undef STR_END
#undef FULL_STR
}

TEST(str_pop_back) {
    char ex_str[] = "I probably have corona while writing this test. I am "
                    "evidently not that good at resting";
    enum {
        EX_STRLEN = ARR_LEN(ex_str) - 1
    };

    Str str = Str_create(EX_STRLEN, ex_str);
    ASSERT_STR(Str_get_data(&str), ex_str);

    size_t curr_len = EX_STRLEN;
    while (curr_len != 0) {
        --curr_len;
        Str_pop_back(&str);
        ex_str[curr_len] = '\0';
        ASSERT_STR(Str_get_data(&str), ex_str);
        ASSERT_SIZE_T(Str_len(&str), curr_len);
    }
    Str_free(&str);
}

TEST_SUITE_BEGIN(str){
    REGISTER_TEST(push_back_to_empty),
    REGISTER_TEST(push_back_to_empty_with_cap),
    REGISTER_TEST(push_back_to_nonempty),
    REGISTER_TEST(concat),
    REGISTER_TEST(copy_take),
    REGISTER_TEST(str_append_c_str),
    REGISTER_TEST(str_reserve),
    REGISTER_TEST(str_concat),
    REGISTER_TEST(str_shrink_to_fit),
    REGISTER_TEST(str_remove_front),
    REGISTER_TEST(str_pop_back),
} TEST_SUITE_END()
