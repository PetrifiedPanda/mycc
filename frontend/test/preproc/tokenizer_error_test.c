#include "frontend/preproc/preproc.h"

#include "testing/testing.h"
#include "testing/asserts.h"

TEST(unterminated_literal) {
    {
        struct preproc_err err = create_preproc_err();
        struct preproc_res res = preproc_string("\"this is a string literal",
                                              "file.c",
                                              &err);
        ASSERT_NULL(res.toks);
        ASSERT_SIZE_T(res.file_info.len, (size_t)0);
        ASSERT_NULL(res.file_info.paths);
        
        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
        ASSERT(!err.is_char_lit);
        
        free_preproc_err(&err);
    }
    {
        struct preproc_err err = create_preproc_err();
        struct preproc_res res = preproc_string(
            "int n = 10;\nchar c = \'char literal that cannot exist",
            "file.c",
            &err);
        ASSERT_NULL(res.toks);
        ASSERT_SIZE_T(res.file_info.len, (size_t)0);
        ASSERT_NULL(res.file_info.paths);
        
        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)2);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)10);
        ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
        ASSERT(err.is_char_lit);
        
        free_preproc_err(&err);
    }
    // TODO: escaped newlines when implemented
}

TEST(invalid_identifier) {
    struct preproc_err err = create_preproc_err();
    struct preproc_res res = preproc_string("int in$valid = 10;", "file.c", &err);
    ASSERT_NULL(res.toks);
    ASSERT_SIZE_T(res.file_info.len, (size_t)0);
    ASSERT_NULL(res.file_info.paths);
    
    ASSERT(err.kind == PREPROC_ERR_INVALID_ID);
    
    ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
    ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
    ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)5);
    ASSERT_STR(str_get_data(&err.invalid_id), "in$valid");

    free_preproc_err(&err);
}

TEST(invalid_number) {
    struct preproc_err err = create_preproc_err();
    struct preproc_res res = preproc_string("int 10in$valid = 10;", "file.c", &err);
    ASSERT_NULL(res.toks);
    ASSERT_SIZE_T(res.file_info.len, (size_t)0);
    ASSERT_NULL(res.file_info.paths);
    
    ASSERT(err.kind == PREPROC_ERR_INVALID_NUMBER);

    ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
    ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
    ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)5);
    ASSERT_STR(str_get_data(&err.invalid_num), "10in$valid");

    free_preproc_err(&err);

}

TEST_SUITE_BEGIN(tokenizer_error, 3) {
    REGISTER_TEST(unterminated_literal);
    REGISTER_TEST(invalid_identifier);
    REGISTER_TEST(invalid_number);
}
TEST_SUITE_END()
