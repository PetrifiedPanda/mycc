#include <stdio.h>

#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/parser/ParserState.h"
#include "frontend/parser/parser.h"

#include "frontend/Token.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static PrimaryExpr* parse_primary_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "not_file.c");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    PrimaryExpr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(s.it->kind, TOKEN_INVALID);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
    return res;
}

static void check_primary_expr_int_constant(ConstantKind type,
                                            const char* spell,
                                            IntValue expected) {
    PrimaryExpr* res = parse_primary_helper(spell);

    ASSERT(res->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(res->constant.kind == type);
    assert(res->constant.kind != CONSTANT_ENUM);

    check_int_value(res->constant.int_val, expected);

    PrimaryExpr_free(res);
}

static void check_primary_expr_float_constant(ConstantKind type,
                                              const char* spell,
                                              FloatValue expected) {
    PrimaryExpr* res = parse_primary_helper(spell);

    ASSERT(res->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(res->constant.kind == type);
    assert(res->constant.kind != CONSTANT_ENUM);

    check_float_value(res->constant.float_val, expected);

    PrimaryExpr_free(res);
}

static void check_primary_expr_string(const char* spell, const char* expected) {
    PrimaryExpr* res = parse_primary_helper(spell);

    ASSERT(res->kind == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res->string.is_func == false);
    ASSERT_STR(Str_get_data(&res->string.lit.lit.contents), expected);

    PrimaryExpr_free(res);
}

static void check_primary_expr_func_name(void) {
    PrimaryExpr* res = parse_primary_helper("__func__");

    ASSERT(res->string.is_func == true);

    PrimaryExpr_free(res);
}

static void check_primary_expr_identifier(const char* spell) {
    PrimaryExpr* res = parse_primary_helper(spell);

    ASSERT(res->kind == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(Str_get_data(&res->identifier->spelling), spell);

    PrimaryExpr_free(res);
}

static void check_primary_expr_bracket_id(const char* code,
                                          const char* bracket_spell) {
    PrimaryExpr* res = parse_primary_helper(code);
    ASSERT(res->kind == PRIMARY_EXPR_BRACKET);
    check_expr_id(res->bracket_expr, bracket_spell);

    PrimaryExpr_free(res);
}

static void check_primary_expr_bracket_float(const char* code,
                                             FloatValue val) {
    PrimaryExpr* res = parse_primary_helper(code);
    ASSERT(res->kind == PRIMARY_EXPR_BRACKET);
    check_expr_float(res->bracket_expr, val);

    PrimaryExpr_free(res);
}

static void primary_expr_generic_sel_test(void) {
    PreprocRes preproc_res = tokenize_string(
        "_Generic(var, int: 0, TypedefName: oops, struct a_struct: 5.0, "
        "default: default_value )",
        "not_file.c");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);
    const Token insert_token = {
        .kind = TOKEN_IDENTIFIER,
        .spelling = STR_NON_HEAP("TypedefName"),
        .loc =
            {
                .file_idx = 0,
                .file_loc = {0, 0},
            },
    };

    parser_register_typedef_name(&s, &insert_token);
    PrimaryExpr* res = parse_primary_expr(&s);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_KIND(s.it->kind, TOKEN_INVALID);

    ASSERT(res->kind == PRIMARY_EXPR_GENERIC);
    check_assign_expr_id(res->generic->assign, "var");

    ASSERT_SIZE_T(res->generic->assocs.len, (size_t)4);
    GenericAssoc* assoc = res->generic->assocs.assocs;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.kind == TYPE_SPEC_INT);
    check_assign_expr_int(assoc->assign, IntValue_create_signed(INT_VALUE_I, 0));

    ++assoc;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.kind == TYPE_SPEC_TYPENAME);
    check_identifier(assoc->type_name->spec_qual_list->specs.typedef_name,
                     "TypedefName");
    check_assign_expr_id(assoc->assign, "oops");

    ++assoc;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.kind == TYPE_SPEC_STRUCT);
    ASSERT(
        assoc->type_name->spec_qual_list->specs.struct_union_spec->is_struct);
    ASSERT_SIZE_T(assoc->type_name->spec_qual_list->specs.struct_union_spec
                      ->decl_list.len,
                  (size_t)0);
    check_identifier(
        assoc->type_name->spec_qual_list->specs.struct_union_spec->identifier,
        "a_struct");
    check_assign_expr_float(assoc->assign,
                            FloatValue_create(FLOAT_VALUE_D, 5.0));

    ++assoc;

    ASSERT_NULL(assoc->type_name);
    check_assign_expr_id(assoc->assign, "default_value");

    PrimaryExpr_free(res);
    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
}

TEST(primary_expr) {
    check_primary_expr_float_constant(
        CONSTANT_FLOAT,
        "3.1e-5f",
        FloatValue_create(FLOAT_VALUE_F, 3.1e-5f));
    check_primary_expr_int_constant(CONSTANT_INT,
                                    "0xdeadbeefl",
                                    IntValue_create_signed(INT_VALUE_L, 0xdeadbeefl));
    check_primary_expr_identifier("super_cool_identifier");
    check_primary_expr_identifier("another_cool_identifier");
    check_primary_expr_string("\"Test string it does not matter whether this "
                              "is an actual string literal but hey\"",
                              "Test string it does not matter whether this "
                              "is an actual string literal but hey");
    check_primary_expr_func_name();
    check_primary_expr_bracket_float("(23.3)",
                                     FloatValue_create(FLOAT_VALUE_D, 23.3));
    check_primary_expr_bracket_id("(var)", "var");
    primary_expr_generic_sel_test();
}

static UnaryExpr* parse_unary_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "skfjdlfs");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);
    UnaryExpr* res = parse_unary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_NONE);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return res;
}

TEST(unary_expr) {
    {
        UnaryExpr* res = parse_unary_helper("++-- sizeof *name");

        ASSERT(res->kind == UNARY_DEREF);

        ASSERT_SIZE_T(res->len, (size_t)3);

        ASSERT(res->ops_before[0] == UNARY_OP_INC);
        ASSERT(res->ops_before[1] == UNARY_OP_DEC);
        ASSERT(res->ops_before[2] == UNARY_OP_SIZEOF);

        check_cast_expr_id(res->cast_expr, "name");

        UnaryExpr_free(res);
    }
    {
        UnaryExpr* res = parse_unary_helper("++++--++--100");

        ASSERT_SIZE_T(res->len, (size_t)5);
        ASSERT(res->kind == UNARY_POSTFIX);

        ASSERT(res->ops_before[0] == UNARY_OP_INC);
        ASSERT(res->ops_before[1] == UNARY_OP_INC);
        ASSERT(res->ops_before[2] == UNARY_OP_DEC);
        ASSERT(res->ops_before[3] == UNARY_OP_INC);
        ASSERT(res->ops_before[4] == UNARY_OP_DEC);

        ASSERT(res->postfix->is_primary);
        check_primary_expr_int(res->postfix->primary,
                               IntValue_create_signed(INT_VALUE_I, 100));

        UnaryExpr_free(res);
    }

    {
        UnaryExpr* res = parse_unary_helper("sizeof(int)");

        ASSERT(res->kind == UNARY_SIZEOF_TYPE);
        ASSERT(res->type_name->spec_qual_list->specs.kind == TYPE_SPEC_INT);

        UnaryExpr_free(res);
    }

    {
        UnaryExpr* res = parse_unary_helper("~*var");

        ASSERT(res->kind == UNARY_BNOT);

        CastExpr* cast = res->cast_expr;
        ASSERT_SIZE_T(cast->len, (size_t)0);
        UnaryExpr* child_unary = cast->rhs;

        ASSERT(child_unary->kind == UNARY_DEREF);
        check_cast_expr_id(child_unary->cast_expr, "var");

        UnaryExpr_free(res);
    }
}

static PostfixExpr* parse_postfix_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "sjfkds");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    PostfixExpr* res = parse_postfix_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_NONE);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return res;
}

static void test_postfix_expr_intializer(bool tailing_comma) {
    char code[] = "(struct a_struct_name){1, test }";
    if (tailing_comma) {
        code[30] = ',';
    }
    PostfixExpr* res = parse_postfix_helper(code);

    ASSERT(res->is_primary == false);
    ASSERT_SIZE_T(res->init_list.len, (size_t)2);

    ASSERT(!Designation_is_valid(&res->init_list.inits[0].designation));
    ASSERT(res->init_list.inits[0].init.is_assign);
    check_assign_expr_int(res->init_list.inits[0].init.assign,
                          IntValue_create_signed(INT_VALUE_I, 1));

    ASSERT(!Designation_is_valid(&res->init_list.inits[1].designation));
    ASSERT(res->init_list.inits[1].init.is_assign);
    check_assign_expr_id(res->init_list.inits[1].init.assign, "test");

    PostfixExpr_free(res);
}

TEST(postfix_expr) {
    {
        PostfixExpr* res = parse_postfix_helper(
            "test.ident->other++--++");

        ASSERT(res->is_primary);

        ASSERT_SIZE_T(res->len, (size_t)5);

        check_primary_expr_id(res->primary, "test");

        ASSERT(res->suffixes[0].kind == POSTFIX_ACCESS);
        ASSERT_STR(Str_get_data(&res->suffixes[0].identifier->spelling),
                   "ident");

        ASSERT(res->suffixes[1].kind == POSTFIX_PTR_ACCESS);
        ASSERT_STR(Str_get_data(&res->suffixes[1].identifier->spelling),
                   "other");

        ASSERT(res->suffixes[2].kind == POSTFIX_INC);

        ASSERT(res->suffixes[3].kind == POSTFIX_DEC);

        ASSERT(res->suffixes[4].kind == POSTFIX_INC);

        PostfixExpr_free(res);
    }

    {
        PostfixExpr* res = parse_postfix_helper(
            "test[i_am_id]()[23](another_id, 34, id)");

        ASSERT_SIZE_T(res->len, (size_t)4);
        PostfixSuffix* suffix = res->suffixes;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_id(suffix->index_expr, "i_am_id");

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)0);

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_int(suffix->index_expr, IntValue_create_signed(INT_VALUE_I, 23));

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)3);
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[0],
                             "another_id");
        check_assign_expr_int(&suffix->bracket_list.assign_exprs[1],
                              IntValue_create_signed(INT_VALUE_I, 34));
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[2], "id");

        PostfixExpr_free(res);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast_int(CondExpr* expr,
                                       TypeSpecKind cast_type,
                                       IntValue val) {
    CastExpr* cast = expr->last_else->log_ands->or_exprs->xor_exprs
                                 ->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs;
    ASSERT_SIZE_T(cast->len, (size_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.kind == cast_type);
    check_unary_expr_int(cast->rhs, val);
}

static AssignExpr* parse_assign_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "blah");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    AssignExpr* res = parse_assign_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(s.it->kind, TOKEN_INVALID);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);

    return res;
}

TEST(assign_expr) {
    {
        AssignExpr* res = parse_assign_helper("10");

        check_assign_expr_int(res, IntValue_create_signed(INT_VALUE_I, 10));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper(
            "x = 100 += y *= 100.0 /= 2");
        ASSERT_NOT_NULL(res);
        ASSERT_SIZE_T(res->len, (size_t)4);

        AssignExprOp expected_ops[] = {
            ASSIGN_EXPR_ASSIGN,
            ASSIGN_EXPR_ADD,
            ASSIGN_EXPR_MUL,
            ASSIGN_EXPR_DIV,
        };

        typedef struct {
            enum {
                INT_VAL,
                FLOAT_VAL,
                STR
            } type;
            union {
                IntValue int_val;
                FloatValue float_val;
                const char* str;
            };
        } ValOrStr;
        ValOrStr expected_spellings[] = {
            {STR, .str = "x"},
            {INT_VAL, .int_val = IntValue_create_signed(INT_VALUE_I, 100)},
            {STR, .str = "y"},
            {FLOAT_VAL, .float_val = FloatValue_create(FLOAT_VALUE_D, 100.0)},
        };

        enum {
            LEN = ARR_LEN(expected_ops)
        };

        for (size_t i = 0; i < LEN; ++i) {
            ASSERT(res->assign_chain[i].op == expected_ops[i]);

            ValOrStr curr = expected_spellings[i];
            switch (curr.type) {
                case INT_VAL:
                    check_unary_expr_int(res->assign_chain[i].unary,
                                         curr.int_val);
                    break;
                case FLOAT_VAL:
                    check_unary_expr_float(res->assign_chain[i].unary,
                                           curr.float_val);
                    break;
                case STR:
                    check_unary_expr_id(res->assign_chain[i].unary, curr.str);
                    break;
            }
        }

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        check_unary_expr_id(res->assign_chain[0].unary, "x");

        check_cond_expr_int(res->value, IntValue_create_signed(INT_VALUE_I, 2));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper("(char)100");

        ASSERT_SIZE_T(res->len, (size_t)0);
        check_assign_expr_cast_int(res->value,
                                   TYPE_SPEC_CHAR,
                                   IntValue_create_signed(INT_VALUE_I, 100));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper(
            "(struct a_struct){1, var} = 0.0");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        UnaryExpr* unary = res->assign_chain[0].unary;
        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);

        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->len, (size_t)0);

        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);
        check_assign_expr_int(unary->postfix->init_list.inits[0].init.assign,
                              IntValue_create_signed(INT_VALUE_I, 1));
        check_assign_expr_id(unary->postfix->init_list.inits[1].init.assign,
                             "var");

        check_cond_expr_float(res->value,
                              FloatValue_create(FLOAT_VALUE_D, 0.0));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper("var *= (double)12");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_MUL);
        check_unary_expr_id(res->assign_chain[0].unary, "var");

        check_assign_expr_cast_int(res->value,
                                   TYPE_SPEC_DOUBLE,
                                   IntValue_create_signed(INT_VALUE_I, 12));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper(
            "var ^= (struct a_struct){1, var}");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_XOR);
        check_unary_expr_id(res->assign_chain[0].unary, "var");

        UnaryExpr* unary = res->value->last_else->log_ands->or_exprs
                                       ->xor_exprs->and_exprs->eq_exprs->lhs
                                       ->lhs->lhs->lhs->lhs->rhs;

        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);
        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);

        ASSERT(!Designation_is_valid(
            &unary->postfix->init_list.inits[0].designation));
        ASSERT(unary->postfix->init_list.inits[0].init.is_assign);
        check_assign_expr_int(unary->postfix->init_list.inits[0].init.assign,
                              IntValue_create_signed(INT_VALUE_I, 1));
        ASSERT(!Designation_is_valid(
            &unary->postfix->init_list.inits[1].designation));
        ASSERT(unary->postfix->init_list.inits[1].init.is_assign);
        check_assign_expr_id(unary->postfix->init_list.inits[1].init.assign,
                             "var");

        AssignExpr_free(res);
    }
}

void check_assign_expr_single_assign_id(AssignExpr* expr,
                                        const char* lhs,
                                        AssignExprOp op,
                                        const char* rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(expr->assign_chain[0].unary, lhs);
    check_cond_expr_id(expr->value, rhs);
}

void check_assign_expr_single_assign_int(AssignExpr* expr,
                                         const char* lhs,
                                         AssignExprOp op,
                                         IntValue rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(expr->assign_chain[0].unary, lhs);
    check_cond_expr_int(expr->value, rhs);
}

void check_assign_expr_single_assign_float(AssignExpr* expr,
                                           const char* lhs,
                                           AssignExprOp op,
                                           FloatValue rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(expr->assign_chain[0].unary, lhs);
    check_cond_expr_float(expr->value, rhs);
}

TEST(expr) {
    PreprocRes preproc_res = tokenize_string("a = 10, b *= x, c += 3.1",
                                                     "file.c");
    ASSERT_NOT_NULL(preproc_res.toks);

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    Expr* expr = parse_expr(&s);
    ASSERT_NOT_NULL(expr);
    ASSERT(err.kind == PARSER_ERR_NONE);

    ASSERT_SIZE_T(expr->len, (size_t)3);
    check_assign_expr_single_assign_int(&expr->assign_exprs[0],
                                        "a",
                                        ASSIGN_EXPR_ASSIGN,
                                        IntValue_create_signed(INT_VALUE_I, 10));
    check_assign_expr_single_assign_id(&expr->assign_exprs[1],
                                       "b",
                                       ASSIGN_EXPR_MUL,
                                       "x");
    check_assign_expr_single_assign_float(
        &expr->assign_exprs[2],
        "c",
        ASSIGN_EXPR_ADD,
        FloatValue_create(FLOAT_VALUE_D, 3.1));

    Expr_free(expr);
    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
}

TEST_SUITE_BEGIN(parser_expr) {
    REGISTER_TEST(primary_expr),
    REGISTER_TEST(unary_expr),
    REGISTER_TEST(postfix_expr),
    REGISTER_TEST(assign_expr),
    REGISTER_TEST(expr),
}
TEST_SUITE_END()
