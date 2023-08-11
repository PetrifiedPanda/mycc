#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_deserializer.h"
#include "frontend/ast/compare_asts.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void compare_with_ex_file(const TranslationUnit* got,
                                 const FileInfo* file_info,
                                 Str ex_filename) {
    FILE* ex_file = fopen(ex_filename.data, "rb");
    ASSERT(ex_file);
    DeserializeAstRes expected = deserialize_ast((File){ex_file});
    ASSERT(expected.is_valid);
    ASSERT(fclose(ex_file) == 0);

    ASSERT(expected.file_info.len == file_info->len);
    ASSERT(compare_asts(got, file_info, &expected.tl, &expected.file_info));
    TranslationUnit_free(&expected.tl);
    FileInfo_free(&expected.file_info);
}

TEST(no_preproc) {
    CStr file = CSTR_LIT("../frontend/test/files/no_preproc.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(&tl,
                         &res.file_info,
                         STR_LIT("../frontend/test/files/no_preproc.c.binast"));

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST(parser_testfile) {
    CStr file = CSTR_LIT("../frontend/test/files/parser_testfile.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(
        &tl,
        &res.file_info,
        STR_LIT("../frontend/test/files/parser_testfile.c.binast"));

    Str tmp_filename = STR_LIT("tmp.ast");
    FILE* tmp_file = fopen(tmp_filename.data, "w");
    ASSERT_NOT_NULL(tmp_file);
    dump_ast(&tl, &res.file_info, (File){tmp_file});

    ASSERT_INT(fclose(tmp_file), 0);

    test_compare_files(
        Str_c_str(tmp_filename),
        Str_c_str(STR_LIT("../frontend/test/files/parser_testfile.c.ast")));

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST(large_testfile) {
    CStr file = CSTR_LIT("../frontend/test/files/large_testfile.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(
        &tl,
        &res.file_info,
        STR_LIT("../frontend/test/files/large_testfile.c.binast"));

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST_SUITE_BEGIN(parser_file){
    REGISTER_TEST(no_preproc),
    REGISTER_TEST(parser_testfile),
    REGISTER_TEST(large_testfile),
} TEST_SUITE_END()
