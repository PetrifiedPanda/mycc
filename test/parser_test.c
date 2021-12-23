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
static void designation_test();
static void unary_expr_test();

void parser_test() {
    primary_expr_test();
    jump_statement_test();
    enum_list_test();
    enum_spec_test();
    designation_test();
    unary_expr_test();
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

static void test_primary_expr_id_or_const(struct primary_expr* e, const char* spell, enum token_type type) {
    enum primary_expr_type expected_type = type == IDENTIFIER ? PRIMARY_EXPR_IDENTIFIER : PRIMARY_EXPR_CONSTANT;
    ASSERT(e->type == expected_type);
    if (type == IDENTIFIER) {
        ASSERT_NOT_NULL(e->identifier);
        ASSERT_STR(e->identifier->spelling, spell);
    } else {
        ASSERT_TOKEN_TYPE(e->constant.type, type);
        ASSERT_STR(e->constant.spelling, spell);
    }
}

static void test_cast_expr_id_or_const(struct cast_expr* expr, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT(expr->rhs->type == UNARY_POSTFIX);
    ASSERT_SIZE_T(expr->rhs->len, (size_t)0);
    ASSERT(expr->rhs->postfix->is_primary == true);
    test_primary_expr_id_or_const(expr->rhs->postfix->primary, spell, type);
}

static void test_cond_expr_id_or_const(struct cond_expr* expr, const char* spell, enum token_type type) {
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
    test_cast_expr_id_or_const(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs, spell, type);
}

static void designation_test() {
    {
        struct token *tokens = tokenize(".test[19].what_is_this.another_one = ", "jsalkf");

        struct parser_state s = {.it = tokens};
        struct designation* res = parse_designation(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NOT_NULL(res->designators.designators);
        ASSERT_NO_ERROR();

        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_SIZE_T(res->designators.len, (size_t)4);

        struct designator* designators = res->designators.designators;
        ASSERT(designators[0].is_index == false);
        test_identifier(designators[0].identifier, "test");

        ASSERT(designators[1].is_index == true);
        test_cond_expr_id_or_const(&designators[1].arr_index->expr, "19", I_CONSTANT);

        ASSERT(designators[2].is_index == false);
        test_identifier(designators[2].identifier, "what_is_this");

        ASSERT(designators[3].is_index == false);
        test_identifier(designators[3].identifier, "another_one");

        free_designation(res);
        free_tokenizer_result(tokens);
    }

    {
        struct token* tokens = tokenize("[0.5].blah[420].oof[2][10] =", "stetsd");

        struct parser_state s = {.it = tokens};
        struct designation* res = parse_designation(&s);
        ASSERT_NO_ERROR();
        ASSERT_NOT_NULL(res);
        ASSERT_NOT_NULL(res->designators.designators);

        ASSERT_SIZE_T(res->designators.len, (size_t)6);

        struct designator* designators = res->designators.designators;
        ASSERT(designators[0].is_index == true);
        test_cond_expr_id_or_const(&designators[0].arr_index->expr, "0.5", F_CONSTANT);

        ASSERT(designators[1].is_index == false);
        test_identifier(designators[1].identifier, "blah");

        ASSERT(designators[2].is_index == true);
        test_cond_expr_id_or_const(&designators[2].arr_index->expr, "420", I_CONSTANT);

        ASSERT(designators[3].is_index == false);
        test_identifier(designators[3].identifier, "oof");

        ASSERT(designators[4].is_index == true);
        test_cond_expr_id_or_const(&designators[4].arr_index->expr, "2", I_CONSTANT);

        ASSERT(designators[5].is_index == true);
        test_cond_expr_id_or_const(&designators[5].arr_index->expr, "10", I_CONSTANT);

        free_designation(res);
        free_tokenizer_result(tokens);
    }
}

static void unary_expr_test() {
    {
        struct token *tokens = tokenize("++-- sizeof *name", "skfjdlfs");

        struct parser_state s = {.it = tokens};
        struct unary_expr *res = parse_unary_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();

        ASSERT(res->type == UNARY_UNARY_OP);

        ASSERT_SIZE_T(res->len, (size_t) 3);

        ASSERT_TOKEN_TYPE(res->operators_before[0], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[1], DEC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[2], SIZEOF);

        ASSERT_TOKEN_TYPE(res->unary_op, ASTERISK);
        test_cast_expr_id_or_const(res->cast_expr, "name", IDENTIFIER);

        free_unary_expr(res);
        free_tokenizer_result(tokens);
    }
    {
        struct token* tokens = tokenize("++++--++--100", "ksjflkdsjf");

        struct parser_state s = {.it = tokens};
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
        test_primary_expr_id_or_const(res->postfix->primary, "100", I_CONSTANT);
    }

    // TODO: test other cases
}
