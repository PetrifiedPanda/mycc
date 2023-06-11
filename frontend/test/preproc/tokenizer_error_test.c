#include "frontend/preproc/preproc.h"

#include "testing/testing.h"
#include "testing/asserts.h"

TEST(unterminated_literal) {
    {
        PreprocErr err = PreprocErr_create();
        PreprocRes res = preproc_string(STR_LIT("\"this is a string literal"),
                                              STR_LIT("file.c"),
                                              &err);
        ASSERT_NULL(res.toks);
        ASSERT_SIZE_T(res.file_info.len, (size_t)0);
        ASSERT_NULL(res.file_info.paths);
        
        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
        ASSERT(!err.is_char_lit);
        
        PreprocErr_free(&err);
    }
    {
        PreprocErr err = PreprocErr_create();
        PreprocRes res = preproc_string(
            STR_LIT("int n = 10;\nchar c = \'char literal that cannot exist"),
            STR_LIT("file.c"),
            &err);
        ASSERT_NULL(res.toks);
        ASSERT_SIZE_T(res.file_info.len, (size_t)0);
        ASSERT_NULL(res.file_info.paths);
        
        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)2);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)10);
        ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
        ASSERT(err.is_char_lit);
        
        PreprocErr_free(&err);
    }
    // TODO: escaped newlines when implemented
}

TEST(invalid_identifier) {
    PreprocErr err = PreprocErr_create();
    PreprocRes res = preproc_string(STR_LIT("int in$valid = 10;"), STR_LIT("file.c"), &err);
    ASSERT_NULL(res.toks);
    ASSERT_SIZE_T(res.file_info.len, (size_t)0);
    ASSERT_NULL(res.file_info.paths);
    
    ASSERT(err.kind == PREPROC_ERR_INVALID_ID);
    
    ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
    ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
    ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)5);
    ASSERT_STR(StrBuf_as_str(&err.invalid_id), STR_LIT("in$valid"));

    PreprocErr_free(&err);
}

TEST(invalid_number) {
    PreprocErr err = PreprocErr_create();
    PreprocRes res = preproc_string(STR_LIT("int 10in$valid = 10;"), STR_LIT("file.c"), &err);
    ASSERT_NULL(res.toks);
    ASSERT_SIZE_T(res.file_info.len, (size_t)0);
    ASSERT_NULL(res.file_info.paths);
    
    ASSERT(err.kind == PREPROC_ERR_INVALID_NUMBER);

    ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
    ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
    ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)5);
    ASSERT_STR(StrBuf_as_str(&err.invalid_num), STR_LIT("10in$valid"));

    PreprocErr_free(&err);

}

TEST_SUITE_BEGIN(tokenizer_error) {
    REGISTER_TEST(unterminated_literal),
    REGISTER_TEST(invalid_identifier),
    REGISTER_TEST(invalid_number),
}
TEST_SUITE_END()
