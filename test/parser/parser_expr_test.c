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

static void test_postfix_expr_intializer(bool tailing_comma) {
    char* code = alloc_string_copy("(struct a_struct_name){1, test }");
    if (tailing_comma) {
        code[30] = ',';
    }
    struct token* tokens = tokenize(code, "not_a_file.c");

    struct parser_state s = create_parser_state(tokens);
    struct postfix_expr* res = parse_postfix_expr(&s);
    ASSERT_NO_ERROR();
    ASSERT_NOT_NULL(res);

    ASSERT(res->is_primary == false);
    ASSERT_SIZE_T(res->init_list.len, (size_t)2);

    ASSERT_NULL(res->init_list.inits[0].designation);
    ASSERT(res->init_list.inits[0].init->is_assign);
    check_assign_expr_id_or_const(res->init_list.inits[0].init->assign, "1", I_CONSTANT);

    ASSERT_NULL(res->init_list.inits[1].designation);
    ASSERT(res->init_list.inits[1].init->is_assign);
    check_assign_expr_id_or_const(res->init_list.inits[1].init->assign, "test", IDENTIFIER);

    free(code);
    free_postfix_expr(res);
    free_parser_state(&s);
    free_tokenizer_result(tokens);
}

static void postfix_expr_test() {
    {
        struct token* tokens = tokenize("test.ident->other++--++", "sjfkds");

        struct parser_state s = create_parser_state(tokens);
        struct postfix_expr* res = parse_postfix_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();

        ASSERT(res->is_primary);

        ASSERT_SIZE_T(res->len, (size_t) 5);

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
    }

    {
        struct token* tokens = tokenize("test[i_am_id]()[23](another_id, 34, id)", "not_a_file.c");

        struct parser_state s = create_parser_state(tokens);
        struct postfix_expr* res = parse_postfix_expr(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)4);
        struct postfix_suffix* suffix = res->suffixes;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_id_or_const(suffix->index_expr, "i_am_id", IDENTIFIER);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)0);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_id_or_const(suffix->index_expr, "23", I_CONSTANT);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)3);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[0], "another_id", IDENTIFIER);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[1], "34", I_CONSTANT);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[2], "id", IDENTIFIER);

        free_postfix_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast(struct cond_expr* expr, enum token_type cast_type, const char* spell, enum token_type value_type) {
    struct cast_expr* cast = expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs;
    ASSERT_SIZE_T(cast->len, (size_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.type == TYPESPEC_PREDEF);
    ASSERT_TOKEN_TYPE(cast->type_names[0].spec_qual_list->specs.type_spec, cast_type);
    check_unary_expr_id_or_const(cast->rhs, spell, value_type);
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

        const char* expected_spellings[] = {
            "x",
            "100",
            "y",
            "100.0"
        };

        enum {SIZE = sizeof expected_ops / sizeof(enum token_type)};

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


    {
        struct token* tokens = tokenize("(char)100", "not_file.c");

        struct parser_state s = create_parser_state(tokens);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)0);
        check_assign_expr_cast(res->value, CHAR, "100", I_CONSTANT);

        free_assign_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }

    {
        struct token* tokens = tokenize("(struct a_struct){1, var} = 0", "not_a_file.c");

        struct parser_state s = create_parser_state(tokens);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, ASSIGN);

        struct unary_expr* unary = res->assign_chain[0].unary;
        ASSERT(unary->type == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);

        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->len, (size_t)0);

        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);
        check_assign_expr_id_or_const(unary->postfix->init_list.inits[0].init->assign, "1", I_CONSTANT);
        check_assign_expr_id_or_const(unary->postfix->init_list.inits[1].init->assign, "var", IDENTIFIER);

        free_assign_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }

    {
        struct token* tokens = tokenize("var *= (double)12", "not_a_file.c");

        struct parser_state s = create_parser_state(tokens);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, MUL_ASSIGN);
        check_unary_expr_id_or_const(res->assign_chain[0].unary, "var", IDENTIFIER);

        check_assign_expr_cast(res->value, DOUBLE, "12", I_CONSTANT);

        free_assign_expr(res);
        free_parser_state(&s);
        free_tokenizer_result(tokens);
    }

    // TODO: add more testcases
}
