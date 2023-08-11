#include "frontend/preproc/preproc.h"

#include "testing/asserts.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

TEST(redefine_typedef_error) {
    PreprocRes preproc_res = tokenize_string(STR_LIT("typedef int MyInt;"),
                                             STR_LIT("a file"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    const StrBuf spell = STR_BUF_NON_HEAP("MyInt");
    ParserState_register_typedef(&s, &spell, (uint32_t)-1);

    bool found_typedef = false;
    DeclarationSpecs res; 
    ASSERT(!parse_declaration_specs(&s, &res, &found_typedef));
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    const Str got_spell = StrBuf_as_str(&s._arr.vals[err.err_token_idx].spelling);
    ASSERT_STR(got_spell, STR_LIT("MyInt"));

    ParserState_free(&s);
    TokenArr_free(&s._arr);
    PreprocRes_free(&preproc_res);
}

typedef struct {
    ParserErr err;
    TokenArr toks;
} ParserErrAndToks;

static ParserErrAndToks parse_type_specs_until_fail(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    TypeSpecs specs = TypeSpecs_create();

    while (update_type_specs(&s, &specs))
        ;

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
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
    PreprocRes preproc_res = tokenize_string(spell, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    Statement* res = parse_statement(&s);
    ASSERT_NULL(res);

    ASSERT(err.kind == PARSER_ERR_EXPECTED_TOKENS);
    const ExpectedTokensErr* ex_tokens_err = &err.expected_tokens_err;
    ASSERT_UINT(ex_tokens_err->num_expected, (uint32_t)1);
    ASSERT_TOKEN_KIND(ex_tokens_err->expected[0], TOKEN_SEMICOLON);
    ASSERT_TOKEN_KIND(ex_tokens_err->got, TOKEN_INVALID);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
}

TEST(jump_statement_error) {
    check_expected_semicolon_jump_statement(STR_LIT("continue"));
    check_expected_semicolon_jump_statement(STR_LIT("break"));
    check_expected_semicolon_jump_statement(STR_LIT("return an_identifier"));
    check_expected_semicolon_jump_statement(STR_LIT("return *id += (int)100"));
}

TEST_SUITE_BEGIN(parser_error){
    REGISTER_TEST(redefine_typedef_error),
    REGISTER_TEST(type_spec_error),
    REGISTER_TEST(jump_statement_error),
} TEST_SUITE_END()
