#include "util/StrBuf.h"

#include "testing/testing.h"
#include "testing/asserts.h"

#include "util/macro_util.h"

enum {
    INSERTION_NUM = 100
};

static void insert_test_helper(StrBuf* str,
                               char* expected_buf,
                               uint32_t initial_len) {
    ASSERT_STR(StrBuf_as_str(str), ((Str){initial_len, expected_buf}));
    for (uint32_t i = 0; i < INSERTION_NUM; ++i) {
        const uint32_t expected_len = initial_len + i;
        ASSERT_STR(StrBuf_as_str(str), ((Str){expected_len, expected_buf}));
        ASSERT_UINT(StrBuf_len(str), expected_len);
        const char to_insert = 'a' + (char)i;
        StrBuf_push_back(str, to_insert);
        ASSERT_CHAR(StrBuf_at(str, expected_len), to_insert);
        expected_buf[expected_len] = to_insert;
    }
    ASSERT_STR(StrBuf_as_str(str),
               ((Str){initial_len + INSERTION_NUM, expected_buf}));
    StrBuf_free(str);
}

TEST(push_back_to_empty) {
    StrBuf str = StrBuf_create_empty();

    char expected[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str, expected, 0);
}

TEST(push_back_to_empty_with_cap) {
    enum {
        STATIC_BUF_LEN = ARR_LEN((StrBuf){0}._static_buf) - 1
    };
    enum {
        STR1_CAP = 200
    };
    StrBuf str1 = StrBuf_create_empty_with_cap(STR1_CAP);
    ASSERT_UINT(StrBuf_cap(&str1), STR1_CAP);

    char expected_buf[INSERTION_NUM + 1] = {0};
    insert_test_helper(&str1, expected_buf, 0);

    StrBuf str2 = StrBuf_create_empty_with_cap(4);
    ASSERT_UINT(StrBuf_cap(&str2), STATIC_BUF_LEN);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str2, expected_buf, 0);

    enum {
        STR3_CAP = STATIC_BUF_LEN + 1
    };
    StrBuf str3 = StrBuf_create_empty_with_cap(STR3_CAP);
    ASSERT_UINT(StrBuf_cap(&str3), STR3_CAP);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str3, expected_buf, 0);

    StrBuf str4 = StrBuf_create_empty_with_cap(0);
    ASSERT_UINT(StrBuf_cap(&str4), STATIC_BUF_LEN);
    memset(expected_buf, 0, sizeof expected_buf);
    insert_test_helper(&str4, expected_buf, 0);
}

#define PUSH_BACK_TEST_HELPER(str_lit)                                         \
    do {                                                                       \
        Str raw_str = STR_LIT(str_lit);                                        \
        enum {                                                                 \
            RAW_STR_STRLEN = ARR_LEN(str_lit) - 1                              \
        };                                                                     \
        StrBuf str = StrBuf_create(raw_str);                                   \
        ASSERT_STR(StrBuf_as_str(&str), raw_str);                              \
                                                                               \
        char expected[RAW_STR_STRLEN + INSERTION_NUM + 1] = {0};               \
        memcpy(expected, raw_str.data, raw_str.len);                           \
        insert_test_helper(&str, expected, raw_str.len);                       \
    } while (0)

TEST(push_back_to_nonempty) {
    PUSH_BACK_TEST_HELPER("small str");
    PUSH_BACK_TEST_HELPER("looong string very long yes yes yes");
    PUSH_BACK_TEST_HELPER("");
}

static void copy_test_helper(Str str) {
    StrBuf original = StrBuf_create(str);
    Str orig_str = StrBuf_as_str(&original);
    ASSERT_STR(orig_str, str);
    StrBuf copy = StrBuf_copy(&original);
    ASSERT_STR(orig_str, str);
    Str copy_str = StrBuf_as_str(&copy);
    ASSERT_STR(copy_str, str);
    ASSERT_STR(copy_str, orig_str);
    StrBuf_free(&original);
    StrBuf_free(&copy);
}

static void take_test_helper(Str str) {
    StrBuf original = StrBuf_create(str);
    Str orig_str = StrBuf_as_str(&original);
    ASSERT_STR(orig_str, str);
    StrBuf taken = StrBuf_take(&original);
    ASSERT(!StrBuf_valid(&original));
    Str taken_str = StrBuf_as_str(&taken);
    ASSERT_STR(taken_str, str);
    StrBuf_free(&original);
    StrBuf_free(&taken);
}

TEST(copy_take) {
    const char small_str[] = "Small string";
    const char long_str[] = "Not so small string, that is very long";
    copy_test_helper(STR_LIT(small_str));
    copy_test_helper(STR_LIT(long_str));
    take_test_helper(STR_LIT(small_str));
    take_test_helper(STR_LIT(long_str));

    StrBuf null_str = StrBuf_null();
    ASSERT(!StrBuf_valid(&null_str));
    StrBuf copy = StrBuf_copy(&null_str);
    ASSERT(!StrBuf_valid(&copy));
}

TEST(append_c_str) {
    StrBuf str = StrBuf_create_empty();
#define SHORT_STR "test"
    Str short_str = STR_LIT(SHORT_STR);
    StrBuf_append(&str, short_str);
    ASSERT_STR(StrBuf_as_str(&str), short_str);
    StrBuf_free(&str);

    str = StrBuf_create_empty();
#define LONG_STR "string long enough to not fit into static buffer"
    Str long_str = STR_LIT(LONG_STR);
    StrBuf_append(&str, long_str);
    ASSERT_STR(StrBuf_as_str(&str), long_str);
    StrBuf_free(&str);

    str = StrBuf_create(short_str);
    StrBuf_append(&str, short_str);
    ASSERT_STR(StrBuf_as_str(&str), STR_LIT(SHORT_STR SHORT_STR));

    StrBuf_append(&str, long_str);
    ASSERT_STR(StrBuf_as_str(&str), STR_LIT(SHORT_STR SHORT_STR LONG_STR));
    StrBuf_free(&str);
#undef SHORT_STR
#undef LONG_STR
}

TEST(reserve) {
    StrBuf str = StrBuf_create_empty();
    enum {
        STATIC_BUF_SIZE = sizeof str._static_buf,
        TO_RESERVE_1 = STATIC_BUF_SIZE - 3,
        TO_RESERVE_2 = TO_RESERVE_1 + 12,
        TO_RESERVE_3 = TO_RESERVE_2 + 8
    };
    StrBuf_reserve(&str, TO_RESERVE_1);
    ASSERT_UINT(StrBuf_cap(&str), STATIC_BUF_SIZE - 1);

    char expected[TO_RESERVE_3 + 1] = {0};
    for (uint32_t i = 0; i < TO_RESERVE_1; ++i) {
        StrBuf_push_back(&str, 'a');
        expected[i] = 'a';
    }
    ASSERT_UINT(StrBuf_cap(&str), STATIC_BUF_SIZE - 1);

    Str ex_str = {TO_RESERVE_1, expected};
    ASSERT_STR(StrBuf_as_str(&str), ex_str);
    StrBuf_reserve(&str, TO_RESERVE_2);
    ASSERT_STR(StrBuf_as_str(&str), ex_str);
    ASSERT_UINT(StrBuf_cap(&str), TO_RESERVE_2);

    for (uint32_t i = TO_RESERVE_1; i < TO_RESERVE_2; ++i) {
        StrBuf_push_back(&str, 'b');
        expected[i] = 'b';
    }
    ASSERT_UINT(StrBuf_cap(&str), TO_RESERVE_2);

    ex_str.len = TO_RESERVE_2;
    ASSERT_STR(StrBuf_as_str(&str), ex_str);
    StrBuf_reserve(&str, TO_RESERVE_3);
    ASSERT_STR(StrBuf_as_str(&str), ex_str);
    for (uint32_t i = TO_RESERVE_2; i < TO_RESERVE_3; ++i) {
        StrBuf_push_back(&str, 'c');
        expected[i] = 'c';
    }
    ASSERT_UINT(StrBuf_cap(&str), TO_RESERVE_3);
    ex_str.len = TO_RESERVE_3;
    ASSERT_STR(StrBuf_as_str(&str), ex_str);

    StrBuf_free(&str);
}

#define CONCAT_TEST_HELPER(lit1, lit2, ex)                                     \
    do {                                                                       \
        Str expected = STR_LIT(lit1 lit2);                                     \
        Str s1 = STR_LIT(lit1);                                                \
        Str s2 = STR_LIT(lit2);                                                \
        StrBuf res = StrBuf_concat(s1, s2);                                    \
        ASSERT_UINT(StrBuf_len(&res), (uint32_t)expected.len);                 \
        ASSERT_STR(StrBuf_as_str(&res), expected);                             \
                                                                               \
        StrBuf_free(&res);                                                     \
    } while (0)

TEST(concat) {
    CONCAT_TEST_HELPER("Hello, ", "World", "Hello, World");
    CONCAT_TEST_HELPER(
        "Hello, ",
        "World, and others, I don't know who but someone else",
        "Hello, World, and others, I don't know who but someone else");
    CONCAT_TEST_HELPER("aaaaaaa", "bbbbbbbb", "aaaaaaabbbbbbbb");
#define SMALL_A "ab"
#define SMALL_B "cd"
    StrBuf small_str = StrBuf_concat(STR_LIT(SMALL_A), STR_LIT(SMALL_B));
    ASSERT_STR(StrBuf_as_str(&small_str), STR_LIT(SMALL_A SMALL_B));
    StrBuf_free(&small_str);
#undef SMALL_A
#undef SMALL_B

#define LARGE_A "this is a longer string "
#define LARGE_B "that will be allocated in the dynamic buffer"
    StrBuf large_str = StrBuf_concat(STR_LIT(LARGE_A), STR_LIT(LARGE_B));
    ASSERT_STR(StrBuf_as_str(&large_str), STR_LIT(LARGE_A LARGE_B));
    StrBuf_free(&large_str);
#undef LARGE_A
#undef LARGE_B
}

TEST(shrink_to_fit) {
    {
        enum {
            STATIC_BUF_LEN = ARR_LEN((StrBuf){0}._static_buf),
        };
        StrBuf str = StrBuf_create_empty_with_cap(STATIC_BUF_LEN);
        const uint32_t num_pushs = STATIC_BUF_LEN - 1;
        char ex[STATIC_BUF_LEN] = {0};
        for (uint32_t i = 0; i < num_pushs; ++i) {
            StrBuf_push_back(&str, 'a');
            ex[i] = 'a';
        }
        const Str ex_str = {num_pushs, ex};
        ASSERT_STR(StrBuf_as_str(&str), ex_str);
        ASSERT_UINT(StrBuf_cap(&str), STATIC_BUF_LEN);
        StrBuf_shrink_to_fit(&str);
        ASSERT_UINT(StrBuf_cap(&str), STATIC_BUF_LEN - 1);
        ASSERT_STR(StrBuf_as_str(&str), ex_str);

        StrBuf_free(&str);
    }

    {
        Str c_str = STR_LIT("Some string that is meaningless, but is long "
                            "enough to not fit into the static buffer");
        const uint32_t init_cap = c_str.len * 3;
        StrBuf str = StrBuf_create_empty_with_cap(init_cap);
        ASSERT_UINT(StrBuf_cap(&str), init_cap);
        StrBuf_append(&str, c_str);
        ASSERT_STR(StrBuf_as_str(&str), c_str);
        ASSERT_UINT(StrBuf_len(&str), c_str.len);

        StrBuf_shrink_to_fit(&str);
        ASSERT_UINT(StrBuf_cap(&str), c_str.len);
        ASSERT_STR(StrBuf_as_str(&str), c_str);
        StrBuf_free(&str);
    }
}

TEST(remove_front) {
#define STR_START "hello"
#define STR_END " there"
    Str full_str = STR_LIT(STR_START STR_END);
    StrBuf str = StrBuf_create(full_str);
    ASSERT_STR(StrBuf_as_str(&str), full_str);
    StrBuf_remove_front(&str, ARR_LEN(STR_START) - 1);
    ASSERT_STR(StrBuf_as_str(&str), STR_LIT(STR_END));
    StrBuf_free(&str);

#undef STR_START
#undef STR_END

#define STR_START "and today we are brothers tonight we're all friends\n"
#define STR_END "a moment of peace in a war that never ends"
    full_str = STR_LIT(STR_START STR_END);
    str = StrBuf_create(full_str);
    ASSERT_STR(StrBuf_as_str(&str), full_str);
    StrBuf_remove_front(&str, ARR_LEN(STR_START) - 1);
    ASSERT_STR(StrBuf_as_str(&str), STR_LIT(STR_END));
    StrBuf_free(&str);

#undef STR_START
#undef STR_END
}

TEST(pop_back) {
    char ex_buf[] = "I probably have corona while writing this test. I am "
                    "evidently not that good at resting";
    enum {
        EX_STRLEN = ARR_LEN(ex_buf) - 1
    };

    StrBuf str = StrBuf_create(STR_LIT(ex_buf));
    Str ex_str = {EX_STRLEN, ex_buf};
    ASSERT_STR(StrBuf_as_str(&str), ex_str);

    uint32_t curr_len = EX_STRLEN;
    while (curr_len != 0) {
        --curr_len;
        StrBuf_pop_back(&str);
        ex_buf[curr_len] = '\0';
        ex_str.len -= 1;
        ASSERT_STR(StrBuf_as_str(&str), ex_str);
        ASSERT_UINT(StrBuf_len(&str), curr_len);
    }
    StrBuf_free(&str);
}

TEST_SUITE_BEGIN(StrBuf){
    REGISTER_TEST(push_back_to_empty),
    REGISTER_TEST(push_back_to_empty_with_cap),
    REGISTER_TEST(push_back_to_nonempty),
    REGISTER_TEST(copy_take),
    REGISTER_TEST(append_c_str),
    REGISTER_TEST(reserve),
    REGISTER_TEST(concat),
    REGISTER_TEST(shrink_to_fit),
    REGISTER_TEST(remove_front),
    REGISTER_TEST(pop_back),
} TEST_SUITE_END()

