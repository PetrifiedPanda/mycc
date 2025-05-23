#include "frontend/preproc/preproc.h"

#include "testing/testing.h"
#include "testing/asserts.h"

TEST(unterminated_literal) {
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    {
        PreprocErr err = PreprocErr_create();
        PreprocRes res = preproc_string(STR_LIT("\"this is a string literal"),
                                        STR_LIT("file.c"),
                                        &(PreprocInitialStrings){0},
                                        0,
                                        NULL,
                                        &info,
                                        &err);
        ASSERT_UINT(res.toks.len, 0);
        ASSERT_UINT(res.file_info.len, (uint32_t)0);
        ASSERT_NULL(res.file_info.paths);

        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_UINT(err.base.loc.file_loc.line, (uint32_t)1);
        ASSERT_UINT(err.base.loc.file_loc.index, (uint32_t)1);
        ASSERT_UINT(err.base.loc.file_idx, (uint32_t)0);
        ASSERT(!err.is_char_lit);

        PreprocErr_free(&err);
    }
    {
        PreprocErr err = PreprocErr_create();
        PreprocRes res = preproc_string(
            STR_LIT("int n = 10;\nchar c = \'char literal that cannot exist"),
            STR_LIT("file.c"),
            &(PreprocInitialStrings){0},
            0,
            NULL,
            &info,
            &err);
        ASSERT_UINT(res.toks.len, 0);
        ASSERT_UINT(res.file_info.len, (uint32_t)0);
        ASSERT_NULL(res.file_info.paths);

        ASSERT(err.kind == PREPROC_ERR_UNTERMINATED_LIT);
        ASSERT_UINT(err.base.loc.file_loc.line, (uint32_t)2);
        ASSERT_UINT(err.base.loc.file_loc.index, (uint32_t)10);
        ASSERT_UINT(err.base.loc.file_idx, (uint32_t)0);
        ASSERT(err.is_char_lit);

        PreprocErr_free(&err);
    }
    // TODO: escaped newlines when implemented
}

TEST(invalid_identifier) {
    PreprocErr err = PreprocErr_create();
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocRes res = preproc_string(STR_LIT("int in$valid = 10;"),
                                    STR_LIT("file.c"),
                                    &(PreprocInitialStrings){0},
                                    0,
                                    NULL,
                                    &info,
                                    &err);
    ASSERT_UINT(res.toks.len, 0);
    ASSERT_UINT(res.file_info.len, (uint32_t)0);
    ASSERT_NULL(res.file_info.paths);

    ASSERT(err.kind == PREPROC_ERR_INVALID_ID);

    ASSERT_UINT(err.base.loc.file_idx, (uint32_t)0);
    ASSERT_UINT(err.base.loc.file_loc.line, (uint32_t)1);
    ASSERT_UINT(err.base.loc.file_loc.index, (uint32_t)5);
    ASSERT_STR(StrBuf_as_str(&err.invalid_id), STR_LIT("in$valid"));

    PreprocErr_free(&err);
}

TEST(invalid_number) {
    PreprocErr err = PreprocErr_create();
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocRes res = preproc_string(STR_LIT("int 10in$valid = 10;"),
                                    STR_LIT("file.c"),
                                    &(PreprocInitialStrings){0},
                                    0,
                                    NULL,
                                    &info,
                                    &err);
    ASSERT_UINT(res.toks.len, 0);
    ASSERT_UINT(res.file_info.len, (uint32_t)0);
    ASSERT_NULL(res.file_info.paths);

    ASSERT(err.kind == PREPROC_ERR_INVALID_NUMBER);

    ASSERT_UINT(err.base.loc.file_idx, (uint32_t)0);
    ASSERT_UINT(err.base.loc.file_loc.line, (uint32_t)1);
    ASSERT_UINT(err.base.loc.file_loc.index, (uint32_t)5);
    ASSERT_STR(StrBuf_as_str(&err.invalid_num), STR_LIT("10in$valid"));

    PreprocErr_free(&err);
}

TEST(preproc_token) {
    PreprocErr err = PreprocErr_create();
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocRes res = preproc_string(STR_LIT("int an_int = a ## b;"),
                                    STR_LIT("file.c"),
                                    &(PreprocInitialStrings){0},
                                    0,
                                    NULL,
                                    &info,
                                    &err);
    ASSERT(res.toks.len != 0);
    TokenArr tokens = convert_preproc_tokens(&res.toks, &res.vals, &info, &err);
    ASSERT(tokens.len == 0);

    ASSERT(err.kind == PREPROC_ERR_MISPLACED_PREPROC_TOKEN);

    ASSERT_UINT(err.base.loc.file_idx, (uint32_t)0);
    ASSERT_UINT(err.base.loc.file_loc.line, (uint32_t)1);
    ASSERT_UINT(err.base.loc.file_loc.index, (uint32_t)16);

    PreprocRes_free(&res);

    PreprocErr_free(&err);
}

TEST_SUITE_BEGIN(tokenizer_error){
    REGISTER_TEST(unterminated_literal),
    REGISTER_TEST(invalid_identifier),
    REGISTER_TEST(invalid_number),
    REGISTER_TEST(preproc_token),
} TEST_SUITE_END()
