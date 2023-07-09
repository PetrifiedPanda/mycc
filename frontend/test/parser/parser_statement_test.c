#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static JumpStatement* parse_jump_statement_helper(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("skfjlskf"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    Statement stat;
    bool res = parse_statement_inplace(&s, &stat);
    ASSERT(res);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT(stat.kind == STATEMENT_JUMP);

    ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return stat.jmp;
}

static void check_jump_statement(Str spell, JumpStatementKind t) {
    JumpStatement* res = parse_jump_statement_helper(spell);

    ASSERT(res->kind == t);

    JumpStatement_free(res);
}

static void check_expected_semicolon_jump_statement(Str spell) {
    PreprocRes preproc_res = tokenize_string(spell, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    Statement* res = parse_statement(&s);
    ASSERT_NULL(res);

    ASSERT(err.kind == PARSER_ERR_EXPECTED_TOKENS);
    const ExpectedTokensErr* ex_tokens_err = &err.expected_tokens_err;
    ASSERT_SIZE_T(ex_tokens_err->num_expected, (size_t)1);
    ASSERT_TOKEN_KIND(ex_tokens_err->expected[0], TOKEN_SEMICOLON);
    ASSERT_TOKEN_KIND(ex_tokens_err->got, TOKEN_INVALID);

    PreprocRes_free(&preproc_res);
    ParserErr_free(&err);
    ParserState_free(&s);
}

TEST(jump_statement) {
    {
        JumpStatement* res = parse_jump_statement_helper(
            STR_LIT("goto my_cool_label;"));

        ASSERT(res->kind == JUMP_STATEMENT_GOTO);

        check_identifier(res->goto_label, STR_LIT("my_cool_label"));

        JumpStatement_free(res);
    }

    check_jump_statement(STR_LIT("continue;"), JUMP_STATEMENT_CONTINUE);
    check_jump_statement(STR_LIT("break;"), JUMP_STATEMENT_BREAK);
    check_jump_statement(STR_LIT("return;"), JUMP_STATEMENT_RETURN);

    check_expected_semicolon_jump_statement(STR_LIT("continue"));
    check_expected_semicolon_jump_statement(STR_LIT("break"));
    check_expected_semicolon_jump_statement(STR_LIT("return an_identifier"));
    check_expected_semicolon_jump_statement(STR_LIT("return *id += (int)100"));

    {
        JumpStatement* res = parse_jump_statement_helper(
            STR_LIT("return 600;"));

        ASSERT(res->kind == JUMP_STATEMENT_RETURN);
        ASSERT_SIZE_T(res->ret_val.len, 1);

        check_expr_val(&res->ret_val, Value_create_sint(VALUE_INT, 600));

        JumpStatement_free(res);
    }
}

TEST(statement) {
    Str code = STR_LIT("for (i = 0; i < 100; ++i) {"
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
                       "}");
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    Statement* res = parse_statement(&s);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);
    ASSERT_NOT_NULL(res);

    ASSERT(res->kind == STATEMENT_ITERATION);
    IterationStatement* iteration = res->it;
    ASSERT(iteration->kind == ITERATION_STATEMENT_FOR);

    ASSERT(iteration->for_loop.is_decl == false);
    ASSERT_SIZE_T(iteration->for_loop.cond->expr.len, (size_t)1);

    ASSERT_SIZE_T(iteration->for_loop.cond->expr.assign_exprs[0].len,
                  (size_t)0);

    RelExpr* rel = &iteration->for_loop.cond->expr.assign_exprs[0]
                        .value.last_else.log_ands->or_exprs->xor_exprs
                        ->and_exprs->eq_exprs->lhs;
    ASSERT_SIZE_T(rel->len, (size_t)1);
    check_shift_expr_id(&rel->lhs, STR_LIT("i"));
    ASSERT(rel->rel_chain[0].op == REL_EXPR_LT);
    check_shift_expr_val(&rel->rel_chain[0].rhs,
                         Value_create_sint(VALUE_INT, 100));

    UnaryExpr* unary = &iteration->for_loop.incr_expr.assign_exprs->value
                            .last_else.log_ands->or_exprs->xor_exprs->and_exprs
                            ->eq_exprs->lhs.lhs.lhs.lhs.lhs.rhs;
    ASSERT_SIZE_T(unary->len, (size_t)1);
    ASSERT(unary->ops_before[0] == UNARY_OP_INC);
    ASSERT(unary->kind == UNARY_POSTFIX);
    check_postfix_expr_id(&unary->postfix, STR_LIT("i"));

    ASSERT(iteration->loop_body->kind == STATEMENT_COMPOUND);
    CompoundStatement* compound = iteration->loop_body->comp;
    ASSERT_SIZE_T(compound->len, (size_t)2);

    SelectionStatement* switch_stat = compound->items[0].stat.sel;
    ASSERT(switch_stat->is_if == false);
    check_expr_id(&switch_stat->sel_expr, STR_LIT("c"));
    ASSERT(switch_stat->sel_stat->kind == STATEMENT_COMPOUND);
    {
        CompoundStatement* switch_compound = switch_stat->sel_stat->comp;
        ASSERT_SIZE_T(switch_compound->len, (size_t)3);
        ASSERT(switch_compound->items[0].stat.kind == STATEMENT_LABELED);
        LabeledStatement* labeled = switch_compound->items[0].stat.labeled;
        ASSERT(labeled->kind == LABELED_STATEMENT_CASE);

        check_const_expr_val(&labeled->case_expr,
                             Value_create_sint(VALUE_INT, 2));

        ASSERT(labeled->stat->kind == STATEMENT_EXPRESSION);
        Expr* case_expr = &labeled->stat->expr->expr;
        ASSERT_SIZE_T(case_expr->assign_exprs->len, (size_t)1);

        check_cond_expr_val(&case_expr->assign_exprs->value,
                            Value_create_sint(VALUE_INT, 5));
        ASSERT(case_expr->assign_exprs->assign_chain[0].op == ASSIGN_EXPR_SUB);
        check_unary_expr_id(&case_expr->assign_exprs->assign_chain[0].unary,
                            STR_LIT("d"));

        ASSERT(switch_compound->items[1].stat.kind == STATEMENT_JUMP);
        JumpStatement* break_stat = switch_compound->items[1].stat.jmp;
        ASSERT(break_stat->kind == JUMP_STATEMENT_BREAK);
        ASSERT_SIZE_T(break_stat->ret_val.len, 0);

        ASSERT(switch_compound->items[2].stat.kind == STATEMENT_LABELED);
        LabeledStatement* default_stat = switch_compound->items[2].stat.labeled;

        ASSERT(default_stat->kind == LABELED_STATEMENT_DEFAULT);

        ASSERT(default_stat->stat->kind == STATEMENT_EXPRESSION);
        Expr* default_expr = &default_stat->stat->expr->expr;

        ASSERT_SIZE_T(default_expr->assign_exprs->len, (size_t)1);
        check_cond_expr_val(&default_expr->assign_exprs->value,
                            Value_create_sint(VALUE_INT, 5));
        ASSERT(default_expr->assign_exprs->assign_chain[0].op
               == ASSIGN_EXPR_ADD);
        check_unary_expr_id(&default_expr->assign_exprs->assign_chain[0].unary,
                            STR_LIT("d"));
    }

    ASSERT(compound->items[1].stat.kind == STATEMENT_SELECTION);
    SelectionStatement* if_stat = compound->items[1].stat.sel;

    ASSERT(if_stat->is_if);

    RelExpr* if_cond = &if_stat->sel_expr.assign_exprs->value.last_else
                            .log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs
                            ->lhs;

    ASSERT_SIZE_T(if_cond->len, (size_t)1);
    check_shift_expr_id(&if_cond->lhs, STR_LIT("i"));
    ASSERT(if_cond->rel_chain[0].op == REL_EXPR_GE);
    check_shift_expr_val(&if_cond->rel_chain[0].rhs,
                         Value_create_sint(VALUE_INT, 5));

    ASSERT(if_stat->sel_stat->kind == STATEMENT_COMPOUND);
    ASSERT_SIZE_T(if_stat->sel_stat->comp->len, (size_t)1);
    Expr* if_cont = &if_stat->sel_stat->comp->items->stat.expr->expr;
    ASSERT_NULL(if_cont->assign_exprs);
    ASSERT_SIZE_T(if_cont->len, (size_t)0);

    ASSERT_NOT_NULL(if_stat->else_stat);
    ASSERT(if_stat->else_stat->kind == STATEMENT_EXPRESSION);
    Expr* else_expr = &if_stat->else_stat->expr->expr;
    check_cond_expr_val(&else_expr->assign_exprs->value,
                        Value_create_sint(VALUE_INT, 0));
    ASSERT_SIZE_T(else_expr->assign_exprs->len, (size_t)1);
    ASSERT(else_expr->assign_exprs->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);
    check_unary_expr_id(&else_expr->assign_exprs->assign_chain[0].unary, STR_LIT("b"));

    Statement_free(res);
    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    // TODO: Add tests with declarations when implemented
}

TEST_SUITE_BEGIN(parser_statement){
    REGISTER_TEST(jump_statement),
    REGISTER_TEST(statement),
} TEST_SUITE_END()
