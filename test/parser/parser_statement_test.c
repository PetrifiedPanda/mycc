#include "util.h"

#include "preproc/preproc.h"

#include "parser/parser.h"

#include "../test_asserts.h"

#include "parser_test_util.h"

static void jump_statement_test();
static void statement_test();

void parser_statement_test() {
    jump_statement_test();
    statement_test();
}

static void check_jump_statement(const char* spell, enum token_type t) {
    struct token* tokens = preproc_string(spell, "skfjlskf");

    struct parser_state s = create_parser_state(tokens);
    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT_NOT_NULL(res);

    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_TOKEN_TYPE(res->type, t);

    free_jump_statement(res);
    free_parser_state(&s);
    free_tokens(tokens);
}

static void check_expected_semicolon_jump_statement(const char* spell) {
    struct token* tokens = preproc_string(spell, "file.c");

    struct parser_state s = create_parser_state(tokens);

    struct jump_statement* res = parse_jump_statement(&s);
    ASSERT_NULL(res);

    ASSERT_ERROR(get_last_error(), ERR_PARSER);
    ASSERT_STR(get_error_string(),
               "Expected token of type SEMICOLON but got to end of file");

    clear_last_error();
    free_tokens(tokens);
    free_parser_state(&s);
}

static void jump_statement_test() {
    {
        struct token* tokens = preproc_string("goto my_cool_label;", "file");

        struct parser_state s = create_parser_state(tokens);
        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NOT_NULL(res);

        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_TOKEN_TYPE(res->type, GOTO);

        check_identifier(res->identifier, "my_cool_label");

        free_jump_statement(res);
        free_parser_state(&s);
        free_tokens(tokens);
    }

    check_jump_statement("continue;", CONTINUE);
    check_jump_statement("break;", BREAK);
    check_jump_statement("return;", RETURN);

    check_expected_semicolon_jump_statement("continue");
    check_expected_semicolon_jump_statement("break");
    check_expected_semicolon_jump_statement("return an_identifier");
    check_expected_semicolon_jump_statement("return *id += (int)100");

    {
        struct token* tokens = preproc_string("not_what_was_expected;",
                                              "a_file.c");

        struct parser_state s = create_parser_state(tokens);

        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NULL(res);
        ASSERT_ERROR(get_last_error(), ERR_PARSER);

        ASSERT_STR(get_error_string(),
                   "a_file.c(1,1):\nExpected token of type GOTO, CONTINUE, "
                   "BREAK, RETURN but got token of type IDENTIFIER");

        clear_last_error();
        free_tokens(tokens);
        free_parser_state(&s);
    }

    {
        struct token* tokens = preproc_string("return 600;", "file.c");

        struct parser_state s = create_parser_state(tokens);
        struct jump_statement* res = parse_jump_statement(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NO_ERROR();
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_TOKEN_TYPE(res->type, RETURN);
        ASSERT_NOT_NULL(res->ret_val);

        check_expr_id_or_const(res->ret_val, "600", I_CONSTANT);

        free_jump_statement(res);
        free_parser_state(&s);
        free_tokens(tokens);
    }
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
    struct token* tokens = preproc_string(code, "file.c");

    struct parser_state s = create_parser_state(tokens);
    struct statement* res = parse_statement(&s);
    ASSERT_NO_ERROR();
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT_NOT_NULL(res);

    ASSERT(res->type == STATEMENT_ITERATION);
    struct iteration_statement* iteration = res->it;
    ASSERT(iteration->type == FOR);

    ASSERT(iteration->for_loop.is_decl == false);
    ASSERT_SIZE_T(iteration->for_loop.cond->expr.len, (size_t)1);

    ASSERT_SIZE_T(iteration->for_loop.cond->expr.assign_exprs[0].len,
                  (size_t)0);

    struct rel_expr* rel = iteration->for_loop.cond->expr.assign_exprs[0]
                               .value->last_else->log_ands->or_exprs->xor_exprs
                               ->and_exprs->eq_exprs->lhs;
    ASSERT_SIZE_T(rel->len, (size_t)1);
    check_shift_expr_id_or_const(rel->lhs, "i", IDENTIFIER);
    ASSERT_TOKEN_TYPE(rel->rel_chain[0].rel_op, LT);
    check_shift_expr_id_or_const(rel->rel_chain[0].rhs, "100", I_CONSTANT);

    struct unary_expr* unary = iteration->for_loop.incr_expr->assign_exprs
                                   ->value->last_else->log_ands->or_exprs
                                   ->xor_exprs->and_exprs->eq_exprs->lhs->lhs
                                   ->lhs->lhs->lhs->rhs;
    ASSERT_SIZE_T(unary->len, (size_t)1);
    ASSERT_TOKEN_TYPE(unary->operators_before[0], INC_OP);
    ASSERT(unary->type == UNARY_POSTFIX);
    check_postfix_expr_id_or_const(unary->postfix, "i", IDENTIFIER);

    ASSERT(iteration->loop_body->type == STATEMENT_COMPOUND);
    struct compound_statement* compound = iteration->loop_body->comp;
    ASSERT_SIZE_T(compound->len, (size_t)2);

    struct selection_statement* switch_stat = compound->items[0].stat.sel;
    ASSERT(switch_stat->is_if == false);
    check_expr_id_or_const(switch_stat->sel_expr, "c", IDENTIFIER);
    ASSERT(switch_stat->sel_stat->type == STATEMENT_COMPOUND);
    {
        struct compound_statement* switch_compound = switch_stat->sel_stat
                                                         ->comp;
        ASSERT_SIZE_T(switch_compound->len, (size_t)3);
        ASSERT(switch_compound->items[0].stat.type == STATEMENT_LABELED);
        struct labeled_statement* labeled = switch_compound->items[0]
                                                .stat.labeled;
        ASSERT_TOKEN_TYPE(labeled->type, CASE);

        ASSERT_NOT_NULL(labeled->case_expr);
        check_const_expr_id_or_const(labeled->case_expr, "2", I_CONSTANT);

        ASSERT(labeled->stat->type == STATEMENT_EXPRESSION);
        struct expr* case_expr = &labeled->stat->expr->expr;
        ASSERT_SIZE_T(case_expr->assign_exprs->len, (size_t)1);

        check_cond_expr_id_or_const(case_expr->assign_exprs->value,
                                    "5",
                                    I_CONSTANT);
        ASSERT_TOKEN_TYPE(case_expr->assign_exprs->assign_chain[0].assign_op,
                          SUB_ASSIGN);
        check_unary_expr_id_or_const(
            case_expr->assign_exprs->assign_chain[0].unary,
            "d",
            IDENTIFIER);

        ASSERT(switch_compound->items[1].stat.type == STATEMENT_JUMP);
        struct jump_statement* break_stat = switch_compound->items[1].stat.jmp;
        ASSERT_TOKEN_TYPE(break_stat->type, BREAK);
        ASSERT_NULL(break_stat->ret_val);

        ASSERT(switch_compound->items[2].stat.type == STATEMENT_LABELED);
        struct labeled_statement* default_stat = switch_compound->items[2]
                                                     .stat.labeled;

        ASSERT_TOKEN_TYPE(default_stat->type, DEFAULT);
        ASSERT_NULL(default_stat->case_expr);

        ASSERT(default_stat->stat->type == STATEMENT_EXPRESSION);
        struct expr* default_expr = &default_stat->stat->expr->expr;

        ASSERT_SIZE_T(default_expr->assign_exprs->len, (size_t)1);
        check_cond_expr_id_or_const(default_expr->assign_exprs->value,
                                    "5",
                                    I_CONSTANT);
        ASSERT_TOKEN_TYPE(default_expr->assign_exprs->assign_chain[0].assign_op,
                          ADD_ASSIGN);
        check_unary_expr_id_or_const(
            default_expr->assign_exprs->assign_chain[0].unary,
            "d",
            IDENTIFIER);
    }

    ASSERT(compound->items[1].stat.type == STATEMENT_SELECTION);
    struct selection_statement* if_stat = compound->items[1].stat.sel;

    ASSERT(if_stat->is_if);

    struct rel_expr* if_cond = if_stat->sel_expr->assign_exprs->value->last_else
                                   ->log_ands->or_exprs->xor_exprs->and_exprs
                                   ->eq_exprs->lhs;

    ASSERT_SIZE_T(if_cond->len, (size_t)1);
    check_shift_expr_id_or_const(if_cond->lhs, "i", IDENTIFIER);
    ASSERT_TOKEN_TYPE(if_cond->rel_chain[0].rel_op, GE_OP);
    check_shift_expr_id_or_const(if_cond->rel_chain[0].rhs, "5", I_CONSTANT);

    ASSERT(if_stat->sel_stat->type == STATEMENT_COMPOUND);
    ASSERT_SIZE_T(if_stat->sel_stat->comp->len, (size_t)1);
    struct expr* if_cont = &if_stat->sel_stat->comp->items->stat.expr->expr;
    ASSERT_NULL(if_cont->assign_exprs);
    ASSERT_SIZE_T(if_cont->len, (size_t)0);

    ASSERT_NOT_NULL(if_stat->else_stat);
    ASSERT(if_stat->else_stat->type == STATEMENT_EXPRESSION);
    struct expr* else_expr = &if_stat->else_stat->expr->expr;
    check_cond_expr_id_or_const(else_expr->assign_exprs->value,
                                "0",
                                I_CONSTANT);
    ASSERT_SIZE_T(else_expr->assign_exprs->len, (size_t)1);
    ASSERT_TOKEN_TYPE(else_expr->assign_exprs->assign_chain[0].assign_op,
                      ASSIGN);
    check_unary_expr_id_or_const(else_expr->assign_exprs->assign_chain[0].unary,
                                 "b",
                                 IDENTIFIER);

    free_statement(res);
    free_parser_state(&s);
    free_tokens(tokens);

    // TODO: Add tests with declarations when implemented
}
