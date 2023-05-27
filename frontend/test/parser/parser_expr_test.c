#include <stdio.h>

#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/parser/ParserState.h"
#include "frontend/parser/parser.h"

#include "frontend/Token.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static PrimaryExpr parse_primary_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "not_file.c");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);
    UnaryExpr unary;
    ASSERT(parse_unary_expr_inplace(&s, &unary));
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(s.it->kind, TOKEN_INVALID);
    ASSERT(unary.kind == UNARY_POSTFIX);
    ASSERT_SIZE_T(unary.len, 0);

    ASSERT(unary.postfix.is_primary);
    ASSERT_SIZE_T(unary.postfix.len, 0);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
    return unary.postfix.primary;
}

static void check_primary_expr_int_constant(ConstantKind type,
                                            const char* spell,
                                            Value expected) {
    PrimaryExpr res = parse_primary_helper(spell);

    ASSERT(res.kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(res.constant.kind == type);
    assert(res.constant.kind != CONSTANT_ENUM);

    check_value(res.constant.val, expected);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_float_constant(ConstantKind type,
                                              const char* spell,
                                              Value expected) {
    PrimaryExpr res = parse_primary_helper(spell);

    ASSERT(res.kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(res.constant.kind == type);
    assert(res.constant.kind != CONSTANT_ENUM);

    check_value(res.constant.val, expected);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_string(const char* spell, const char* expected) {
    PrimaryExpr res = parse_primary_helper(spell);

    ASSERT(res.kind == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res.string.is_func == false);
    ASSERT_STR(Str_get_data(&res.string.lit.lit.contents), expected);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_func_name(void) {
    PrimaryExpr res = parse_primary_helper("__func__");

    ASSERT(res.string.is_func == true);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_identifier(const char* spell) {
    PrimaryExpr res = parse_primary_helper(spell);

    ASSERT(res.kind == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(Str_get_data(&res.identifier->spelling), spell);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_bracket_id(const char* code,
                                          const char* bracket_spell) {
    PrimaryExpr res = parse_primary_helper(code);
    ASSERT(res.kind == PRIMARY_EXPR_BRACKET);
    check_expr_id(&res.bracket_expr, bracket_spell);

    PrimaryExpr_free_children(&res);
}

static void check_primary_expr_bracket_float(const char* code,
                                             Value val) {
    PrimaryExpr res = parse_primary_helper(code);
    ASSERT(res.kind == PRIMARY_EXPR_BRACKET);
    check_expr_val(&res.bracket_expr, val);

    PrimaryExpr_free_children(&res);
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
    UnaryExpr unary;
    ASSERT(parse_unary_expr_inplace(&s, &unary));
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_TOKEN_KIND(s.it->kind, TOKEN_INVALID);
    ASSERT(unary.kind == UNARY_POSTFIX);
    ASSERT_SIZE_T(unary.len, 0);

    ASSERT(unary.postfix.is_primary);
    ASSERT_SIZE_T(unary.postfix.len, 0);

    PrimaryExpr res = unary.postfix.primary;

    ASSERT(res.kind == PRIMARY_EXPR_GENERIC);
    check_assign_expr_id(res.generic.assign, "var");

    ASSERT_SIZE_T(res.generic.assocs.len, (size_t)4);
    GenericAssoc* assoc = res.generic.assocs.assocs;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.kind == TYPE_SPEC_INT);
    check_assign_expr_val(assoc->assign, Value_create_sint(VALUE_I, 0));

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
    check_assign_expr_val(assoc->assign,
                            Value_create_float(VALUE_D, 5.0));

    ++assoc;

    ASSERT_NULL(assoc->type_name);
    check_assign_expr_id(assoc->assign, "default_value");

    PrimaryExpr_free_children(&res);
    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
}

TEST(primary_expr) {
    check_primary_expr_float_constant(
        CONSTANT_VAL,
        "3.1e-5f",
        Value_create_float(VALUE_F, 3.1e-5f));
    check_primary_expr_int_constant(CONSTANT_VAL,
                                    "0xdeadbeefl",
                                    Value_create_sint(VALUE_L, 0xdeadbeefl));
    check_primary_expr_identifier("super_cool_identifier");
    check_primary_expr_identifier("another_cool_identifier");
    check_primary_expr_string("\"Test string it does not matter whether this "
                              "is an actual string literal but hey\"",
                              "Test string it does not matter whether this "
                              "is an actual string literal but hey");
    check_primary_expr_func_name();
    check_primary_expr_bracket_float("(23.3)",
                                     Value_create_float(VALUE_D, 23.3));
    check_primary_expr_bracket_id("(var)", "var");
    primary_expr_generic_sel_test();
}

static UnaryExpr parse_unary_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "skfjdlfs");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);
    UnaryExpr res;
    ASSERT(parse_unary_expr_inplace(&s, &res));
    ASSERT(err.kind == PARSER_ERR_NONE);

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);

    return res;
}

TEST(unary_expr) {
    {
        UnaryExpr res = parse_unary_helper("++-- sizeof *name");

        ASSERT(res.kind == UNARY_DEREF);

        ASSERT_SIZE_T(res.len, (size_t)3);

        ASSERT(res.ops_before[0] == UNARY_OP_INC);
        ASSERT(res.ops_before[1] == UNARY_OP_DEC);
        ASSERT(res.ops_before[2] == UNARY_OP_SIZEOF);

        check_cast_expr_id(res.cast_expr, "name");

        UnaryExpr_free_children(&res);
    }
    {
        UnaryExpr res = parse_unary_helper("++++--++--100");

        ASSERT_SIZE_T(res.len, (size_t)5);
        ASSERT(res.kind == UNARY_POSTFIX);

        ASSERT(res.ops_before[0] == UNARY_OP_INC);
        ASSERT(res.ops_before[1] == UNARY_OP_INC);
        ASSERT(res.ops_before[2] == UNARY_OP_DEC);
        ASSERT(res.ops_before[3] == UNARY_OP_INC);
        ASSERT(res.ops_before[4] == UNARY_OP_DEC);

        ASSERT(res.postfix.is_primary);
        check_primary_expr_val(&res.postfix.primary,
                               Value_create_sint(VALUE_I, 100));

        UnaryExpr_free_children(&res);
    }

    {
        UnaryExpr res = parse_unary_helper("sizeof(int)");

        ASSERT(res.kind == UNARY_SIZEOF_TYPE);
        ASSERT(res.type_name->spec_qual_list->specs.kind == TYPE_SPEC_INT);

        UnaryExpr_free_children(&res);
    }

    {
        UnaryExpr res = parse_unary_helper("~*var");

        ASSERT(res.kind == UNARY_BNOT);

        CastExpr* cast = res.cast_expr;
        ASSERT_SIZE_T(cast->len, (size_t)0);
        UnaryExpr* child_unary = &cast->rhs;

        ASSERT(child_unary->kind == UNARY_DEREF);
        check_cast_expr_id(child_unary->cast_expr, "var");

        UnaryExpr_free_children(&res);
    }
}

static PostfixExpr parse_postfix_helper(const char* code) {
    PreprocRes preproc_res = tokenize_string(code, "sjfkds");

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);
    
    UnaryExpr unary;
    ASSERT(parse_unary_expr_inplace(&s, &unary));
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_SIZE_T(unary.len, 0);
    ASSERT(unary.kind == UNARY_POSTFIX);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);

    return unary.postfix;
}

static void test_postfix_expr_intializer(bool tailing_comma) {
    char code[] = "(struct a_struct_name){1, test }";
    if (tailing_comma) {
        code[30] = ',';
    }
    PostfixExpr res = parse_postfix_helper(code);

    ASSERT(res.is_primary == false);
    ASSERT_SIZE_T(res.init_list.len, (size_t)2);

    ASSERT(!Designation_is_valid(&res.init_list.inits[0].designation));
    ASSERT(res.init_list.inits[0].init.is_assign);
    check_assign_expr_val(res.init_list.inits[0].init.assign,
                          Value_create_sint(VALUE_I, 1));

    ASSERT(!Designation_is_valid(&res.init_list.inits[1].designation));
    ASSERT(res.init_list.inits[1].init.is_assign);
    check_assign_expr_id(res.init_list.inits[1].init.assign, "test");

    PostfixExpr_free_children(&res);
}

TEST(postfix_expr) {
    {
        PostfixExpr res = parse_postfix_helper("test.ident->other++--++");

        ASSERT(res.is_primary);

        ASSERT_SIZE_T(res.len, (size_t)5);

        check_primary_expr_id(&res.primary, "test");

        ASSERT(res.suffixes[0].kind == POSTFIX_ACCESS);
        ASSERT_STR(Str_get_data(&res.suffixes[0].identifier->spelling),
                   "ident");

        ASSERT(res.suffixes[1].kind == POSTFIX_PTR_ACCESS);
        ASSERT_STR(Str_get_data(&res.suffixes[1].identifier->spelling),
                   "other");

        ASSERT(res.suffixes[2].kind == POSTFIX_INC);

        ASSERT(res.suffixes[3].kind == POSTFIX_DEC);

        ASSERT(res.suffixes[4].kind == POSTFIX_INC);

        PostfixExpr_free_children(&res);
    }

    {
        PostfixExpr res = parse_postfix_helper("test[i_am_id]()[23](another_id, 34, id)");

        ASSERT_SIZE_T(res.len, (size_t)4);
        PostfixSuffix* suffix = res.suffixes;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_id(&suffix->index_expr, "i_am_id");

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)0);

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_INDEX);
        check_expr_val(&suffix->index_expr, Value_create_sint(VALUE_I, 23));

        ++suffix;

        ASSERT(suffix->kind == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)3);
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[0],
                             "another_id");
        check_assign_expr_val(&suffix->bracket_list.assign_exprs[1],
                              Value_create_sint(VALUE_I, 34));
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[2], "id");

        PostfixExpr_free_children(&res);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast_int(CondExpr* expr,
                                       TypeSpecKind cast_type,
                                       Value val) {
    CastExpr* cast = &expr->last_else.log_ands->or_exprs->xor_exprs
                                 ->and_exprs->eq_exprs->lhs.lhs.lhs.lhs.lhs;
    ASSERT_SIZE_T(cast->len, (size_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.kind == cast_type);
    check_unary_expr_val(&cast->rhs, val);
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

        check_assign_expr_val(res, Value_create_sint(VALUE_I, 10));

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
                VAL,
                STR
            } type;
            union {
                Value val;
                const char* str;
            };
        } ValOrStr;
        ValOrStr expected_spellings[] = {
            {STR, .str = "x"},
            {VAL, .val = Value_create_sint(VALUE_I, 100)},
            {STR, .str = "y"},
            {VAL, .val = Value_create_float(VALUE_D, 100.0)},
        };

        enum {
            LEN = ARR_LEN(expected_ops)
        };

        for (size_t i = 0; i < LEN; ++i) {
            ASSERT(res->assign_chain[i].op == expected_ops[i]);

            ValOrStr curr = expected_spellings[i];
            switch (curr.type) {
                case VAL:
                    check_unary_expr_val(&res->assign_chain[i].unary,
                                         curr.val);
                    break;
                case STR:
                    check_unary_expr_id(&res->assign_chain[i].unary, curr.str);
                    break;
            }
        }

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        check_unary_expr_id(&res->assign_chain[0].unary, "x");

        check_cond_expr_val(&res->value, Value_create_sint(VALUE_I, 2));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper("(char)100");

        ASSERT_SIZE_T(res->len, (size_t)0);
        check_assign_expr_cast_int(&res->value,
                                   TYPE_SPEC_CHAR,
                                   Value_create_sint(VALUE_I, 100));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper(
            "(struct a_struct){1, var} = 0.0");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        UnaryExpr* unary = &res->assign_chain[0].unary;
        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);

        ASSERT(unary->postfix.is_primary == false);
        ASSERT_SIZE_T(unary->postfix.len, (size_t)0);

        ASSERT_SIZE_T(unary->postfix.init_list.len, (size_t)2);
        check_assign_expr_val(unary->postfix.init_list.inits[0].init.assign,
                              Value_create_sint(VALUE_I, 1));
        check_assign_expr_id(unary->postfix.init_list.inits[1].init.assign,
                             "var");

        check_cond_expr_val(&res->value,
                              Value_create_float(VALUE_D, 0.0));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper("var *= (double)12");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_MUL);
        check_unary_expr_id(&res->assign_chain[0].unary, "var");

        check_assign_expr_cast_int(&res->value,
                                   TYPE_SPEC_DOUBLE,
                                   Value_create_sint(VALUE_I, 12));

        AssignExpr_free(res);
    }

    {
        AssignExpr* res = parse_assign_helper(
            "var ^= (struct a_struct){1, var}");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_XOR);
        check_unary_expr_id(&res->assign_chain[0].unary, "var");

        UnaryExpr* unary = &res->value.last_else.log_ands->or_exprs
                                       ->xor_exprs->and_exprs->eq_exprs->lhs
                                       .lhs.lhs.lhs.lhs.rhs;

        ASSERT(unary->kind == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);
        ASSERT(unary->postfix.is_primary == false);
        ASSERT_SIZE_T(unary->postfix.init_list.len, (size_t)2);

        ASSERT(!Designation_is_valid(
            &unary->postfix.init_list.inits[0].designation));
        ASSERT(unary->postfix.init_list.inits[0].init.is_assign);
        check_assign_expr_val(unary->postfix.init_list.inits[0].init.assign,
                              Value_create_sint(VALUE_I, 1));
        ASSERT(!Designation_is_valid(
            &unary->postfix.init_list.inits[1].designation));
        ASSERT(unary->postfix.init_list.inits[1].init.is_assign);
        check_assign_expr_id(unary->postfix.init_list.inits[1].init.assign,
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
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs);
    check_cond_expr_id(&expr->value, rhs);
}

void check_assign_expr_single_assign_int(AssignExpr* expr,
                                         const char* lhs,
                                         AssignExprOp op,
                                         Value rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs);
    check_cond_expr_val(&expr->value, rhs);
}

void check_assign_expr_single_assign_float(AssignExpr* expr,
                                           const char* lhs,
                                           AssignExprOp op,
                                           Value rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(&expr->assign_chain[0].unary, lhs);
    check_cond_expr_val(&expr->value, rhs);
}

TEST(expr) {
    PreprocRes preproc_res = tokenize_string("a = 10, b *= x, c += 3.1",
                                                     "file.c");
    ASSERT_NOT_NULL(preproc_res.toks);

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(preproc_res.toks, &err);

    Expr expr; 
    ASSERT(parse_expr_inplace(&s, &expr));
    ASSERT(err.kind == PARSER_ERR_NONE);

    ASSERT_SIZE_T(expr.len, (size_t)3);
    check_assign_expr_single_assign_int(&expr.assign_exprs[0],
                                        "a",
                                        ASSIGN_EXPR_ASSIGN,
                                        Value_create_sint(VALUE_I, 10));
    check_assign_expr_single_assign_id(&expr.assign_exprs[1],
                                       "b",
                                       ASSIGN_EXPR_MUL,
                                       "x");
    check_assign_expr_single_assign_float(
        &expr.assign_exprs[2],
        "c",
        ASSIGN_EXPR_ADD,
        Value_create_float(VALUE_D, 3.1));

    Expr_free_children(&expr);
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
