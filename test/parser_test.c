#include <stdio.h>

#include "token.h"
#include "tokenizer.h"
#include "util.h"

#include "ast/translation_unit.h"

#include "test_asserts.h"

static void primary_expr_test();
static void jump_statement_test();
static void enum_list_test();
static void enum_spec_test();
static void designator_list_test();

void parser_test() {
    primary_expr_test();
    jump_statement_test();
    enum_list_test();
    enum_spec_test();
    designator_list_test();
    printf("Parser test successful\n");
}

static void test_primary_expr_identifier(const char* spell) {
    struct token* tokens = tokenize(spell, "a string");

    struct parser_state s = {.it = tokens};

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_ERROR(get_last_error(), ERR_NONE);

    ASSERT(res->type == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(res->identifier->spelling, spell);

    ASSERT_NULL(tokens[0].spelling);

    free_primary_expr(res);
    free_tokenizer_result(tokens);
}

static void test_primary_expr_constant(enum token_type type, const char* spell) {
    struct token* tokens = tokenize(spell, "not a file so I can write whatever here");

    struct parser_state s = {.it = tokens};

    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_CONSTANT);
    ASSERT_TOKEN_TYPE(res->constant.type, type);
    ASSERT_STR(res->constant.spelling, spell);

    ASSERT_NULL(tokens[0].spelling);

    free_primary_expr(res);
    free_tokenizer_result(tokens);
}

static void test_primary_expr_string(const char* spell) {
    struct token* tokens = tokenize(spell, "no_file.c");

    struct parser_state s = {.it = tokens};

    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res->string.is_func == false);
    ASSERT_STR(res->string.lit.spelling, spell);

    free_primary_expr(res);
    free_tokenizer_result(tokens);
}

static void test_primary_expr_func_name() {
    struct token *tokens = tokenize("__func__", "not a file so go away");
    struct parser_state s = {.it = tokens};
    struct primary_expr *res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->string.is_func == true);
    ASSERT_NULL(res->string.lit.spelling);

    free_tokenizer_result(tokens);
    free_primary_expr(res);
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

static void test_jump_statement(const char* spell, enum token_type t) {
    struct token* tokens = tokenize(spell, "skfjlskf");

    struct parser_state s = {.it = tokens};
    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT_NOT_NULL(res);

    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_TOKEN_TYPE(res->type, t);

    free_jump_statement(res);
    free_tokenizer_result(tokens);
}

static void test_expected_semicolon_jump_statement(const char* spell) {
    struct token* tokens = tokenize(spell, "file.c");

    struct parser_state s = {.it = tokens};

    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT_NULL(res);

    ASSERT_ERROR(get_last_error(), ERR_PARSER);
    ASSERT_STR(get_error_string(), "Expected token of type SEMICOLON but got to end of file");

    clear_last_error();
    free_tokenizer_result(tokens);
}

static void test_identifier(struct identifier* i, const char* spell) {
    ASSERT_NOT_NULL(i);
    ASSERT_STR(i->spelling, spell);
}

static void jump_statement_test() {
    {
        struct token* tokens = tokenize("goto my_cool_label;", "file");

        struct parser_state s = {.it = tokens};
        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NOT_NULL(res);

        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_TOKEN_TYPE(res->type, GOTO);

        test_identifier(res->identifier, "my_cool_label");

        free_jump_statement(res);
        free_tokenizer_result(tokens);
    }

    test_jump_statement("continue;", CONTINUE);
    test_jump_statement("break;", BREAK);
    test_jump_statement("return;", RETURN);

    test_expected_semicolon_jump_statement("continue");
    test_expected_semicolon_jump_statement("break");

    {
        struct token* tokens = tokenize("not_what_was_expected;", "a_file.c");

        struct parser_state s = {.it = tokens};

        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NULL(res);
        ASSERT_ERROR(get_last_error(), ERR_PARSER);

        ASSERT_STR(get_error_string(), "a_file.c(1,1):\nExpected token of type GOTO, CONTINUE, BREAK, RETURN but got token of type IDENTIFIER");

        clear_last_error();
        free_tokenizer_result(tokens);
    }

    // TODO: test with return value
}

static void test_enum_list_ids(struct enum_list* l, const char** enum_constants, size_t len) {
    ASSERT_SIZE_T(l->len, len);
    for (size_t i = 0; i < len; ++i) {
        ASSERT_NOT_NULL(l->enums[i].identifier);
        ASSERT_STR(l->enums[i].identifier->spelling, enum_constants[i]);
    }
}

static void enum_list_test() {
    struct token* tokens = tokenize("ENUM_VAL1, enum_VAl2, enum_val_3, enum_val_4, foo, bar, baz, BAD", "saffds");

    enum {EXPECTED_LEN = 8};
    struct parser_state s = {.it = tokens};
    struct enum_list res = parse_enum_list(&s);
    ASSERT_NO_ERROR();
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_SIZE_T(res.len, (size_t)EXPECTED_LEN);
    ASSERT_NOT_NULL(res.enums);

    for (size_t i = 0; i < EXPECTED_LEN; ++i) {
        ASSERT_NOT_NULL(res.enums[i].identifier);
        ASSERT_NULL(res.enums[i].enum_val);
    }
    const char* enum_constants[] = {
            "ENUM_VAL1",
            "enum_VAl2",
            "enum_val_3",
            "enum_val_4",
            "foo",
            "bar",
            "baz",
            "BAD"
    };
    test_enum_list_ids(&res, enum_constants, sizeof enum_constants / sizeof(char*));

    free_enum_list(&res);
    free_tokenizer_result(tokens);

    // TODO: test with initializers
}

static void enum_spec_test() {
    const char *enum_constants[] = {
            "TEST1",
            "TEST2",
            "TEST3",
            "TEST4"
    };
    {
        struct token *tokens = tokenize("enum my_enum { TEST1, TEST2, TEST3, TEST4 }", "sfjlfjk");

        struct parser_state s = {.it = tokens};
        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_NOT_NULL(res->identifier);
        ASSERT_STR(res->identifier->spelling, "my_enum");

        test_enum_list_ids(&res->enum_list, enum_constants, sizeof enum_constants / sizeof(char *));

        free_tokenizer_result(tokens);
        free_enum_spec(res);
    }

    {
        struct token* tokens = tokenize("enum {TEST1, TEST2, TEST3, TEST4, }", "jsfjsf");

        struct parser_state s = {.it = tokens};
        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_NULL(res->identifier);

        test_enum_list_ids(&res->enum_list, enum_constants, sizeof enum_constants / sizeof(char *));

        free_tokenizer_result(tokens);
        free_enum_spec(res);
    }
}

static void test_cond_expr_constant(struct cond_expr* expr, const char* spell, enum token_type type) {
    const size_t one = (size_t)1;
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(expr->len, zero);
    ASSERT_SIZE_T(expr->last_else->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->len, zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->len, zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->len, zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->len, zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->len, zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->len, zero);
    ASSERT(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->type == UNARY_POSTFIX);
    ASSERT(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->postfix->is_primary == true);
    ASSERT(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->postfix->primary->type == PRIMARY_EXPR_CONSTANT);
    ASSERT_TOKEN_TYPE(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->postfix->primary->constant.type, type);
    ASSERT_NOT_NULL(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->postfix->primary->constant.spelling);
    ASSERT_STR(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs->postfix->primary->constant.spelling, spell);
}

static void designator_list_test() {
    {
        struct token *tokens = tokenize(".test[19].what_is_this.another_one", "jsalkf");

        struct parser_state s = {.it = tokens};
        struct designator_list res = parse_designator_list(&s);
        ASSERT_NOT_NULL(res.designators);
        ASSERT_NO_ERROR();

        ASSERT_SIZE_T(res.len, (size_t)4);

        ASSERT(res.designators[0].is_index == false);
        test_identifier(res.designators[0].identifier, "test");

        ASSERT(res.designators[1].is_index == true);
        test_cond_expr_constant(&res.designators[1].arr_index->expr, "19", I_CONSTANT);

        ASSERT(res.designators[2].is_index == false);
        test_identifier(res.designators[2].identifier, "what_is_this");

        ASSERT(res.designators[3].is_index == false);
        test_identifier(res.designators[3].identifier, "another_one");

        free_designator_list(&res);
        free_tokenizer_result(tokens);
    }

    {
        struct token* tokens = tokenize("[0.5].blah[420].oof[2][10]", "stetsd");

        struct parser_state s = {.it = tokens};
        struct designator_list res = parse_designator_list(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res.designators);

        ASSERT_SIZE_T(res.len, (size_t)6);

        ASSERT(res.designators[0].is_index == true);
        test_cond_expr_constant(&res.designators[0].arr_index->expr, "0.5", F_CONSTANT);

        ASSERT(res.designators[1].is_index == false);
        test_identifier(res.designators[1].identifier, "blah");

        ASSERT(res.designators[2].is_index == true);
        test_cond_expr_constant(&res.designators[2].arr_index->expr, "420", I_CONSTANT);

        ASSERT(res.designators[3].is_index == false);
        test_identifier(res.designators[3].identifier, "oof");

        ASSERT(res.designators[4].is_index == true);
        test_cond_expr_constant(&res.designators[4].arr_index->expr, "2", I_CONSTANT);

        ASSERT(res.designators[5].is_index == true);
        test_cond_expr_constant(&res.designators[5].arr_index->expr, "10", I_CONSTANT);
    }
}
