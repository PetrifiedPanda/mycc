#include "error.h"

#include "preproc/preproc.h"

#include "../test.h"
#include "../test_asserts.h"

TEST(unterminated_literal) {
    {
        struct token* tokens = preproc_string("\"this is a string literal",
                                              "file.c");
        ASSERT_NULL(tokens);
        ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);

        ASSERT_STR(get_error_string(),
                   "file.c(1,1):\n"
                   "String literal not properly terminated");

        clear_last_error();
    }
    {
        struct token* tokens = preproc_string(
            "int n = 10;\nchar c = \'char literal that cannot exist",
            "file.c");
        ASSERT_NULL(tokens);
        ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);

        ASSERT_STR(get_error_string(),
                   "file.c(2,10):\n"
                   "Char literal not properly terminated");

        clear_last_error();
    }
    // TODO: escaped newlines when implemented
}

TEST(invalid_identifier) {
    struct token* tokens = preproc_string("int in$valid = 10;", "file.c");
    ASSERT_NULL(tokens);
    ASSERT_ERROR(get_last_error(), ERR_TOKENIZER);

    ASSERT_STR(get_error_string(),
               "file.c(1,5):\nInvalid identifier: in$valid");

    clear_last_error();
}

TEST_SUITE_BEGIN(tokenizer_error, 2) {
    REGISTER_TEST(unterminated_literal);
    REGISTER_TEST(invalid_identifier);
}
TEST_SUITE_END()
