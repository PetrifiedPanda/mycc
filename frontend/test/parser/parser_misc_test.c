#include "preproc/preproc.h"

#include "testing/asserts.h"

#include "parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void check_enum_list_ids(struct enum_list* l,
                                const char** enum_constants,
                                size_t len) {
    ASSERT_SIZE_T(l->len, len);
    for (size_t i = 0; i < len; ++i) {
        ASSERT_NOT_NULL(l->enums[i].identifier);
        ASSERT_STR(l->enums[i].identifier->spelling, enum_constants[i]);
    }
}

TEST(enum_list) {
    enum { EXPECTED_LEN = 8 };
    const char* enum_constants[EXPECTED_LEN] = {
        "ENUM_VAL1",
        "enum_VAl2",
        "enum_val_3",
        "enum_val_4",
        "foo",
        "bar",
        "baz",
        "BAD",
    };

    {
        struct token* tokens = tokenize_string(
            "ENUM_VAL1, enum_VAl2, enum_val_3, enum_val_4, foo, bar, baz, BAD",
            "saffds");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct enum_list res = parse_enum_list(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_SIZE_T(res.len, (size_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res.enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res.enums[i].identifier);
            ASSERT_NULL(res.enums[i].enum_val);
        }
        check_enum_list_ids(&res,
                            enum_constants,
                            sizeof enum_constants / sizeof(char*));

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(is_enum_constant(&s, enum_constants[i]));
        }

        free_enum_list(&res);
        free_parser_state(&s);
        free_tokens(tokens);
    }

    {
        struct token* tokens = tokenize_string(
            "ENUM_VAL1 = 0, enum_VAl2 = 1000.0, enum_val_3 = n, enum_val_4 = "
            "test, foo, bar, baz, BAD",
            "saffds");
        
        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct enum_list res = parse_enum_list(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);
        ASSERT_SIZE_T(res.len, (size_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res.enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res.enums[i].identifier);
        }

        ASSERT_NOT_NULL(res.enums[0].enum_val);
        check_const_expr_id_or_const(res.enums[0].enum_val, "0", I_CONSTANT);

        ASSERT_NOT_NULL(res.enums[1].enum_val);
        check_const_expr_id_or_const(res.enums[1].enum_val,
                                     "1000.0",
                                     F_CONSTANT);

        ASSERT_NOT_NULL(res.enums[2].enum_val);
        check_const_expr_id_or_const(res.enums[2].enum_val, "n", IDENTIFIER);

        ASSERT_NOT_NULL(res.enums[3].enum_val);
        check_const_expr_id_or_const(res.enums[3].enum_val, "test", IDENTIFIER);

        for (size_t i = 4; i < EXPECTED_LEN; ++i) {
            ASSERT_NULL(res.enums[i].enum_val);
        }

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(is_enum_constant(&s, enum_constants[i]));
        }

        free_enum_list(&res);
        free_parser_state(&s);
        free_tokens(tokens);
    }
}

TEST(enum_spec) {
    const char* enum_constants[] = {"TEST1", "TEST2", "TEST3", "TEST4"};
    {
        struct token* tokens = tokenize_string(
            "enum my_enum { TEST1, TEST2, TEST3, TEST4 }",
            "sfjlfjk");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_NOT_NULL(res->identifier);
        ASSERT_STR(res->identifier->spelling, "my_enum");

        check_enum_list_ids(&res->enum_list,
                            enum_constants,
                            sizeof enum_constants / sizeof(char*));

        free_tokens(tokens);
        free_parser_state(&s);
        free_enum_spec(res);
    }

    {
        struct token* tokens = tokenize_string(
            "enum {TEST1, TEST2, TEST3, TEST4, }",
            "jsfjsf");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_NULL(res->identifier);

        check_enum_list_ids(&res->enum_list,
                            enum_constants,
                            sizeof enum_constants / sizeof(char*));

        free_tokens(tokens);
        free_parser_state(&s);
        free_enum_spec(res);
    }
}

TEST(designation) {
    {
        struct token* tokens = tokenize_string(
            ".test[19].what_is_this.another_one = ",
            "jsalkf");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct designation* res = parse_designation(&s);
        ASSERT_NOT_NULL(res);
        ASSERT_NOT_NULL(res->designators.designators);
        ASSERT(err.type == PARSER_ERR_NONE);

        ASSERT_TOKEN_TYPE(s.it->type, INVALID);

        ASSERT_SIZE_T(res->designators.len, (size_t)4);

        struct designator* designators = res->designators.designators;
        ASSERT(designators[0].is_index == false);
        check_identifier(designators[0].identifier, "test");

        ASSERT(designators[1].is_index == true);
        check_cond_expr_id_or_const(&designators[1].arr_index->expr,
                                    "19",
                                    I_CONSTANT);

        ASSERT(designators[2].is_index == false);
        check_identifier(designators[2].identifier, "what_is_this");

        ASSERT(designators[3].is_index == false);
        check_identifier(designators[3].identifier, "another_one");

        free_designation(res);
        free_parser_state(&s);
        free_tokens(tokens);
    }

    {
        struct token* tokens = tokenize_string("[0.5].blah[420].oof[2][10] =",
                                              "stetsd");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(tokens, &err);

        struct designation* res = parse_designation(&s);
        ASSERT(err.type == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_NOT_NULL(res->designators.designators);

        ASSERT_SIZE_T(res->designators.len, (size_t)6);

        struct designator* designators = res->designators.designators;
        ASSERT(designators[0].is_index == true);
        check_cond_expr_id_or_const(&designators[0].arr_index->expr,
                                    "0.5",
                                    F_CONSTANT);

        ASSERT(designators[1].is_index == false);
        check_identifier(designators[1].identifier, "blah");

        ASSERT(designators[2].is_index == true);
        check_cond_expr_id_or_const(&designators[2].arr_index->expr,
                                    "420",
                                    I_CONSTANT);

        ASSERT(designators[3].is_index == false);
        check_identifier(designators[3].identifier, "oof");

        ASSERT(designators[4].is_index == true);
        check_cond_expr_id_or_const(&designators[4].arr_index->expr,
                                    "2",
                                    I_CONSTANT);

        ASSERT(designators[5].is_index == true);
        check_cond_expr_id_or_const(&designators[5].arr_index->expr,
                                    "10",
                                    I_CONSTANT);

        free_designation(res);
        free_parser_state(&s);
        free_tokens(tokens);
    }
}

TEST(static_assert_declaration) {
    struct token* tokens = tokenize_string(
        "_Static_assert(12345, \"This is a string literal\");",
        "slkfjsak");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(tokens, &err);

    struct static_assert_declaration* res = parse_static_assert_declaration(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_TYPE(s.it->type, INVALID);
    ASSERT(err.type == PARSER_ERR_NONE);

    ASSERT_STR(res->err_msg.spelling, "\"This is a string literal\"");
    check_const_expr_id_or_const(res->const_expr, "12345", I_CONSTANT);

    free_tokens(tokens);
    free_parser_state(&s);
    free_static_assert_declaration(res);
}

TEST_SUITE_BEGIN(parser_misc, 4) {
    REGISTER_TEST(enum_list);
    REGISTER_TEST(enum_spec);
    REGISTER_TEST(designation);
    REGISTER_TEST(static_assert_declaration);
}
TEST_SUITE_END()
