#include "preproc/preproc.h"

#include "../test.h"
#include "../test_asserts.h"

TEST(unterminated_literal) {
    {
        struct preproc_err err = create_preproc_err();
        struct token* tokens = preproc_string("\"this is a string literal",
                                              "file.c",
                                              &err);
        ASSERT_NULL(tokens);
        
        ASSERT(err.type == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.line, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.index, (size_t)1);
        ASSERT_STR(err.base.file, "file.c");
        ASSERT(!err.is_char_lit);
        
        free_preproc_err(&err);
    }
    {
        struct preproc_err err = create_preproc_err();
        struct token* tokens = preproc_string(
            "int n = 10;\nchar c = \'char literal that cannot exist",
            "file.c",
            &err);
        ASSERT_NULL(tokens);
        
        ASSERT(err.type == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.line, (size_t)2);
        ASSERT_SIZE_T(err.base.loc.index, (size_t)10);
        ASSERT_STR(err.base.file, "file.c");
        ASSERT(err.is_char_lit);
        
        free_preproc_err(&err);
    }
    // TODO: escaped newlines when implemented
}

TEST(invalid_identifier) {
    struct preproc_err err = create_preproc_err();
    struct token* tokens = preproc_string("int in$valid = 10;", "file.c", &err);
    ASSERT_NULL(tokens);
    
    ASSERT(err.type == PREPROC_ERR_INVALID_ID);

    ASSERT_STR(err.invalid_id, "in$valid");

    free_preproc_err(&err);
}

TEST_SUITE_BEGIN(tokenizer_error, 2) {
    REGISTER_TEST(unterminated_literal);
    REGISTER_TEST(invalid_identifier);
}
TEST_SUITE_END()
