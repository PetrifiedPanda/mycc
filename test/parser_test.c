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
static void postfix_expr_test();
static void assign_expr_test();
static void static_assert_declaration_test();
static void statement_test();

void parser_test() {
    primary_expr_test();
    jump_statement_test();
    enum_list_test();
    enum_spec_test();
    designation_test();
    unary_expr_test();
    postfix_expr_test();
    assign_expr_test();
    static_assert_declaration_test();
    statement_test();
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

static void test_postfix_expr_id_or_const(struct postfix_expr* e, const char* spell, enum token_type type) {
    ASSERT(e->is_primary);
    test_primary_expr_id_or_const(e->primary, spell, type);
}

static void test_unary_expr_id_or_const(struct unary_expr* unary, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(unary->len, (size_t)0);
    ASSERT_NULL(unary->operators_before);

    ASSERT(unary->type == UNARY_POSTFIX);
    test_postfix_expr_id_or_const(unary->postfix, spell, type);
}

static void test_cast_expr_id_or_const(struct cast_expr* expr, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NULL(expr->type_names);

    test_unary_expr_id_or_const(expr->rhs, spell, type);
}

static void test_shift_expr_id_or_const(struct shift_expr* expr, const char* spell, enum token_type type) {
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NOT_NULL(expr->lhs);
    ASSERT_SIZE_T(expr->lhs->len, zero);
    ASSERT_NOT_NULL(expr->lhs->lhs);
    ASSERT_SIZE_T(expr->lhs->lhs->len, zero);
    ASSERT_NOT_NULL(expr->lhs->lhs->lhs);
    test_cast_expr_id_or_const(expr->lhs->lhs->lhs, spell, type);
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
    test_shift_expr_id_or_const(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs, spell, type);
}

static void test_const_expr_id_or_const(struct const_expr* expr, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(expr->expr.len, (size_t)0);
    ASSERT_NOT_NULL(expr->expr.last_else);
    test_cond_expr_id_or_const(&expr->expr, spell, type);
}

static void test_assign_expr_id_or_const(struct assign_expr* expr, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NULL(expr->assign_chain);
    ASSERT_NOT_NULL(expr->value);
    test_cond_expr_id_or_const(expr->value, spell, type);
}

static void test_expr_id_or_const(struct expr* expr, const char* spell, enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT_NOT_NULL(expr->assign_exprs);
    test_assign_expr_id_or_const(&expr->assign_exprs[0], spell, type);
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

    {
        struct token* tokens = tokenize("return 600;", "file.c");

        struct parser_state s = {.it = tokens};
        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_TOKEN_TYPE(res->type, RETURN);
        ASSERT_NOT_NULL(res->ret_val);

        test_expr_id_or_const(res->ret_val, "600", I_CONSTANT);

        free_jump_statement(res);
        free_tokenizer_result(tokens);
    }
}

static void test_enum_list_ids(struct enum_list* l, const char** enum_constants, size_t len) {
    ASSERT_SIZE_T(l->len, len);
    for (size_t i = 0; i < len; ++i) {
        ASSERT_NOT_NULL(l->enums[i].identifier);
        ASSERT_STR(l->enums[i].identifier->spelling, enum_constants[i]);
    }
}

static void enum_list_test() {
    enum {
        EXPECTED_LEN = 8
    };
    const char* enum_constants[EXPECTED_LEN] = {
            "ENUM_VAL1",
            "enum_VAl2",
            "enum_val_3",
            "enum_val_4",
            "foo",
            "bar",
            "baz",
            "BAD"
    };

    {
        struct token* tokens = tokenize("ENUM_VAL1, enum_VAl2, enum_val_3, enum_val_4, foo, bar, baz, BAD", "saffds");

        struct parser_state s = {.it = tokens};
        struct enum_list res = parse_enum_list(&s);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_SIZE_T(res.len, (size_t) EXPECTED_LEN);
        ASSERT_NOT_NULL(res.enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res.enums[i].identifier);
            ASSERT_NULL(res.enums[i].enum_val);
        }
        test_enum_list_ids(&res, enum_constants, sizeof enum_constants / sizeof(char*));

        free_enum_list(&res);
        free_tokenizer_result(tokens);
    }

    {
        struct token* tokens = tokenize("ENUM_VAL1 = 0, enum_VAl2 = 1000.0, enum_val_3 = n, enum_val_4 = test, foo, bar, baz, BAD", "saffds");

        struct parser_state s = {.it = tokens};
        struct enum_list res = parse_enum_list(&s);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_SIZE_T(res.len, (size_t) EXPECTED_LEN);
        ASSERT_NOT_NULL(res.enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res.enums[i].identifier);
        }

        ASSERT_NOT_NULL(res.enums[0].enum_val);
        test_const_expr_id_or_const(res.enums[0].enum_val, "0", I_CONSTANT);

        ASSERT_NOT_NULL(res.enums[1].enum_val);
        test_const_expr_id_or_const(res.enums[1].enum_val, "1000.0", F_CONSTANT);

        ASSERT_NOT_NULL(res.enums[2].enum_val);
        test_const_expr_id_or_const(res.enums[2].enum_val, "n", IDENTIFIER);

        ASSERT_NOT_NULL(res.enums[3].enum_val);
        test_const_expr_id_or_const(res.enums[3].enum_val, "test", IDENTIFIER);

        for (size_t i = 4; i < EXPECTED_LEN; ++i) {
            ASSERT_NULL(res.enums[i].enum_val);
        }

        free_enum_list(&res);
        free_tokenizer_result(tokens);
    }

    // TODO: test whether the identifiers were registered as enums
}

static void enum_spec_test() {
    const char* enum_constants[] = {
            "TEST1",
            "TEST2",
            "TEST3",
            "TEST4"
    };
    {
        struct token* tokens = tokenize("enum my_enum { TEST1, TEST2, TEST3, TEST4 }", "sfjlfjk");

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

static void designation_test() {
    {
        struct token* tokens = tokenize(".test[19].what_is_this.another_one = ", "jsalkf");

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
        struct token* tokens = tokenize("++-- sizeof *name", "skfjdlfs");

        struct parser_state s = {.it = tokens};
        struct unary_expr* res = parse_unary_expr(&s);
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

        free_unary_expr(res);
        free_tokenizer_result(tokens);
    }

    // TODO: test other cases
}

static void postfix_expr_test() {
    struct token* tokens = tokenize("test.ident->other++--++", "sjfkds");

    struct parser_state s = {.it = tokens};
    struct postfix_expr* res = parse_postfix_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_NO_ERROR();

    ASSERT(res->is_primary);

    ASSERT_SIZE_T(res->len, (size_t)5);

    test_primary_expr_id_or_const(res->primary, "test", IDENTIFIER);

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
    free_tokenizer_result(tokens);

    // TODO: add more cases
}

static void assign_expr_test() {
    {
        struct token* tokens = tokenize("10", "blah");

        struct parser_state s = {.it = tokens};
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        test_assign_expr_id_or_const(res, "10", I_CONSTANT);

        free_tokenizer_result(tokens);
        free_assign_expr(res);
    }

    {
        struct token* tokens = tokenize("x = 100 += y *= 100.0 /= 2", "not a file");

        struct parser_state s = {.it = tokens};
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

            test_unary_expr_id_or_const(res->assign_chain[i].unary, expected_spellings[i], expected_types[i]);
        }

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, ASSIGN);

        test_unary_expr_id_or_const(res->assign_chain[0].unary, "x", IDENTIFIER);

        test_cond_expr_id_or_const(res->value, "2", I_CONSTANT);

        free_tokenizer_result(tokens);
        free_assign_expr(res);
    }

    // TODO: add more testcases
}

static void static_assert_declaration_test() {
    struct token* tokens = tokenize("_Static_assert(12345, \"This is a string literal\");", "slkfjsak");

    struct parser_state s = {.it = tokens};
    struct static_assert_declaration* res = parse_static_assert_declaration(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_NO_ERROR();

    ASSERT_STR(res->err_msg.spelling, "\"This is a string literal\"");
    test_const_expr_id_or_const(res->const_expr, "12345", I_CONSTANT);

    free_tokenizer_result(tokens);
    free_static_assert_declaration(res);
}

static void statement_test() {
    const char* code = "for (i = 0; i < 100; ++i) {"
                       "    switch (c) {"
                       "        case 2:"
                       "            d -= 5;"
                       "            break;"
                       "        default:"
                       "            d += 5;"
                       "    }"
                       "    if (i >= 5) {"
                       "        ; "
                       "    } else b = 0;"
                       "}";
    struct token* tokens = tokenize(code, "file.c");

    struct parser_state s = {.it = tokens};
    struct statement* res = parse_statement(&s);
    ASSERT_NO_ERROR();
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_NOT_NULL(res);

    ASSERT(res->type == STATEMENT_ITERATION);
    struct iteration_statement* iteration = res->it;
    ASSERT(iteration->type == FOR);

    ASSERT(iteration->for_loop.is_decl == false);
    ASSERT_SIZE_T(iteration->for_loop.cond->expr.len, (size_t)1);

    ASSERT_SIZE_T(iteration->for_loop.cond->expr.assign_exprs[0].len, (size_t)0);

    struct rel_expr* rel = iteration->for_loop.cond->expr.assign_exprs[0].value->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs;
    ASSERT_SIZE_T(rel->len, (size_t)1);
    test_shift_expr_id_or_const(rel->lhs, "i", IDENTIFIER);
    ASSERT_TOKEN_TYPE(rel->rel_chain[0].rel_op, LT);
    test_shift_expr_id_or_const(rel->rel_chain[0].rhs, "100", I_CONSTANT);


    struct unary_expr* unary = iteration->for_loop.incr_expr->assign_exprs->value->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs->rhs;
    ASSERT_SIZE_T(unary->len, (size_t)1);
    ASSERT_TOKEN_TYPE(unary->operators_before[0], INC_OP);
    ASSERT(unary->type == UNARY_POSTFIX);
    test_postfix_expr_id_or_const(unary->postfix, "i", IDENTIFIER);

    ASSERT(iteration->loop_body->type == STATEMENT_COMPOUND);
    struct compound_statement* compound = iteration->loop_body->comp;
    ASSERT_SIZE_T(compound->len, (size_t)2);

    struct selection_statement* switch_stat = compound->items[0].stat.sel;
    ASSERT(switch_stat->is_if == false);
    test_expr_id_or_const(switch_stat->sel_expr, "c", IDENTIFIER);
    ASSERT(switch_stat->sel_stat->type == STATEMENT_COMPOUND);
    {
        struct compound_statement* switch_compound = switch_stat->sel_stat->comp;
        ASSERT_SIZE_T(switch_compound->len, (size_t)3);
        ASSERT(switch_compound->items[0].stat.type == STATEMENT_LABELED);
        struct labeled_statement* labeled = switch_compound->items[0].stat.labeled;
        ASSERT_TOKEN_TYPE(labeled->type, CASE);

        ASSERT_NOT_NULL(labeled->case_expr);
        test_const_expr_id_or_const(labeled->case_expr, "2", I_CONSTANT);

        ASSERT(labeled->stat->type == STATEMENT_EXPRESSION);
        struct expr* case_expr = &labeled->stat->expr->expr;
        ASSERT_SIZE_T(case_expr->assign_exprs->len, (size_t)1);

        test_cond_expr_id_or_const(case_expr->assign_exprs->value, "5", I_CONSTANT);
        ASSERT_TOKEN_TYPE(case_expr->assign_exprs->assign_chain[0].assign_op, SUB_ASSIGN);
        test_unary_expr_id_or_const(case_expr->assign_exprs->assign_chain[0].unary, "d", IDENTIFIER);

        ASSERT(switch_compound->items[1].stat.type == STATEMENT_JUMP);
        struct jump_statement* break_stat = switch_compound->items[1].stat.jmp;
        ASSERT_TOKEN_TYPE(break_stat->type, BREAK);
        ASSERT_NULL(break_stat->ret_val);

        ASSERT(switch_compound->items[2].stat.type == STATEMENT_LABELED);
        struct labeled_statement* default_stat = switch_compound->items[2].stat.labeled;

        ASSERT_TOKEN_TYPE(default_stat->type, DEFAULT);
        ASSERT_NULL(default_stat->case_expr);

        ASSERT(default_stat->stat->type == STATEMENT_EXPRESSION);
        struct expr* default_expr = &default_stat->stat->expr->expr;

        ASSERT_SIZE_T(default_expr->assign_exprs->len, (size_t)1);
        test_cond_expr_id_or_const(default_expr->assign_exprs->value, "5", I_CONSTANT);
        ASSERT_TOKEN_TYPE(default_expr->assign_exprs->assign_chain[0].assign_op, ADD_ASSIGN);
        test_unary_expr_id_or_const(default_expr->assign_exprs->assign_chain[0].unary, "d", IDENTIFIER);
    }

    ASSERT(compound->items[1].stat.type == STATEMENT_SELECTION);
    struct selection_statement* if_stat = compound->items[1].stat.sel;

    ASSERT(if_stat->is_if);

    struct rel_expr* if_cond = if_stat->sel_expr->assign_exprs->value->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->lhs;

    ASSERT_SIZE_T(if_cond->len, (size_t)1);
    test_shift_expr_id_or_const(if_cond->lhs, "i", IDENTIFIER);
    ASSERT_TOKEN_TYPE(if_cond->rel_chain[0].rel_op, GE_OP);
    test_shift_expr_id_or_const(if_cond->rel_chain[0].rhs, "5", I_CONSTANT);

    ASSERT(if_stat->sel_stat->type == STATEMENT_COMPOUND);
    ASSERT_SIZE_T(if_stat->sel_stat->comp->len, (size_t)1);
    struct expr* if_cont = &if_stat->sel_stat->comp->items->stat.expr->expr;
    ASSERT_NULL(if_cont->assign_exprs);
    ASSERT_SIZE_T(if_cont->len, (size_t)0);

    ASSERT_NOT_NULL(if_stat->else_stat);
    ASSERT(if_stat->else_stat->type == STATEMENT_EXPRESSION);
    struct expr* else_expr = &if_stat->else_stat->expr->expr;
    test_cond_expr_id_or_const(else_expr->assign_exprs->value, "0", I_CONSTANT);
    ASSERT_SIZE_T(else_expr->assign_exprs->len, (size_t)1);
    ASSERT_TOKEN_TYPE(else_expr->assign_exprs->assign_chain[0].assign_op, ASSIGN);
    test_unary_expr_id_or_const(else_expr->assign_exprs->assign_chain[0].unary, "b", IDENTIFIER);

    free_statement(res);
    free_tokenizer_result(tokens);

    // TODO: Add tests with declarations when implemented
}
