#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/parser/ParserState.h"
#include "frontend/parser/parser.h"

#include "frontend/Token.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

typedef struct {
    UnaryExpr expr;
    TokenArr toks;
} UnaryExprAndToks;

static UnaryExprAndToks parse_unary_helper(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("skfjdlfs"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);
    UnaryExpr res;
    ASSERT(parse_unary_expr_inplace(&s, &res));
    ASSERT(err.kind == PARSER_ERR_NONE);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return (UnaryExprAndToks){res, s._arr};
}

TEST(unary_expr) {
    {
        UnaryExprAndToks res = parse_unary_helper(STR_LIT("++-- sizeof *name"));

        ASSERT(res.expr.kind == UNARY_DEREF);

        ASSERT_UINT(res.expr.len, (uint32_t)3);

        ASSERT(res.expr.ops_before[0] == UNARY_OP_INC);
        ASSERT(res.expr.ops_before[1] == UNARY_OP_DEC);
        ASSERT(res.expr.ops_before[2] == UNARY_OP_SIZEOF);

        check_cast_expr_id(res.expr.cast_expr, STR_LIT("name"), &res.toks);

        UnaryExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }
    {
        UnaryExprAndToks res = parse_unary_helper(STR_LIT("++++--++--100"));

        ASSERT_UINT(res.expr.len, (uint32_t)5);
        ASSERT(res.expr.kind == UNARY_POSTFIX);

        ASSERT(res.expr.ops_before[0] == UNARY_OP_INC);
        ASSERT(res.expr.ops_before[1] == UNARY_OP_INC);
        ASSERT(res.expr.ops_before[2] == UNARY_OP_DEC);
        ASSERT(res.expr.ops_before[3] == UNARY_OP_INC);
        ASSERT(res.expr.ops_before[4] == UNARY_OP_DEC);

        ASSERT(res.expr.postfix.is_primary);
        check_primary_expr_val(&res.expr.postfix.primary,
                               Value_create_sint(VALUE_INT, 100), &res.toks);

        UnaryExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }

    {
        UnaryExprAndToks res = parse_unary_helper(STR_LIT("sizeof(int)"));

        ASSERT(res.expr.kind == UNARY_SIZEOF_TYPE);
        ASSERT(res.expr.type_name->spec_qual_list->specs.kind == TYPE_SPEC_INT);

        UnaryExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }

    {
        UnaryExprAndToks res = parse_unary_helper(STR_LIT("~*var"));

        ASSERT(res.expr.kind == UNARY_BNOT);

        CastExpr* cast = res.expr.cast_expr;
        ASSERT_UINT(cast->len, (uint32_t)0);
        UnaryExpr* child_unary = &cast->rhs;

        ASSERT(child_unary->kind == UNARY_DEREF);
        check_cast_expr_id(child_unary->cast_expr, STR_LIT("var"), &res.toks);

        UnaryExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }
}

typedef struct {
    PostfixExpr expr;
    TokenArr toks;
} PostfixExprAndToks;

static PostfixExprAndToks parse_postfix_helper(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("sjfkds"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    UnaryExpr unary;
    ASSERT(parse_unary_expr_inplace(&s, &unary));
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_UINT(unary.len, 0);
    ASSERT(unary.kind == UNARY_POSTFIX);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);

    return (PostfixExprAndToks){unary.postfix, s._arr};
}

static void test_postfix_expr_intializer(bool tailing_comma) {
    char code[] = "(struct a_struct_name){1, test }";
    if (tailing_comma) {
        code[30] = ',';
    }
    PostfixExprAndToks res = parse_postfix_helper(STR_LIT(code));

    ASSERT(res.expr.is_primary == false);
    ASSERT_UINT(res.expr.init_list.len, (uint32_t)2);

    ASSERT(!Designation_is_valid(&res.expr.init_list.inits[0].designation));
    ASSERT(res.expr.init_list.inits[0].init.is_assign);
    check_assign_expr_val(res.expr.init_list.inits[0].init.assign,
                          Value_create_sint(VALUE_INT, 1), &res.toks);

    ASSERT(!Designation_is_valid(&res.expr.init_list.inits[1].designation));
    ASSERT(res.expr.init_list.inits[1].init.is_assign);
    check_assign_expr_id(res.expr.init_list.inits[1].init.assign, STR_LIT("test"), &res.toks);

    PostfixExpr_free_children(&res.expr);
    TokenArr_free(&res.toks);
}

TEST(postfix_expr) {
    {
        PostfixExprAndToks res = parse_postfix_helper(
            STR_LIT("test.ident->other++--++"));

        ASSERT(res.expr.is_primary);

        ASSERT_UINT(res.expr.len, (uint32_t)5);

        check_primary_expr_id(&res.expr.primary, STR_LIT("test"), &res.toks);

        ASSERT(res.expr.suffixes[0].kind == POSTFIX_ACCESS);
        ASSERT_STR(StrBuf_as_str(&res.toks.vals[res.expr.suffixes[0].identifier->info.token_idx].spelling),
                   STR_LIT("ident"));

        ASSERT(res.expr.suffixes[1].kind == POSTFIX_PTR_ACCESS);
        ASSERT_STR(StrBuf_as_str(&res.toks.vals[res.expr.suffixes[1].identifier->info.token_idx].spelling),
                   STR_LIT("other"));

        ASSERT(res.expr.suffixes[2].kind == POSTFIX_INC);

        ASSERT(res.expr.suffixes[3].kind == POSTFIX_DEC);

        ASSERT(res.expr.suffixes[4].kind == POSTFIX_INC);

        PostfixExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }

    {
        PostfixExprAndToks res = parse_postfix_helper(
            STR_LIT("test[i_am_id]()[23](another_id, 34, id)"));

        ASSERT_UINT(res.expr.len, (uint32_t)4);
        PostfixSuffix* suffix = res.expr.suffixes;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_id(&suffix->index_expr, STR_LIT("i_am_id"), &res.toks);

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_UINT(suffix->bracket_list.len, (uint32_t)0);

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_val(&suffix->index_expr, Value_create_sint(VALUE_INT, 23), &res.toks);

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_UINT(suffix->bracket_list.len, (uint32_t)3);
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[0],
                             STR_LIT("another_id"), &res.toks);
        check_assign_expr_val(&suffix->bracket_list.assign_exprs[1],
                              Value_create_sint(VALUE_INT, 34), &res.toks);
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[2],
                             STR_LIT("id"), &res.toks);

        PostfixExpr_free_children(&res.expr);
        TokenArr_free(&res.toks);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast_int(CondExpr* expr,
                                       TypeSpecKind cast_type,
                                       Value val,
                                       const TokenArr* arr) {
    CastExpr* cast = &expr->last_else.log_ands->or_exprs->xor_exprs->and_exprs
                          ->eq_exprs->lhs.lhs.lhs.lhs.lhs;
    ASSERT_UINT(cast->len, (uint32_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.kind == cast_type);
    check_unary_expr_val(&cast->rhs, val, arr);
}

typedef struct {
    AssignExpr* expr;
    TokenArr toks;
} AssignExprAndToks;

static AssignExprAndToks parse_assign_helper(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("blah"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    AssignExpr* res = parse_assign_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);

    return (AssignExprAndToks){res, s._arr};
}

TEST(assign_expr) {
    {
        AssignExprAndToks res = parse_assign_helper(STR_LIT("10"));
        check_assign_expr_val(res.expr, Value_create_sint(VALUE_INT, 10), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }

    {
        AssignExprAndToks res = parse_assign_helper(
            STR_LIT("x = 100 += y *= 100.0 /= 2"));
        ASSERT_NOT_NULL(res.expr);
        ASSERT_UINT(res.expr->len, (uint32_t)4);

        AssignExprOp expected_ops[] = {
            ASSIGN_EXPR_ASSIGN,
            ASSIGN_EXPR_ADD,
            ASSIGN_EXPR_MUL,
            ASSIGN_EXPR_DIV,
        };

        typedef struct {
            enum {
                VAL,
                STR
            } type;
            union {
                Value val;
                Str str;
            };
        } ValOrStr;
        ValOrStr expected_spellings[] = {
            {STR, .str = STR_LIT("x")},
            {VAL, .val = Value_create_sint(VALUE_INT, 100)},
            {STR, .str = STR_LIT("y")},
            {VAL, .val = Value_create_float(VALUE_DOUBLE, 100.0)},
        };

        enum {
            LEN = ARR_LEN(expected_ops)
        };

        for (uint32_t i = 0; i < LEN; ++i) {
            ASSERT(res.expr->assign_chain[i].op == expected_ops[i]);

            ValOrStr curr = expected_spellings[i];
            switch (curr.type) {
                case VAL:
                    check_unary_expr_val(&res.expr->assign_chain[i].unary, curr.val, &res.toks);
                    break;
                case STR:
                    check_unary_expr_id(&res.expr->assign_chain[i].unary, curr.str, &res.toks);
                    break;
            }
        }

        ASSERT(res.expr->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        check_unary_expr_id(&res.expr->assign_chain[0].unary, STR_LIT("x"), &res.toks);

        check_cond_expr_val(&res.expr->value, Value_create_sint(VALUE_INT, 2), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }

    {
        AssignExprAndToks res = parse_assign_helper(STR_LIT("(char)100"));

        ASSERT_UINT(res.expr->len, (uint32_t)0);
        check_assign_expr_cast_int(&res.expr->value,
                                   TYPE_SPEC_CHAR,
                                   Value_create_sint(VALUE_INT, 100), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }

    {
        AssignExprAndToks res = parse_assign_helper(
            STR_LIT("(struct a_struct){1, var} = 0.0"));

        ASSERT_UINT(res.expr->len, (uint32_t)1);

        ASSERT(res.expr->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        UnaryExpr* unary = &res.expr->assign_chain[0].unary;
        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_UINT(unary->len, (uint32_t)0);

        ASSERT(unary->postfix.is_primary == false);
        ASSERT_UINT(unary->postfix.len, (uint32_t)0);

        ASSERT_UINT(unary->postfix.init_list.len, (uint32_t)2);
        check_assign_expr_val(unary->postfix.init_list.inits[0].init.assign,
                              Value_create_sint(VALUE_INT, 1), &res.toks);
        check_assign_expr_id(unary->postfix.init_list.inits[1].init.assign,
                             STR_LIT("var"), &res.toks);

        check_cond_expr_val(&res.expr->value, Value_create_float(VALUE_DOUBLE, 0.0), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }

    {
        AssignExprAndToks res = parse_assign_helper(STR_LIT("var *= (double)12"));

        ASSERT_UINT(res.expr->len, (uint32_t)1);

        ASSERT(res.expr->assign_chain[0].op == ASSIGN_EXPR_MUL);
        check_unary_expr_id(&res.expr->assign_chain[0].unary, STR_LIT("var"), &res.toks);

        check_assign_expr_cast_int(&res.expr->value,
                                   TYPE_SPEC_DOUBLE,
                                   Value_create_sint(VALUE_INT, 12), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }

    {
        AssignExprAndToks res = parse_assign_helper(
            STR_LIT("var ^= (struct a_struct){1, var}"));

        ASSERT_UINT(res.expr->len, (uint32_t)1);

        ASSERT(res.expr->assign_chain[0].op == ASSIGN_EXPR_XOR);
        check_unary_expr_id(&res.expr->assign_chain[0].unary, STR_LIT("var"), &res.toks);

        UnaryExpr* unary = &res.expr->value.last_else.log_ands->or_exprs->xor_exprs
                                ->and_exprs->eq_exprs->lhs.lhs.lhs.lhs.lhs.rhs;

        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_UINT(unary->len, (uint32_t)0);
        ASSERT(unary->postfix.is_primary == false);
        ASSERT_UINT(unary->postfix.init_list.len, (uint32_t)2);

        ASSERT(!Designation_is_valid(
            &unary->postfix.init_list.inits[0].designation));
        ASSERT(unary->postfix.init_list.inits[0].init.is_assign);
        check_assign_expr_val(unary->postfix.init_list.inits[0].init.assign,
                              Value_create_sint(VALUE_INT, 1), &res.toks);
        ASSERT(!Designation_is_valid(
            &unary->postfix.init_list.inits[1].designation));
        ASSERT(unary->postfix.init_list.inits[1].init.is_assign);
        check_assign_expr_id(unary->postfix.init_list.inits[1].init.assign,
                             STR_LIT("var"), &res.toks);

        AssignExpr_free(res.expr);
        TokenArr_free(&res.toks);
    }
}

static void check_assign_expr_single_assign_id(AssignExpr* expr,
                                        Str lhs,
                                        AssignExprOp op,
                                        Str rhs,
                                        const TokenArr* arr) {
    ASSERT_UINT(expr->len, (uint32_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs, arr);
    check_cond_expr_id(&expr->value, rhs, arr);
}

static void check_assign_expr_single_assign_int(AssignExpr* expr,
                                         Str lhs,
                                         AssignExprOp op,
                                         Value rhs,
                                         const TokenArr* arr) {
    ASSERT_UINT(expr->len, (uint32_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs, arr);
    check_cond_expr_val(&expr->value, rhs, arr);
}

void check_assign_expr_single_assign_float(AssignExpr* expr,
                                           Str lhs,
                                           AssignExprOp op,
                                           Value rhs,
                                           const TokenArr* arr) {
    ASSERT_UINT(expr->len, (uint32_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs, arr);
    check_cond_expr_val(&expr->value, rhs, arr);
}

TEST(expr) {
    PreprocRes preproc_res = tokenize_string(
        STR_LIT("a = 10, b *= x, c += 3.1"),
        STR_LIT("file.c"));
    ASSERT(preproc_res.toks.len != 0);

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    Expr expr;
    ASSERT(parse_expr_inplace(&s, &expr));
    ASSERT(err.kind == PARSER_ERR_NONE);

    ASSERT_UINT(expr.len, (uint32_t)3);
    check_assign_expr_single_assign_int(&expr.assign_exprs[0],
                                        STR_LIT("a"),
                                        ASSIGN_EXPR_ASSIGN,
                                        Value_create_sint(VALUE_INT, 10),
                                        &s._arr);
    check_assign_expr_single_assign_id(&expr.assign_exprs[1],
                                       STR_LIT("b"),
                                       ASSIGN_EXPR_MUL,
                                       STR_LIT("x"),
                                       &s._arr);
    check_assign_expr_single_assign_float(&expr.assign_exprs[2],
                                          STR_LIT("c"),
                                          ASSIGN_EXPR_ADD,
                                          Value_create_float(VALUE_DOUBLE, 3.1),
                                          &s._arr);

    Expr_free_children(&expr);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
    PreprocRes_free(&preproc_res);
}

TEST_SUITE_BEGIN(parser_expr){
    REGISTER_TEST(unary_expr),
    REGISTER_TEST(postfix_expr),
    REGISTER_TEST(assign_expr),
    REGISTER_TEST(expr),
} TEST_SUITE_END()
