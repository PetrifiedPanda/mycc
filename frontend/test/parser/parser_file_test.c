#include "testing/asserts.h"

#include "frontend/ast/ast_serializer.h"

#include "util/StrBuf.h"

#include "../test_helpers.h"

static void compare_int_vals(const IntVal* got, const IntVal* ex) {
    ASSERT(got->kind == ex->kind);
    if (IntValKind_is_sint(got->kind)) {
        ASSERT_INT(got->sint_val, ex->sint_val);   
    } else {
        ASSERT_UINT(got->uint_val, ex->uint_val);
    }
}

static void compare_float_vals(const FloatVal* got, const FloatVal* ex) {
    ASSERT(got->kind == ex->kind);
    ASSERT_DOUBLE(got->val, ex->val, 0.);
}

static void compare_str_lits(const StrLit* got, const StrLit* ex) {
    ASSERT(got->kind == ex->kind);
    ASSERT_STR(StrBuf_as_str(&got->contents), StrBuf_as_str(&ex->contents));
}

static void compare_tokens(const TokenArr* got, const TokenArr* ex) {
    ASSERT_UINT(got->len, ex->len);
    ASSERT(memcmp(got->kinds, ex->kinds, sizeof *got->kinds * got->len)
        == 0);

    // TODO: compare better (compare indices then arrays)
    for (uint32_t i = 0; i < got->len; ++i) {
        const uint32_t got_val_idx = got->val_indices[i];
        const uint32_t ex_val_idx = ex->val_indices[i];
        switch (got->kinds[i]) {
            case TOKEN_IDENTIFIER:
                ASSERT_STR(StrBuf_as_str(&got->identifiers[got_val_idx]),
                           StrBuf_as_str(&ex->identifiers[ex_val_idx]));
                break;
            case TOKEN_I_CONSTANT:
                compare_int_vals(&got->int_consts[got_val_idx],
                                 &ex->int_consts[ex_val_idx]);
                break;
            case TOKEN_F_CONSTANT:
                compare_float_vals(&got->float_consts[got_val_idx],
                                   &ex->float_consts[ex_val_idx]);
                break;
            case TOKEN_STRING_LITERAL:
                compare_str_lits(&got->str_lits[got_val_idx],
                                 &ex->str_lits[ex_val_idx]);
                break;
            default:
                break;
        }
    }

    ASSERT(memcmp(got->locs, ex->locs, sizeof *got->locs * got->len)
           == 0);
}
static void compare_with_ex_file(const AST* ast,
                                   const FileInfo* file_info,
                                   CStr path) {
    File f = File_open(path, FILE_READ | FILE_BINARY);
    DeserializeASTRes res = deserialize_ast(f);

    ASSERT_UINT(res.file_info.len, file_info->len);
    for (uint32_t i = 0; i < file_info->len; ++i) {
        const Str got = StrBuf_as_str(&file_info->paths[i]);
        const Str ex = StrBuf_as_str(&res.file_info.paths[i]);
        ASSERT_STR(got, ex);
    }
    compare_tokens(&res.ast.toks, &ast->toks);
    ASSERT_UINT(ast->len, res.ast.len);
    ASSERT(memcmp(ast->kinds, res.ast.kinds, sizeof *ast->kinds * ast->len)
           == 0);
    ASSERT(memcmp(ast->datas, res.ast.datas, sizeof *ast->datas * ast->len)
           == 0);

    ASSERT_UINT(ast->type_data_len, res.ast.type_data_len);
    File_close(f);
    FileInfo_free(&res.file_info);
    AST_free(&res.ast);
}

TEST(no_preproc) {
    const CStr file = CSTR_LIT("../frontend/test/files/no_preproc.c");
    TestPreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    AST ast = parse_ast(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);

    compare_with_ex_file(
        &ast,
        &res.file_info,
        CSTR_LIT("../frontend/test/files/no_preproc.c.binast"));
    TestPreprocRes_free(&res);
    AST_free(&ast);
}

TEST(parser_testfile) {
    const CStr file = CSTR_LIT("../frontend/test/files/parser_testfile.c");
    TestPreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    AST ast = parse_ast(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);

    compare_with_ex_file(
        &ast,
        &res.file_info,
        CSTR_LIT("../frontend/test/files/parser_testfile.c.binast"));
    TestPreprocRes_free(&res);
    AST_free(&ast);
}

TEST(large_testfile) {
    const CStr file = CSTR_LIT("../frontend/test/files/large_testfile.c");
    TestPreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    AST ast = parse_ast(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);

    compare_with_ex_file(
        &ast,
        &res.file_info,
        CSTR_LIT("../frontend/test/files/large_testfile.c.binast"));
    TestPreprocRes_free(&res);
    AST_free(&ast);
}

TEST_SUITE_BEGIN(parser_file){
    REGISTER_TEST(no_preproc),
    REGISTER_TEST(parser_testfile),
    REGISTER_TEST(large_testfile),
} TEST_SUITE_END()
