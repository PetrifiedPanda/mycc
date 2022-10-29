#include <stdio.h>

#include "util/mem.h"

#include "testing/asserts.h"

#include "frontend/parser/parser_state.h"
#include "frontend/parser/parser.h"

#include "frontend/token.h"
#include "frontend/token_type.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static struct primary_expr* parse_primary_helper(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "not_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    free_parser_state(&s);
    free_preproc_res(&preproc_res);
    return res;
}

static void check_primary_expr_constant(enum constant_type type,
                                        const char* spell,
                                        struct value expected) {
    struct primary_expr* res = parse_primary_helper(spell);

    ASSERT(res->type == PRIMARY_EXPR_CONSTANT);
    ASSERT(res->constant.type == type);
    assert(res->constant.type != CONSTANT_ENUM);

    check_value(res->constant.val, expected);

    free_primary_expr(res);
}

static void check_primary_expr_string(const char* spell) {
    struct primary_expr* res = parse_primary_helper(spell);

    ASSERT(res->type == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res->string.is_func == false);
    ASSERT_STR(str_get_data(&res->string.lit.spelling), spell);

    free_primary_expr(res);
}

static void check_primary_expr_func_name(void) {
    struct primary_expr* res = parse_primary_helper("__func__");

    ASSERT(res->string.is_func == true);

    free_primary_expr(res);
}

static void check_primary_expr_identifier(const char* spell) {
    struct primary_expr* res = parse_primary_helper(spell);

    ASSERT(res->type == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(str_get_data(&res->identifier->spelling), spell);

    free_primary_expr(res);
}

static void check_primary_expr_bracket_id(const char* code,
                                          const char* bracket_spell) {
    struct primary_expr* res = parse_primary_helper(code);
    ASSERT(res->type == PRIMARY_EXPR_BRACKET);
    check_expr_id(res->bracket_expr, bracket_spell);

    free_primary_expr(res);
}

static void check_primary_expr_bracket_const(const char* code,
                                             struct value val) {
    struct primary_expr* res = parse_primary_helper(code);
    ASSERT(res->type == PRIMARY_EXPR_BRACKET);
    check_expr_const(res->bracket_expr, val);

    free_primary_expr(res);
}

static void primary_expr_generic_sel_test(void) {
    struct preproc_res preproc_res = tokenize_string(
        "_Generic(var, int: 0, TypedefName: oops, struct a_struct: 5.0, "
        "default: default_value )",
        "not_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);
    const struct token insert_token = {
        .type = IDENTIFIER,
        .spelling = STR_NON_HEAP("TypedefName"),
        .loc =
            {
                .file_idx = 0,
                .file_loc = {0, 0},
            },
    };

    register_typedef_name(&s, &insert_token);
    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_GENERIC);
    check_assign_expr_id(res->generic->assign, "var");

    ASSERT_SIZE_T(res->generic->assocs.len, (size_t)4);
    struct generic_assoc* assoc = res->generic->assocs.assocs;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.type == TYPE_SPEC_INT);
    check_assign_expr_const(assoc->assign, create_int_value(VALUE_INT, 0));

    ++assoc;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.type == TYPE_SPEC_TYPENAME);
    check_identifier(assoc->type_name->spec_qual_list->specs.typedef_name,
                     "TypedefName");
    check_assign_expr_id(assoc->assign, "oops");

    ++assoc;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.type == TYPE_SPEC_STRUCT);
    ASSERT(
        assoc->type_name->spec_qual_list->specs.struct_union_spec->is_struct);
    ASSERT_SIZE_T(assoc->type_name->spec_qual_list->specs.struct_union_spec
                      ->decl_list.len,
                  (size_t)0);
    check_identifier(
        assoc->type_name->spec_qual_list->specs.struct_union_spec->identifier,
        "a_struct");
    check_assign_expr_const(assoc->assign,
                            create_float_value(VALUE_DOUBLE, 5.0));

    ++assoc;

    ASSERT_NULL(assoc->type_name);
    check_assign_expr_id(assoc->assign, "default_value");

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

TEST(primary_expr) {
    check_primary_expr_constant(CONSTANT_FLOAT,
                                "3.1e-5f",
                                create_float_value(VALUE_FLOAT, 3.1e-5f));
    check_primary_expr_constant(CONSTANT_INT,
                                "0xdeadbeefl",
                                create_int_value(VALUE_LINT, 0xdeadbeefl));
    check_primary_expr_identifier("super_cool_identifier");
    check_primary_expr_identifier("another_cool_identifier");
    check_primary_expr_string("\"Test string it does not matter whether this "
                              "is an actual string literal but hey\"");
    check_primary_expr_func_name();
    check_primary_expr_bracket_const("(23.3)",
                                     create_float_value(VALUE_DOUBLE, 23.3));
    check_primary_expr_bracket_id("(var)", "var");
    primary_expr_generic_sel_test();
}

static struct unary_expr* parse_unary_helper(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "skfjdlfs");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);
    struct unary_expr* res = parse_unary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);

    free_parser_state(&s);
    free_preproc_res(&preproc_res);

    return res;
}

TEST(unary_expr) {
    {
        struct unary_expr* res = parse_unary_helper("++-- sizeof *name");

        ASSERT(res->type == UNARY_DEREF);

        ASSERT_SIZE_T(res->len, (size_t)3);

        ASSERT(res->ops_before[0] == UNARY_OP_INC);
        ASSERT(res->ops_before[1] == UNARY_OP_DEC);
        ASSERT(res->ops_before[2] == UNARY_OP_SIZEOF);

        check_cast_expr_id(res->cast_expr, "name");

        free_unary_expr(res);
    }
    {
        struct unary_expr* res = parse_unary_helper("++++--++--100");

        ASSERT_SIZE_T(res->len, (size_t)5);
        ASSERT(res->type == UNARY_POSTFIX);

        ASSERT(res->ops_before[0] == UNARY_OP_INC);
        ASSERT(res->ops_before[1] == UNARY_OP_INC);
        ASSERT(res->ops_before[2] == UNARY_OP_DEC);
        ASSERT(res->ops_before[3] == UNARY_OP_INC);
        ASSERT(res->ops_before[4] == UNARY_OP_DEC);

        ASSERT(res->postfix->is_primary);
        check_primary_expr_const(res->postfix->primary,
                                 create_int_value(VALUE_INT, 100));

        free_unary_expr(res);
    }

    {
        struct unary_expr* res = parse_unary_helper("sizeof(int)");

        ASSERT(res->type == UNARY_SIZEOF_TYPE);
        ASSERT(res->type_name->spec_qual_list->specs.type == TYPE_SPEC_INT);

        free_unary_expr(res);
    }

    {
        struct unary_expr* res = parse_unary_helper("~*var");

        ASSERT(res->type == UNARY_BNOT);

        struct cast_expr* cast = res->cast_expr;
        ASSERT_SIZE_T(cast->len, (size_t)0);
        struct unary_expr* child_unary = cast->rhs;

        ASSERT(child_unary->type == UNARY_DEREF);
        check_cast_expr_id(child_unary->cast_expr, "var");

        free_unary_expr(res);
    }
}

static struct postfix_expr* parse_postfix_helper(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "sjfkds");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct postfix_expr* res = parse_postfix_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);

    free_parser_state(&s);
    free_preproc_res(&preproc_res);

    return res;
}

static void test_postfix_expr_intializer(bool tailing_comma) {
    char code[] = "(struct a_struct_name){1, test }";
    if (tailing_comma) {
        code[30] = ',';
    }
    struct postfix_expr* res = parse_postfix_helper(code);

    ASSERT(res->is_primary == false);
    ASSERT_SIZE_T(res->init_list.len, (size_t)2);

    ASSERT_NULL(res->init_list.inits[0].designation);
    ASSERT(res->init_list.inits[0].init->is_assign);
    check_assign_expr_const(res->init_list.inits[0].init->assign,
                            create_int_value(VALUE_INT, 1));

    ASSERT_NULL(res->init_list.inits[1].designation);
    ASSERT(res->init_list.inits[1].init->is_assign);
    check_assign_expr_id(res->init_list.inits[1].init->assign, "test");

    free_postfix_expr(res);
}

TEST(postfix_expr) {
    {
        struct postfix_expr* res = parse_postfix_helper(
            "test.ident->other++--++");

        ASSERT(res->is_primary);

        ASSERT_SIZE_T(res->len, (size_t)5);

        check_primary_expr_id(res->primary, "test");

        ASSERT(res->suffixes[0].type == POSTFIX_ACCESS);
        ASSERT_STR(str_get_data(&res->suffixes[0].identifier->spelling),
                   "ident");

        ASSERT(res->suffixes[1].type == POSTFIX_PTR_ACCESS);
        ASSERT_STR(str_get_data(&res->suffixes[1].identifier->spelling),
                   "other");

        ASSERT(res->suffixes[2].type == POSTFIX_INC);

        ASSERT(res->suffixes[3].type == POSTFIX_DEC);

        ASSERT(res->suffixes[4].type == POSTFIX_INC);

        free_postfix_expr(res);
    }

    {
        struct postfix_expr* res = parse_postfix_helper(
            "test[i_am_id]()[23](another_id, 34, id)");

        ASSERT_SIZE_T(res->len, (size_t)4);
        struct postfix_suffix* suffix = res->suffixes;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_id(suffix->index_expr, "i_am_id");

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)0);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_const(suffix->index_expr, create_int_value(VALUE_INT, 23));

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)3);
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[0],
                             "another_id");
        check_assign_expr_const(&suffix->bracket_list.assign_exprs[1],
                                create_int_value(VALUE_INT, 34));
        check_assign_expr_id(&suffix->bracket_list.assign_exprs[2], "id");

        free_postfix_expr(res);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast_const(struct cond_expr* expr,
                                         enum type_spec_type cast_type,
                                         struct value val) {
    struct cast_expr* cast = expr->last_else->log_ands->or_exprs->xor_exprs
                                 ->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs;
    ASSERT_SIZE_T(cast->len, (size_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.type == cast_type);
    check_unary_expr_const(cast->rhs, val);
}

static struct assign_expr* parse_assign_helper(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "blah");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct assign_expr* res = parse_assign_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    free_preproc_res(&preproc_res);
    free_parser_state(&s);

    return res;
}

TEST(assign_expr) {
    {
        struct assign_expr* res = parse_assign_helper("10");

        check_assign_expr_const(res, create_int_value(VALUE_INT, 10));

        free_assign_expr(res);
    }

    {
        struct assign_expr* res = parse_assign_helper(
            "x = 100 += y *= 100.0 /= 2");
        ASSERT_NOT_NULL(res);
        ASSERT_SIZE_T(res->len, (size_t)4);

        enum assign_expr_op expected_ops[] = {
            ASSIGN_EXPR_ASSIGN,
            ASSIGN_EXPR_ADD,
            ASSIGN_EXPR_MUL,
            ASSIGN_EXPR_DIV,
        };

        struct val_or_str {
            enum {
                VAL,
                STR
            } type;
            union {
                struct value val;
                const char* str;
            };
        };
        struct val_or_str expected_spellings[] = {
            {STR, .str = "x"},
            {VAL, .val = create_int_value(VALUE_INT, 100)},
            {STR, .str = "y"},
            {VAL, .val = create_float_value(VALUE_DOUBLE, 100.0)},
        };

        enum {
            SIZE = sizeof expected_ops / sizeof *expected_ops
        };

        for (size_t i = 0; i < SIZE; ++i) {
            ASSERT(res->assign_chain[i].op == expected_ops[i]);

            struct val_or_str curr = expected_spellings[i];
            if (curr.type == VAL) {
                check_unary_expr_const(res->assign_chain[i].unary, curr.val);
            } else {
                check_unary_expr_id(res->assign_chain[i].unary, curr.str);
            }
        }

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        check_unary_expr_id(res->assign_chain[0].unary, "x");

        check_cond_expr_const(res->value, create_int_value(VALUE_INT, 2));

        free_assign_expr(res);
    }

    {
        struct assign_expr* res = parse_assign_helper("(char)100");

        ASSERT_SIZE_T(res->len, (size_t)0);
        check_assign_expr_cast_const(res->value,
                                     TYPE_SPEC_CHAR,
                                     create_int_value(VALUE_INT, 100));

        free_assign_expr(res);
    }

    {
        struct assign_expr* res = parse_assign_helper(
            "(struct a_struct){1, var} = 0.0");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_ASSIGN);

        struct unary_expr* unary = res->assign_chain[0].unary;
        ASSERT(unary->type == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);

        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->len, (size_t)0);

        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);
        check_assign_expr_const(unary->postfix->init_list.inits[0].init->assign,
                                create_int_value(VALUE_INT, 1));
        check_assign_expr_id(unary->postfix->init_list.inits[1].init->assign,
                             "var");

        check_cond_expr_const(res->value,
                              create_float_value(VALUE_DOUBLE, 0.0));

        free_assign_expr(res);
    }

    {
        struct assign_expr* res = parse_assign_helper("var *= (double)12");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_MUL);
        check_unary_expr_id(res->assign_chain[0].unary, "var");

        check_assign_expr_cast_const(res->value,
                                     TYPE_SPEC_DOUBLE,
                                     create_int_value(VALUE_INT, 12));

        free_assign_expr(res);
    }

    {
        struct assign_expr* res = parse_assign_helper(
            "var ^= (struct a_struct){1, var}");

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT(res->assign_chain[0].op == ASSIGN_EXPR_XOR);
        check_unary_expr_id(res->assign_chain[0].unary, "var");

        struct unary_expr* unary = res->value->last_else->log_ands->or_exprs
                                       ->xor_exprs->and_exprs->eq_exprs->lhs
                                       ->lhs->lhs->lhs->lhs->rhs;

        ASSERT(unary->type == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);
        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);

        ASSERT_NULL(unary->postfix->init_list.inits[0].designation);
        ASSERT(unary->postfix->init_list.inits[0].init->is_assign);
        check_assign_expr_const(unary->postfix->init_list.inits[0].init->assign,
                                create_int_value(VALUE_INT, 1));
        ASSERT_NULL(unary->postfix->init_list.inits[1].designation);
        ASSERT(unary->postfix->init_list.inits[1].init->is_assign);
        check_assign_expr_id(unary->postfix->init_list.inits[1].init->assign,
                             "var");

        free_assign_expr(res);
    }
}

void check_assign_expr_single_assign_id(struct assign_expr* expr,
                                        const char* lhs,
                                        enum assign_expr_op op,
                                        const char* rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(expr->assign_chain[0].unary, lhs);
    check_cond_expr_id(expr->value, rhs);
}

void check_assign_expr_single_assign_const(struct assign_expr* expr,
                                           const char* lhs,
                                           enum assign_expr_op op,
                                           struct value rhs) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT(expr->assign_chain[0].op == op);
    check_unary_expr_id(expr->assign_chain[0].unary, lhs);
    check_cond_expr_const(expr->value, rhs);
}

TEST(expr) {
    struct preproc_res preproc_res = tokenize_string("a = 10, b *= x, c += 3.1",
                                                     "file.c");
    ASSERT_NOT_NULL(preproc_res.toks);

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct expr* expr = parse_expr(&s);
    ASSERT_NOT_NULL(expr);
    ASSERT(err.type == PARSER_ERR_NONE);

    ASSERT_SIZE_T(expr->len, (size_t)3);
    check_assign_expr_single_assign_const(&expr->assign_exprs[0],
                                          "a",
                                          ASSIGN_EXPR_ASSIGN,
                                          create_int_value(VALUE_INT, 10));
    check_assign_expr_single_assign_id(&expr->assign_exprs[1],
                                       "b",
                                       ASSIGN_EXPR_MUL,
                                       "x");
    check_assign_expr_single_assign_const(
        &expr->assign_exprs[2],
        "c",
        ASSIGN_EXPR_ADD,
        create_float_value(VALUE_DOUBLE, 3.1));

    free_expr(expr);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

TEST_SUITE_BEGIN(parser_expr, 5) {
    REGISTER_TEST(primary_expr);
    REGISTER_TEST(unary_expr);
    REGISTER_TEST(postfix_expr);
    REGISTER_TEST(assign_expr);
    REGISTER_TEST(expr);
}
TEST_SUITE_END()
