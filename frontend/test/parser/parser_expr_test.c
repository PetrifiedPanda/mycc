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

}

TEST_SUITE_BEGIN(parser_expr){
    REGISTER_TEST(unary_expr),
    REGISTER_TEST(postfix_expr),
} TEST_SUITE_END()
