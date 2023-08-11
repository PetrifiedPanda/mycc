#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

typedef struct {
    JumpStatement* stat;
    TokenArr toks;
} JumpStatementAndToks;

static JumpStatementAndToks parse_jump_statement_helper(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("skfjlskf"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    Statement stat;
    bool res = parse_statement_inplace(&s, &stat);
    ASSERT(res);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT(stat.kind == STATEMENT_JUMP);

    ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return (JumpStatementAndToks){stat.jmp, s._arr};
}

static void check_jump_statement(Str spell, JumpStatementKind t) {
    JumpStatementAndToks res = parse_jump_statement_helper(spell);

    ASSERT(res.stat->kind == t);

    JumpStatement_free(res.stat);
    TokenArr_free(&res.toks);
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

TEST(jump_statement) {
    {
        JumpStatementAndToks res = parse_jump_statement_helper(
            STR_LIT("goto my_cool_label;"));

        ASSERT(res.stat->kind == JUMP_STATEMENT_GOTO);

        check_identifier(res.stat->goto_label, STR_LIT("my_cool_label"), &res.toks);

        JumpStatement_free(res.stat);
        TokenArr_free(&res.toks);
    }

    check_jump_statement(STR_LIT("continue;"), JUMP_STATEMENT_CONTINUE);
    check_jump_statement(STR_LIT("break;"), JUMP_STATEMENT_BREAK);
    check_jump_statement(STR_LIT("return;"), JUMP_STATEMENT_RETURN);

    check_expected_semicolon_jump_statement(STR_LIT("continue"));
    check_expected_semicolon_jump_statement(STR_LIT("break"));
    check_expected_semicolon_jump_statement(STR_LIT("return an_identifier"));
    check_expected_semicolon_jump_statement(STR_LIT("return *id += (int)100"));

    {
        JumpStatementAndToks res = parse_jump_statement_helper(
            STR_LIT("return 600;"));

        ASSERT(res.stat->kind == JUMP_STATEMENT_RETURN);
        ASSERT_UINT(res.stat->ret_val.len, 1);

        check_expr_val(&res.stat->ret_val, Value_create_sint(VALUE_INT, 600), &res.toks);

        JumpStatement_free(res.stat);
        TokenArr_free(&res.toks);
    }
}

TEST_SUITE_BEGIN(parser_statement){
    REGISTER_TEST(jump_statement),
} TEST_SUITE_END()
