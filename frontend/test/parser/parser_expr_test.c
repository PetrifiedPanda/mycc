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

static void check_primary_expr_constant(enum token_type type,
                                        const char* spell) {
    struct preproc_res preproc_res = tokenize_string(
        spell,
        "not a file so I can write whatever here");
    
    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_CONSTANT);
    ASSERT_TOKEN_TYPE(res->constant.type, type);
    ASSERT_STR(res->constant.spelling, spell);

    ASSERT_NULL(preproc_res.toks[0].spelling);

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

static void check_primary_expr_string(const char* spell) {
    struct preproc_res preproc_res = tokenize_string(spell, "no_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_STRING_LITERAL);
    ASSERT(res->string.is_func == false);
    ASSERT_STR(res->string.lit.spelling, spell);

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

static void check_primary_expr_func_name(void) {
    struct preproc_res preproc_res = tokenize_string("__func__", "not a file so go away");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->string.is_func == true);

    free_preproc_res(&preproc_res);
    free_parser_state(&s);
    free_primary_expr(res);
}

static void check_primary_expr_identifier(const char* spell) {
    struct preproc_res preproc_res = tokenize_string(spell, "a string");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT(err.type == PARSER_ERR_NONE);

    ASSERT(res->type == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_STR(res->identifier->spelling, spell);

    ASSERT_NULL(preproc_res.toks[0].spelling);

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

static void check_primary_expr_bracket(const char* code,
                                       const char* cont_spell,
                                       enum token_type expr_type) {
    struct preproc_res preproc_res = tokenize_string(code, "not_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct primary_expr* res = parse_primary_expr(&s);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);

    ASSERT(res->type == PRIMARY_EXPR_BRACKET);
    check_expr_id_or_const(res->bracket_expr, cont_spell, expr_type);

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

static void primary_expr_generic_sel_test(void) {
    struct preproc_res preproc_res = tokenize_string(
        "_Generic(var, int: 0, TypedefName: oops, struct a_struct: 5.0, "
        "default: default_value )",
        "not_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);
    struct token insert_token = {
        .type = IDENTIFIER,
        .spelling = "TypedefName",
        .loc = {
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
    check_assign_expr_id_or_const(res->generic->assign, "var", IDENTIFIER);

    ASSERT_SIZE_T(res->generic->assocs.len, (size_t)4);
    struct generic_assoc* assoc = res->generic->assocs.assocs;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.type == TYPE_SPEC_INT);
    check_assign_expr_id_or_const(assoc->assign, "0", I_CONSTANT);

    ++assoc;

    ASSERT_NULL(assoc->type_name->abstract_decl);
    ASSERT(assoc->type_name->spec_qual_list->specs.type == TYPE_SPEC_TYPENAME);
    check_identifier(assoc->type_name->spec_qual_list->specs.typedef_name,
                     "TypedefName");
    check_assign_expr_id_or_const(assoc->assign, "oops", IDENTIFIER);

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
    check_assign_expr_id_or_const(assoc->assign, "5.0", F_CONSTANT);

    ++assoc;

    ASSERT_NULL(assoc->type_name);
    check_assign_expr_id_or_const(assoc->assign, "default_value", IDENTIFIER);

    free_primary_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

TEST(primary_expr) {
    check_primary_expr_constant(F_CONSTANT, "3.1e-5f");
    check_primary_expr_constant(I_CONSTANT, "0xdeadbeefl");
    check_primary_expr_identifier("super_cool_identifier");
    check_primary_expr_identifier("another_cool_identifier");
    check_primary_expr_string("\"Test string it does not matter whether this "
                              "is an actual string literal but hey\"");
    check_primary_expr_func_name();
    check_primary_expr_bracket("(23.3)", "23.3", F_CONSTANT);
    check_primary_expr_bracket("(var)", "var", IDENTIFIER);
    primary_expr_generic_sel_test();
}

TEST(unary_expr) {
    {
        struct preproc_res preproc_res = tokenize_string("++-- sizeof *name", "skfjdlfs");
        
        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);
        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT(err.type == PARSER_ERR_NONE);

        ASSERT(res->type == UNARY_UNARY_OP);

        ASSERT_SIZE_T(res->len, (size_t)3);

        ASSERT_TOKEN_TYPE(res->operators_before[0], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[1], DEC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[2], SIZEOF);

        ASSERT_TOKEN_TYPE(res->unary_op, ASTERISK);
        check_cast_expr_id_or_const(res->cast_expr, "name", IDENTIFIER);

        free_unary_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }
    {
        struct preproc_res preproc_res = tokenize_string("++++--++--100", "ksjflkdsjf");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);
        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)5);
        ASSERT(res->type == UNARY_POSTFIX);

        ASSERT_TOKEN_TYPE(res->operators_before[0], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[1], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[2], DEC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[3], INC_OP);
        ASSERT_TOKEN_TYPE(res->operators_before[4], DEC_OP);

        ASSERT(res->postfix->is_primary);
        check_primary_expr_id_or_const(res->postfix->primary,
                                       "100",
                                       I_CONSTANT);

        free_unary_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("sizeof(int)", "no");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT(res->type == UNARY_SIZEOF_TYPE);
        ASSERT(res->type_name->spec_qual_list->specs.type == TYPE_SPEC_INT);

        free_unary_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("~*var", "a file");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct unary_expr* res = parse_unary_expr(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT(res->type == UNARY_UNARY_OP);
        ASSERT_TOKEN_TYPE(res->unary_op, BNOT);

        struct cast_expr* cast = res->cast_expr;
        ASSERT_SIZE_T(cast->len, (size_t)0);
        struct unary_expr* child_unary = cast->rhs;

        ASSERT(child_unary->type == UNARY_UNARY_OP);
        ASSERT_TOKEN_TYPE(child_unary->unary_op, ASTERISK);
        check_cast_expr_id_or_const(child_unary->cast_expr, "var", IDENTIFIER);

        free_unary_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }
}

static void test_postfix_expr_intializer(bool tailing_comma) {
    char* code = alloc_string_copy("(struct a_struct_name){1, test }");
    if (tailing_comma) {
        code[30] = ',';
    }
    struct preproc_res preproc_res = tokenize_string(code, "not_a_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct postfix_expr* res = parse_postfix_expr(&s);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_NOT_NULL(res);

    ASSERT(res->is_primary == false);
    ASSERT_SIZE_T(res->init_list.len, (size_t)2);

    ASSERT_NULL(res->init_list.inits[0].designation);
    ASSERT(res->init_list.inits[0].init->is_assign);
    check_assign_expr_id_or_const(res->init_list.inits[0].init->assign,
                                  "1",
                                  I_CONSTANT);

    ASSERT_NULL(res->init_list.inits[1].designation);
    ASSERT(res->init_list.inits[1].init->is_assign);
    check_assign_expr_id_or_const(res->init_list.inits[1].init->assign,
                                  "test",
                                  IDENTIFIER);

    free(code);
    free_postfix_expr(res);
    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

TEST(postfix_expr) {
    {
        struct preproc_res preproc_res = tokenize_string("test.ident->other++--++",
                                              "sjfkds");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct postfix_expr* res = parse_postfix_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT(err.type == PARSER_ERR_NONE);

        ASSERT(res->is_primary);

        ASSERT_SIZE_T(res->len, (size_t)5);

        check_primary_expr_id_or_const(res->primary, "test", IDENTIFIER);

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
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string(
            "test[i_am_id]()[23](another_id, 34, id)",
            "not_a_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct postfix_expr* res = parse_postfix_expr(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)4);
        struct postfix_suffix* suffix = res->suffixes;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_id_or_const(suffix->index_expr, "i_am_id", IDENTIFIER);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)0);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_INDEX);
        check_expr_id_or_const(suffix->index_expr, "23", I_CONSTANT);

        ++suffix;

        ASSERT(suffix->type == POSTFIX_BRACKET);
        ASSERT_SIZE_T(suffix->bracket_list.len, (size_t)3);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[0],
                                      "another_id",
                                      IDENTIFIER);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[1],
                                      "34",
                                      I_CONSTANT);
        check_assign_expr_id_or_const(&suffix->bracket_list.assign_exprs[2],
                                      "id",
                                      IDENTIFIER);

        free_postfix_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    test_postfix_expr_intializer(true);
    test_postfix_expr_intializer(false);
}

static void check_assign_expr_cast(struct cond_expr* expr,
                                   enum type_spec_type cast_type,
                                   const char* spell,
                                   enum token_type value_type) {
    struct cast_expr* cast = expr->last_else->log_ands->or_exprs->xor_exprs
                                 ->and_exprs->eq_exprs->lhs->lhs->lhs->lhs->lhs;
    ASSERT_SIZE_T(cast->len, (size_t)1);
    ASSERT(cast->type_names[0].spec_qual_list->specs.type == cast_type);
    check_unary_expr_id_or_const(cast->rhs, spell, value_type);
}

TEST(assign_expr) {
    {
        struct preproc_res preproc_res = tokenize_string("10", "blah");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        check_assign_expr_id_or_const(res, "10", I_CONSTANT);

        free_preproc_res(&preproc_res);
        free_parser_state(&s);
        free_assign_expr(res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("x = 100 += y *= 100.0 /= 2",
                                              "not a file");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_SIZE_T(res->len, (size_t)4);
        ASSERT(err.type == PARSER_ERR_NONE);

        enum token_type expected_ops[] = {
            ASSIGN,
            ADD_ASSIGN,
            MUL_ASSIGN,
            DIV_ASSIGN,
        };

        enum token_type expected_types[] = {
            IDENTIFIER,
            I_CONSTANT,
            IDENTIFIER,
            F_CONSTANT,
        };

        const char* expected_spellings[] = {"x", "100", "y", "100.0"};

        enum { SIZE = sizeof expected_ops / sizeof(enum token_type) };

        for (size_t i = 0; i < SIZE; ++i) {
            ASSERT_TOKEN_TYPE(res->assign_chain[i].assign_op, expected_ops[i]);

            check_unary_expr_id_or_const(res->assign_chain[i].unary,
                                         expected_spellings[i],
                                         expected_types[i]);
        }

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, ASSIGN);

        check_unary_expr_id_or_const(res->assign_chain[0].unary,
                                     "x",
                                     IDENTIFIER);

        check_cond_expr_id_or_const(res->value, "2", I_CONSTANT);

        free_preproc_res(&preproc_res);
        free_parser_state(&s);
        free_assign_expr(res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("(char)100", "not_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);
        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)0);
        check_assign_expr_cast(res->value, TYPE_SPEC_CHAR, "100", I_CONSTANT);

        free_assign_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("(struct a_struct){1, var} = 0.0",
                                              "not_a_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, ASSIGN);

        struct unary_expr* unary = res->assign_chain[0].unary;
        ASSERT(unary->type == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);

        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->len, (size_t)0);

        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);
        check_assign_expr_id_or_const(
            unary->postfix->init_list.inits[0].init->assign,
            "1",
            I_CONSTANT);
        check_assign_expr_id_or_const(
            unary->postfix->init_list.inits[1].init->assign,
            "var",
            IDENTIFIER);

        check_cond_expr_id_or_const(res->value, "0.0", F_CONSTANT);

        free_assign_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string("var *= (double)12",
                                              "not_a_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, MUL_ASSIGN);
        check_unary_expr_id_or_const(res->assign_chain[0].unary,
                                     "var",
                                     IDENTIFIER);

        check_assign_expr_cast(res->value, TYPE_SPEC_DOUBLE, "12", I_CONSTANT);

        free_assign_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string(
            "var ^= (struct a_struct){1, var}",
            "not_a_file.c");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct assign_expr* res = parse_assign_expr(&s);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);

        ASSERT_SIZE_T(res->len, (size_t)1);

        ASSERT_TOKEN_TYPE(res->assign_chain[0].assign_op, XOR_ASSIGN);
        check_unary_expr_id_or_const(res->assign_chain[0].unary,
                                     "var",
                                     IDENTIFIER);

        struct unary_expr* unary = res->value->last_else->log_ands->or_exprs
                                       ->xor_exprs->and_exprs->eq_exprs->lhs
                                       ->lhs->lhs->lhs->lhs->rhs;

        ASSERT(unary->type == UNARY_POSTFIX);
        ASSERT_SIZE_T(unary->len, (size_t)0);
        ASSERT(unary->postfix->is_primary == false);
        ASSERT_SIZE_T(unary->postfix->init_list.len, (size_t)2);

        ASSERT_NULL(unary->postfix->init_list.inits[0].designation);
        ASSERT(unary->postfix->init_list.inits[0].init->is_assign);
        check_assign_expr_id_or_const(
            unary->postfix->init_list.inits[0].init->assign,
            "1",
            I_CONSTANT);
        ASSERT_NULL(unary->postfix->init_list.inits[1].designation);
        ASSERT(unary->postfix->init_list.inits[1].init->is_assign);
        check_assign_expr_id_or_const(
            unary->postfix->init_list.inits[1].init->assign,
            "var",
            IDENTIFIER);

        free_assign_expr(res);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }
}

void check_assign_expr_single_assign(struct assign_expr* expr,
                                     const char* lhs,
                                     enum token_type op,
                                     const char* rhs,
                                     enum token_type rhs_type) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT_TOKEN_TYPE(expr->assign_chain[0].assign_op, op);
    check_unary_expr_id_or_const(expr->assign_chain[0].unary, lhs, IDENTIFIER);
    check_cond_expr_id_or_const(expr->value, rhs, rhs_type);
}

TEST(expr) {
    struct preproc_res preproc_res = tokenize_string("a = 10, b *= x, c += 3.1", "file.c");
    ASSERT_NOT_NULL(preproc_res.toks);

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct expr* expr = parse_expr(&s);
    ASSERT_NOT_NULL(expr);
    ASSERT(err.type == PARSER_ERR_NONE);

    ASSERT_SIZE_T(expr->len, (size_t)3);
    check_assign_expr_single_assign(&expr->assign_exprs[0], "a", ASSIGN, "10", I_CONSTANT);
    check_assign_expr_single_assign(&expr->assign_exprs[1], "b", MUL_ASSIGN, "x", IDENTIFIER);
    check_assign_expr_single_assign(&expr->assign_exprs[2], "c", ADD_ASSIGN, "3.1", F_CONSTANT);

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
