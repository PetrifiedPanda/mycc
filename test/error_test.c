#include <stdio.h>
#include <string.h>

#include "error.h"

#include "test_asserts.h"

static void clear_err_assert() {
    clear_last_error();
    ASSERT_ERROR(get_last_error(), ERR_NONE);
}

static void compare_buffers(const char* got, const char* expected) {
    ASSERT_SIZE_T(strlen(got), strlen(expected));
    ASSERT_STR(got, expected);
}

TEST(set_error) {
    enum { TEST_BUF_SIZE = 512 };
    char test_buf[TEST_BUF_SIZE];
    {
        const char* format = "Hello %d %f %s";
        int num = 100;
        double d = 3.14;
        const char* str = "test";
        sprintf(test_buf, format, num, d, str);

        set_error(ERR_TOKENIZER, format, num, d, str);

        ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);
        compare_buffers(get_error_string(), test_buf);

        clear_err_assert();
    }

    {
        for (size_t i = 0; i < TEST_BUF_SIZE - 1; ++i) {
            test_buf[i] = 's';
        }
        test_buf[TEST_BUF_SIZE - 1] = '\0';

        set_error(ERR_PARSER, test_buf);

        ASSERT_ERROR(get_last_error(), ERR_PARSER);
        compare_buffers(get_error_string(), test_buf);

        clear_err_assert();
    }
}

TEST(set_error_file) {
    enum { TEST_BUF_SIZE = 512 };
    char test_buf[TEST_BUF_SIZE];

    {
        const char* format = "Hello %s %f %d";
        const char* filename = "test.c";
        struct source_location loc = {1000, 67};

        int num = 34234;
        double d = 2.17;
        const char* str = "test123";
        sprintf(test_buf, "test.c(1000,67):\nHello %s %f %d", str, d, num);

        set_error_file(ERR_TOKENIZER, filename, loc, format, str, d, num);

        ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);
        compare_buffers(get_error_string(), test_buf);

        clear_err_assert();
    }

    {
        const char* filename = "abcdefghij.c";
        struct source_location loc = {10000, 123456789};
        int last = sprintf(test_buf,
                           "%s(%zu,%zu):\n",
                           filename,
                           loc.line,
                           loc.index);

        char print_buf[TEST_BUF_SIZE];
        int print_buf_len = 0;
        for (int i = last; i < TEST_BUF_SIZE - 1; ++i) {
            test_buf[i] = '4';

            print_buf[print_buf_len] = '4';
            ++print_buf_len;
        }
        test_buf[TEST_BUF_SIZE - 1] = '\0';
        print_buf[print_buf_len] = '\0';

        set_error_file(ERR_PARSER, filename, loc, print_buf);

        ASSERT_ERROR(get_last_error(), ERR_PARSER);
        compare_buffers(get_error_string(), test_buf);

        clear_err_assert();
    }
}

TEST(append_error_msg) {
    enum { ARR_SIZE = 6 };
    const char* split_string[ARR_SIZE] =
        {"Hello", " this", " is", " a", " split", " string"};

    const char* expected_steps[5] = {"Hello this",
                                     "Hello this is",
                                     "Hello this is a",
                                     "Hello this is a split",
                                     "Hello this is a split string"};

    set_error(ERR_TOKENIZER, split_string[0]);
    ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);
    compare_buffers(get_error_string(), split_string[0]);

    for (int i = 1; i < ARR_SIZE; ++i) {
        append_error_msg(split_string[i]);
        ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);
        compare_buffers(get_error_string(), expected_steps[i - 1]);
    }

    clear_err_assert();
}

TEST_SUITE_BEGIN(error, 3) {
    REGISTER_TEST(set_error);
    REGISTER_TEST(set_error_file);
    REGISTER_TEST(append_error_msg);
}
TEST_SUITE_END()
