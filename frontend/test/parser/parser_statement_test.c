#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static struct jump_statement* parse_jump_statement_helper(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "skfjlskf");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_NOT_NULL(res);

    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    free_parser_state(&s);
    free_preproc_res(&preproc_res);

    return res;
}

static void check_jump_statement(const char* spell,
                                 enum jump_statement_type t) {
    struct jump_statement* res = parse_jump_statement_helper(spell);

    ASSERT(res->type == t);

    free_jump_statement(res);
}

static void check_expected_semicolon_jump_statement(const char* spell) {
    struct preproc_res preproc_res = tokenize_string(spell, "file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT_NULL(res);

    ASSERT(err.type == PARSER_ERR_EXPECTED_TOKENS);
    ASSERT_SIZE_T(err.num_expected, (size_t)1);
    ASSERT_TOKEN_TYPE(err.expected[0], SEMICOLON);
    ASSERT_TOKEN_TYPE(err.got, INVALID);

    free_preproc_res(&preproc_res);
    free_parser_err(&err);
    free_parser_state(&s);
}

TEST(jump_statement) {
    {
        struct jump_statement* res = parse_jump_statement_helper(
            "goto my_cool_label;");

        ASSERT(res->type == JUMP_STATEMENT_GOTO);

        check_identifier(res->goto_label, "my_cool_label");

        free_jump_statement(res);
    }

    check_jump_statement("continue;", JUMP_STATEMENT_CONTINUE);
    check_jump_statement("break;", JUMP_STATEMENT_BREAK);
    check_jump_statement("return;", JUMP_STATEMENT_RETURN);

    check_expected_semicolon_jump_statement("continue");
    check_expected_semicolon_jump_statement("break");
    check_expected_semicolon_jump_statement("return an_identifier");
    check_expected_semicolon_jump_statement("return *id += (int)100");

    {
        struct preproc_res preproc_res = tokenize_string(
            "not_what_was_expected;",
            "a_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NULL(res);

        ASSERT_SIZE_T(err.base.loc.file_idx, (size_t)0);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, (size_t)1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, (size_t)1);

        ASSERT_SIZE_T(err.num_expected, (size_t)4);
        ASSERT_TOKEN_TYPE(err.got, IDENTIFIER);
        ASSERT_TOKEN_TYPE(err.expected[0], GOTO);
        ASSERT_TOKEN_TYPE(err.expected[1], CONTINUE);
        ASSERT_TOKEN_TYPE(err.expected[2], BREAK);
        ASSERT_TOKEN_TYPE(err.expected[3], RETURN);

        free_preproc_res(&preproc_res);
        free_parser_err(&err);
        free_parser_state(&s);
    }

    {
        struct jump_statement* res = parse_jump_statement_helper("return 600;");

        ASSERT(res->type == JUMP_STATEMENT_RETURN);
        ASSERT_NOT_NULL(res->ret_val);

        check_expr_int(res->ret_val, create_int_value(INT_VALUE_I, 600));

        free_jump_statement(res);
    }
}

TEST(statement) {
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
    struct preproc_res preproc_res = tokenize_string(code, "file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct statement* res = parse_statement(&s);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_NOT_NULL(res);

    ASSERT(res->type == STATEMENT_ITERATION);
    struct iteration_statement* iteration = res->it;
    ASSERT(iteration->type == ITERATION_STATEMENT_FOR);

    ASSERT(iteration->for_loop.is_decl == false);
    ASSERT_SIZE_T(iteration->for_loop.cond->expr.len, (size_t)1);

    ASSERT_SIZE_T(iteration->for_loop.cond->expr.assign_exprs[0].len,
                  (size_t)0);

    struct rel_expr* rel = iteration->for_loop.cond->expr.assign_exprs[0]
                               .value->last_else->log_ands->or_exprs->xor_exprs
                               ->and_exprs->eq_exprs->lhs;
    ASSERT_SIZE_T(rel->len, (size_t)1);
    check_shift_expr_id(rel->lhs, "i");
    ASSERT(rel->rel_chain[0].op == REL_EXPR_LT);
    check_shift_expr_int(rel->rel_chain[0].rhs,
                         create_int_value(INT_VALUE_I, 100));

    struct unary_expr* unary = iteration->for_loop.incr_expr->assign_exprs
                                   ->value->last_else->log_ands->or_exprs
                                   ->xor_exprs->and_exprs->eq_exprs->lhs->lhs
                                   ->lhs->lhs->lhs->rhs;
    ASSERT_SIZE_T(unary->len, (size_t)1);
    ASSERT(unary->ops_before[0] == UNARY_OP_INC);
    ASSERT(unary->type == UNARY_POSTFIX);
    check_postfix_expr_id(unary->postfix, "i");

    ASSERT(iteration->loop_body->type == STATEMENT_COMPOUND);
    struct compound_statement* compound = iteration->loop_body->comp;
    ASSERT_SIZE_T(compound->len, (size_t)2);

    struct selection_statement* switch_stat = compound->items[0].stat.sel;
    ASSERT(switch_stat->is_if == false);
    check_expr_id(switch_stat->sel_expr, "c");
    ASSERT(switch_stat->sel_stat->type == STATEMENT_COMPOUND);
    {
        struct compound_statement* switch_compound = switch_stat->sel_stat
                                                         ->comp;
        ASSERT_SIZE_T(switch_compound->len, (size_t)3);
        ASSERT(switch_compound->items[0].stat.type == STATEMENT_LABELED);
        struct labeled_statement* labeled = switch_compound->items[0]
                                                .stat.labeled;
        ASSERT(labeled->type == LABELED_STATEMENT_CASE);

        ASSERT_NOT_NULL(labeled->case_expr);
        check_const_expr_int(labeled->case_expr,
                             create_int_value(INT_VALUE_I, 2));

        ASSERT(labeled->stat->type == STATEMENT_EXPRESSION);
        struct expr* case_expr = &labeled->stat->expr->expr;
        ASSERT_SIZE_T(case_expr->assign_exprs->len, (size_t)1);

        check_cond_expr_int(case_expr->assign_exprs->value,
                            create_int_value(INT_VALUE_I, 5));
        ASSERT(case_expr->assign_exprs->assign_chain[0].op == ASSIGN_EXPR_SUB);
        check_unary_expr_id(case_expr->assign_exprs->assign_chain[0].unary,
                            "d");

        ASSERT(switch_compound->items[1].stat.type == STATEMENT_JUMP);
        struct jump_statement* break_stat = switch_compound->items[1].stat.jmp;
        ASSERT(break_stat->type == JUMP_STATEMENT_BREAK);
        ASSERT_NULL(break_stat->ret_val);

        ASSERT(switch_compound->items[2].stat.type == STATEMENT_LABELED);
        struct labeled_statement* default_stat = switch_compound->items[2]
                                                     .stat.labeled;

        ASSERT(default_stat->type == LABELED_STATEMENT_DEFAULT);
        ASSERT_NULL(default_stat->case_expr);

        ASSERT(default_stat->stat->type == STATEMENT_EXPRESSION);
        struct expr* default_expr = &default_stat->stat->expr->expr;

        ASSERT_SIZE_T(default_expr->assign_exprs->len, (size_t)1);
        check_cond_expr_int(default_expr->assign_exprs->value,
                            create_int_value(INT_VALUE_I, 5));
        ASSERT(default_expr->assign_exprs->assign_chain[0].op
               == ASSIGN_EXPR_ADD);
        check_unary_expr_id(default_expr->assign_exprs->assign_chain[0].unary,
                            "d");
    }

    ASSERT(compound->items[1].stat.type == STATEMENT_SELECTION);
    struct selection_statement* if_stat = compound->items[1].stat.sel;

    ASSERT(if_stat->is_if);

    struct rel_expr* if_cond = if_stat->sel_expr->assign_exprs->value->last_else
                                   ->log_ands->or_exprs->xor_exprs->and_exprs
                                   ->eq_exprs->lhs;

    ASSERT_SIZE_T(if_cond->len, (size_t)1);
    check_shift_expr_id(if_cond->lhs, "i");
    ASSERT(if_cond->rel_chain[0].op == REL_EXPR_GE);
    check_shift_expr_int(if_cond->rel_chain[0].rhs,
                         create_int_value(INT_VALUE_I, 5));

    ASSERT(if_stat->sel_stat->type == STATEMENT_COMPOUND);
    ASSERT_SIZE_T(if_stat->sel_stat->comp->len, (size_t)1);
    struct expr* if_cont = &if_stat->sel_stat->comp->items->stat.expr->expr;
    ASSERT_NULL(if_cont->assign_exprs);
    ASSERT_SIZE_T(if_cont->len, (size_t)0);

    ASSERT_NOT_NULL(if_stat->else_stat);
    ASSERT(if_stat->else_stat->type == STATEMENT_EXPRESSION);
    struct expr* else_expr = &if_stat->else_stat->expr->expr;
    check_cond_expr_int(else_expr->assign_exprs->value,
                        create_int_value(INT_VALUE_I, 0));
    ASSERT_SIZE_T(else_expr->assign_exprs->len, (size_t)1);
    ASSERT(else_expr->assign_exprs->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);
    check_unary_expr_id(else_expr->assign_exprs->assign_chain[0].unary, "b");

    free_statement(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);

    // TODO: Add tests with declarations when implemented
}

TEST_SUITE_BEGIN(parser_statement, 2) {
    REGISTER_TEST(jump_statement);
    REGISTER_TEST(statement);
}
TEST_SUITE_END()
