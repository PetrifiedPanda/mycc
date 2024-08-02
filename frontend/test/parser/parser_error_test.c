#include "frontend/preproc/preproc.h"

#include "testing/asserts.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

TEST(redefine_typedef_error_2) {
    TestPreprocRes preproc_res = tokenize_string(STR_LIT("typedef int MyInt;\n"
                                                     "typedef char MyInt;\n"),
                                             STR_LIT("a file"));

    ParserErr err = ParserErr_create();

    AST ast = parse_ast(&preproc_res.toks, &err);
    ASSERT_UINT(ast.len, 0);
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    const uint32_t val_idx = ast.toks.val_indices[err.err_token_idx];
    const Str got_spell = StrBuf_as_str(
        &ast.toks.identifiers[val_idx]);
    ASSERT_STR(got_spell, STR_LIT("MyInt"));

    AST_free(&ast);
    TestPreprocRes_free(&preproc_res);
}

TEST(redefine_typedef_error) {
    TestPreprocRes preproc_res = tokenize_string(STR_LIT("typedef int MyInt;"),
                                             STR_LIT("a file"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    const StrBuf spell = STR_BUF_NON_HEAP("MyInt");
    ParserState_register_typedef(&s, &spell, UINT32_MAX);

    bool found_typedef = false;
    DeclarationSpecs res;
    ASSERT(!parse_declaration_specs(&s, &res, &found_typedef));
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    const uint32_t val_idx = s._arr.val_indices[err.err_token_idx];
    const Str got_spell = StrBuf_as_str(
        &s._arr.identifiers[val_idx]);
    ASSERT_STR(got_spell, STR_LIT("MyInt"));

    ParserState_free(&s);
    TokenArr_free(&s._arr);
    TestPreprocRes_free(&preproc_res);
}

typedef struct {
    ParserErr err;
    TokenArr toks;
} ParserErrAndToks;

static ParserErrAndToks parse_type_specs_until_fail(Str code) {
    TestPreprocRes preproc_res = tokenize_string(code, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    TypeSpecs specs = TypeSpecs_create();

    while (update_type_specs(&s, &specs))
        ;

    ParserState_free(&s);
    TestPreprocRes_free(&preproc_res);
    TypeSpecs_free_children(&specs);
    return (ParserErrAndToks){err, s._arr};
}

static void check_too_many_long(Str code) {
    ParserErrAndToks res = parse_type_specs_until_fail(code);

    ASSERT(res.err.kind == PARSER_ERR_TOO_MUCH_LONG);
    TokenArr_free(&res.toks);
}

static void check_cannot_combine_type_specs(Str code,
                                            TokenKind prev_spec,
                                            TokenKind type_spec,
                                            SourceLoc ex_loc) {
    ParserErrAndToks res = parse_type_specs_until_fail(code);

    ASSERT(res.err.kind == PARSER_ERR_INCOMPATIBLE_TYPE_SPECS);
    ASSERT_TOKEN_KIND(res.err.prev_type_spec, prev_spec);
    ASSERT_TOKEN_KIND(res.err.type_spec, type_spec);
    const SourceLoc loc = res.toks.locs[res.err.err_token_idx];
    ASSERT_UINT(loc.file_idx, ex_loc.file_idx);
    ASSERT_UINT(loc.file_loc.line, ex_loc.file_loc.line);
    ASSERT_UINT(loc.file_loc.index, ex_loc.file_loc.index);

    TokenArr_free(&res.toks);
}

TEST(type_spec_error) {
    check_too_many_long(STR_LIT("long long long"));
    check_too_many_long(STR_LIT("int long long long"));
    check_too_many_long(STR_LIT("long long int long"));
    check_cannot_combine_type_specs(STR_LIT("long short"),
                                    TOKEN_LONG,
                                    TOKEN_SHORT,
                                    (SourceLoc){0, {1, 6}});
    check_cannot_combine_type_specs(STR_LIT("long short long"),
                                    TOKEN_LONG,
                                    TOKEN_SHORT,
                                    (SourceLoc){0, {1, 6}});
    check_cannot_combine_type_specs(STR_LIT("short long"),
                                    TOKEN_SHORT,
                                    TOKEN_LONG,
                                    (SourceLoc){0, {1, 7}});
    check_cannot_combine_type_specs(STR_LIT("unsigned signed"),
                                    TOKEN_UNSIGNED,
                                    TOKEN_SIGNED,
                                    (SourceLoc){0, {1, 10}});
    check_cannot_combine_type_specs(STR_LIT("signed unsigned"),
                                    TOKEN_SIGNED,
                                    TOKEN_UNSIGNED,
                                    (SourceLoc){0, {1, 8}});
    // TODO: DISALLOWED_TYPE_QUALS
}

static void check_expected_semicolon_jump_statement(Str spell) {
    TestPreprocRes preproc_res = tokenize_string(spell, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    Statement* res = parse_statement(&s);
    ASSERT_NULL(res);

    ASSERT(err.kind == PARSER_ERR_EXPECTED_TOKENS);
    const ExpectedTokensErr* ex_tokens_err = &err.expected_tokens_err;
    ASSERT_UINT(ex_tokens_err->num_expected, (uint32_t)1);
    ASSERT_TOKEN_KIND(ex_tokens_err->expected[0], TOKEN_SEMICOLON);
    ASSERT_TOKEN_KIND(ex_tokens_err->got, TOKEN_INVALID);

    TestPreprocRes_free(&preproc_res);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
}

TEST(jump_statement_error) {
    check_expected_semicolon_jump_statement(STR_LIT("continue"));
    check_expected_semicolon_jump_statement(STR_LIT("break"));
    check_expected_semicolon_jump_statement(STR_LIT("return an_identifier"));
    check_expected_semicolon_jump_statement(STR_LIT("return *id += (int)100"));
}

static void check_expected_semicolon(Str spell,
                                     TokenKind got,
                                     FileLoc err_loc) {
    TestPreprocRes preproc_res = tokenize_string(spell, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    AST ast = parse_ast(&preproc_res.toks, &err);

    ASSERT(err.kind == PARSER_ERR_EXPECTED_TOKENS);
    const SourceLoc loc = ast.toks.locs[err.err_token_idx];
    ASSERT_UINT(loc.file_idx, 0);
    ASSERT_UINT(loc.file_loc.line, err_loc.line);
    ASSERT_UINT(loc.file_loc.index, err_loc.index);
    const ExpectedTokensErr* ex_tokens = &err.expected_tokens_err;
    ASSERT_UINT(ex_tokens->num_expected, 1);
    ASSERT_TOKEN_KIND(ex_tokens->expected[0], TOKEN_SEMICOLON);
    ASSERT_TOKEN_KIND(ex_tokens->got, got);

    TestPreprocRes_free(&preproc_res);
    AST_free(&ast);
}

TEST(expected_semicolon) {
    check_expected_semicolon(STR_LIT("int main() {\n"
                                     "  return 0\n"
                                     "}"), TOKEN_RBRACE, (FileLoc){3, 1});
    check_expected_semicolon(STR_LIT("int main() {\n"
                                     "  for (int i = 0; i < 10; ++i) {\n"
                                     "      break\n"
                                     "  }\n"
                                     "}"), TOKEN_RBRACE, (FileLoc){4, 3});
    check_expected_semicolon(STR_LIT("int main() {\n"
                                     "  for (int i = 0; i < 10; ++i) {\n"
                                     "      continue\n"
                                     "  }\n"
                                     "}"), TOKEN_RBRACE, (FileLoc){4, 3});
    /* TODO: fix this (Error token is wrong for some reason)
    check_expected_semicolon(STR_LIT("typedef struct {\n"
                                     "  int n, m;\n"
                                     "} Test\n"
                                     "int main(void) {\n"
                                     "}"),
                             TOKEN_INT,
                             (FileLoc){4, 1});
    */
}

TEST_SUITE_BEGIN(parser_error){
    REGISTER_TEST(redefine_typedef_error_2),
    REGISTER_TEST(redefine_typedef_error),
    REGISTER_TEST(type_spec_error),
    REGISTER_TEST(jump_statement_error),
    REGISTER_TEST(expected_semicolon),
} TEST_SUITE_END()
