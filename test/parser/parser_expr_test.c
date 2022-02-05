#include <stdio.h>

#include "token.h"
#include "tokenizer.h"
#include "util.h"

#include "parser/parser.h"

#include "../test_asserts.h"

#include "parser_test_util.h"

static void primary_expr_test();
static void unary_expr_test();
static void postfix_expr_test();
static void assign_expr_test();

void parser_expr_test() {
    primary_expr_test();
    unary_expr_test();
    postfix_expr_test();
    assign_expr_test();
}

static void test_primary_expr_constant(enum token_type type, const char* spell) {
    struct token* tokens = tokenize(spell, "not a file so I can write whatever here");

    struct parser_state s = create_parser_state(tokens);

    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_CONSTANT);
    ASSERT_TOKEN_TYPE(res->constant.type, type);
    ASSERT_STR(res->constant.spelling, spell);

    ASSERT_NULL(tokens[0].spelling);

    free_primary_expr(res);
    free_parser_state(&s);
    free_tokenizer_result(tokens);
}

static void test_primary_expr_string(const char* spell) {
    struct token* tokens = tokenize(spell, "no_file.c");

    struct parser_state s = create_parser_state(tokens);

    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res->string.is_func == false);
    ASSERT_STR(res->string.lit.spelling, spell);

    free_primary_expr(res);
    free_parser_state(&s);
    free_tokenizer_result(tokens);
}

static void test_primary_expr_func_name() {
    struct token *tokens = tokenize("__func__", "not a file so go away");
    struct parser_state s = create_parser_state(tokens);
    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->string.is_func == true);
    ASSERT_NULL(res->string.lit.spelling);

    free_tokenizer_result(tokens);
    free_parser_state(&s);
    free_primary_expr(res);
}

static void test_primary_expr_identifier(const char* spell) {
    struct token* tokens = tokenize(spell, "a string");

    struct parser_state s = create_parser_state(tokens);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_ERROR(get_last_error(), ERR_NONE);

    ASSERT(res->type == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(res->identifier->spelling, spell);

    ASSERT_NULL(tokens[0].spelling);

    free_primary_expr(res);
    free_parser_state(&s);
    free_tokenizer_result(tokens);
}

static void primary_expr_test() {
    test_primary_expr_constant(F_CONSTANT, "3.1e-5f");
    test_primary_expr_constant(I_CONSTANT, "0xdeadbeefl");
    test_primary_expr_identifier("super_cool_identifier");
    test_primary_expr_identifier("another_cool_identifier");
    test_primary_expr_string("\"Test string it does not matter whether this is an actual string literal but hey\"");
    test_primary_expr_string("\"Multi\\\nline\\\nstring\\\nbaybee\"");
    test_primary_expr_func_name();

    // TODO: '(' expr ')' and generic selection
}

static void unary_expr_test() {
    {
        struct token* tokens = tokenize("++-- sizeof *name", "skfjdlfs");

        struct parser_state s = create_parser_state(tokens);
        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();

        ASSERT(res->type == UNARY_UNARY_OP);

        ASSERT_SIZE_T(res->len, (size_t) 3);

        ASSERT_TOKEN_TYPE(res->operators_before[0], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[1], DEC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[2], SIZEOF);

        ASSERT_TOKEN_TYPE(res->unary_op, ASTERISK);
        check_cast_expr_id_or_const(res->cast_expr, "name", IDENTIFIER);

        free_unary_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }
    {
        struct token* tokens = tokenize("++++--++--100", "ksjflkdsjf");

        struct parser_state s = create_parser_state(tokens);
        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)5);
        ASSERT(res->type == UNARY_POSTFIX);

        ASSERT_TOKEN_TYPE(res->operators_before[0], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[1], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[2], DEC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[3], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[4], DEC_OP);

        ASSERT(res->postfix->is_primary);
        check_primary_expr_id_or_const(res->postfix->primary, "100", I_CONSTANT);

        free_unary_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }

    // TODO: test other cases
}

static void postfix_expr_test() {
    struct token* tokens = tokenize("test.ident->other++--++", "sjfkds");

    struct parser_state s = create_parser_state(tokens);
    struct postfix_expr* res = parse_postfix_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_NO_ERROR();

    ASSERT(res->is_primary);

    ASSERT_SIZE_T(res->len, (size_t)5);

    check_primary_expr_id_or_const(res->primary, "test", IDENTIFIER);

    ASSERT(res->suffixes[0].type == POSTFIX_ACCESS);
    ASSERT_STR(res->suffixes[0].identifier->spelling, "ident");

    ASSERT(res->suffixes[1].type == POSTFIX_PTR_ACCESS);
    ASSERT_STR(res->suffixes[1].identifier->spelling, "other");

    ASSERT(res->suffixes[2].type == POSTFIX_INC_DEC);
    ASSERT_TOKEN_TYPE(res->suffixes[2].inc_dec, INC_OP);

    ASSERT(res->suffixes[3].type == POSTFIX_INC_DEC);
    ASSERT_TOKEN_TYPE(res->suffixes[3].inc_dec, DEC_OP);

    ASSERT(res->suffixes[4].type == POSTFIX_INC_DEC);
    ASSERT_TOKEN_TYPE(res->suffixes[4].inc_dec, INC_OP);

    free_postfix_expr(res);
    free_parser_state(&s);
    free_tokenizer_result(tokens);

    // TODO: add more cases
}

static void assign_expr_test() {
    {
        struct token* tokens = tokenize("10", "blah");

        struct parser_state s = create_parser_state(tokens);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        check_assign_expr_id_or_const(res, "10", I_CONSTANT);

        free_tokenizer_result(tokens);
        free_parser_state(&s);
        free_assign_expr(res);
    }

    {
        struct token* tokens = tokenize("x = 100 += y *= 100.0 /= 2", "not a file");

        struct parser_state s = create_parser_state(tokens);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_SIZE_T(res->len, (size_t) 4);

        enum token_type expected_ops[] = {
            ASSIGN,
            ADD_ASSIGN,
            MUL_ASSIGN,
            DIV_ASSIGN
        };

        enum token_type expected_types[] = {
            IDENTIFIER,
            I_CONSTANT,
            IDENTIFIER,
            F_CONSTANT
        };

        const char *expected_spellings[] = {
            "x",
            "100",
            "y",
            "100.0"
        };

        enum {
            SIZE = sizeof expected_ops / sizeof(enum token_type)
        };

        for (size_t i = 0; i < SIZE; ++i) {
            ASSERT_TOKEN_TYPE(res->assign_chain[i].assign_op, expected_ops[i]);

            check_unary_expr_id_or_const(res->assign_chain[i].unary, expected_spellings[i], expected_types[i]);
        }

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, ASSIGN);

        check_unary_expr_id_or_const(res->assign_chain[0].unary, "x", IDENTIFIER);

        check_cond_expr_id_or_const(res->value, "2", I_CONSTANT);

        free_tokenizer_result(tokens);
        free_parser_state(&s);
        free_assign_expr(res);
    }

    // TODO: add more testcases
}
