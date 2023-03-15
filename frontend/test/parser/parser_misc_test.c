#include "frontend/preproc/preproc.h"

#include "testing/asserts.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void check_enum_list_ids(const struct enum_list* l,
                                const struct str* enum_constants,
                                size_t len) {
    ASSERT_SIZE_T(l->len, len);
    for (size_t i = 0; i < len; ++i) {
        const struct identifier* id = l->enums[i].identifier;
        ASSERT_NOT_NULL(id);
        ASSERT_STR(str_get_data(&id->spelling),
                   str_get_data(&enum_constants[i]));
    }
}

TEST(enum_list) {
    enum {
        EXPECTED_LEN = 8
    };
    const struct str enum_constants[EXPECTED_LEN] = {
        STR_NON_HEAP("ENUM_VAL1"),
        STR_NON_HEAP("enum_VAl2"),
        STR_NON_HEAP("enum_val_3"),
        STR_NON_HEAP("enum_val_4"),
        STR_NON_HEAP("foo"),
        STR_NON_HEAP("bar"),
        STR_NON_HEAP("baz"),
        STR_NON_HEAP("BAD"),
    };

    {
        struct preproc_res preproc_res = tokenize_string(
            "enum {ENUM_VAL1, enum_VAl2, enum_val_3, enum_val_4, foo, bar, "
            "baz, BAD}",
            "saffds");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct enum_spec* e_spec = parse_enum_spec(&s);
        ASSERT_NULL(e_spec->identifier);
        ASSERT_NOT_NULL(e_spec);
        const struct enum_list* res = &e_spec->enum_list;
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_TOKEN_KIND(s.it->kind, INVALID);
        ASSERT_SIZE_T(res->len, (size_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res->enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res->enums[i].identifier);
            ASSERT_NULL(res->enums[i].enum_val);
        }
        check_enum_list_ids(res, enum_constants, ARR_LEN(enum_constants));

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(is_enum_constant(&s, &enum_constants[i]));
        }

        free_enum_spec(e_spec);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }

    {
        struct preproc_res preproc_res = tokenize_string(
            "enum {ENUM_VAL1 = 0, enum_VAl2 = 1000.0, enum_val_3 = n, "
            "enum_val_4 = "
            "test, foo, bar, baz, BAD}",
            "saffds");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct enum_spec* e_spec = parse_enum_spec(&s);
        ASSERT_NOT_NULL(e_spec);
        ASSERT_NULL(e_spec->identifier);
        const struct enum_list* res = &e_spec->enum_list;
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_TOKEN_KIND(s.it->kind, INVALID);
        ASSERT_SIZE_T(res->len, (size_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res->enums);

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res->enums[i].identifier);
        }

        ASSERT_NOT_NULL(res->enums[0].enum_val);
        check_const_expr_int(res->enums[0].enum_val,
                             create_int_value(INT_VALUE_I, 0));

        ASSERT_NOT_NULL(res->enums[1].enum_val);
        check_const_expr_float(res->enums[1].enum_val,
                               create_float_value(FLOAT_VALUE_D, 1000.0));

        ASSERT_NOT_NULL(res->enums[2].enum_val);
        check_const_expr_id(res->enums[2].enum_val, "n");

        ASSERT_NOT_NULL(res->enums[3].enum_val);
        check_const_expr_id(res->enums[3].enum_val, "test");

        for (size_t i = 4; i < EXPECTED_LEN; ++i) {
            ASSERT_NULL(res->enums[i].enum_val);
        }

        for (size_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(is_enum_constant(&s, &enum_constants[i]));
        }

        free_enum_spec(e_spec);
        free_parser_state(&s);
        free_preproc_res(&preproc_res);
    }
}

TEST(enum_spec) {
    const struct str enum_constants[] = {
        STR_NON_HEAP("TEST1"),
        STR_NON_HEAP("TEST2"),
        STR_NON_HEAP("TEST3"),
        STR_NON_HEAP("TEST4"),
    };
    {
        struct preproc_res preproc_res = tokenize_string(
            "enum my_enum { TEST1, TEST2, TEST3, TEST4 }",
            "sfjlfjk");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_KIND(s.it->kind, INVALID);

        ASSERT_NOT_NULL(res->identifier);
        ASSERT_STR(str_get_data(&res->identifier->spelling), "my_enum");

        check_enum_list_ids(&res->enum_list,
                            enum_constants,
                            ARR_LEN(enum_constants));

        free_preproc_res(&preproc_res);
        free_parser_state(&s);
        free_enum_spec(res);
    }

    {
        struct preproc_res preproc_res = tokenize_string(
            "enum {TEST1, TEST2, TEST3, TEST4, }",
            "jsfjsf");

        struct parser_err err = create_parser_err();
        struct parser_state s = create_parser_state(preproc_res.toks, &err);

        struct enum_spec* res = parse_enum_spec(&s);
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_KIND(s.it->kind, INVALID);

        ASSERT_NULL(res->identifier);

        check_enum_list_ids(&res->enum_list,
                            enum_constants,
                            ARR_LEN(enum_constants));

        free_preproc_res(&preproc_res);
        free_parser_state(&s);
        free_enum_spec(res);
    }
}

TEST(designation) {
    struct preproc_res preproc_res = tokenize_string(
        "{ .test[19].what_is_this.another_one = a, [0.5].blah[420].oof[2][10] "
        "= 2 }",
        "kdsjflkf");
    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);
    struct initializer* init = parse_initializer(&s);
    ASSERT_NOT_NULL(init);
    ASSERT(!init->is_assign);

    const struct init_list* list = &init->init_list;
    ASSERT_SIZE_T(list->len, 2);
    const struct designation* des1 = &list->inits[0].designation;
    const struct initializer* val1 = &list->inits[0].init;
    ASSERT(val1->is_assign);
    check_assign_expr_id(val1->assign, "a");

    ASSERT_SIZE_T(des1->designators.len, (size_t)4);

    struct designator* designators = des1->designators.designators;
    ASSERT(designators[0].is_index == false);
    check_identifier(designators[0].identifier, "test");

    ASSERT(designators[1].is_index == true);
    check_cond_expr_int(&designators[1].arr_index->expr,
                        create_int_value(INT_VALUE_I, 19));

    ASSERT(designators[2].is_index == false);
    check_identifier(designators[2].identifier, "what_is_this");

    ASSERT(designators[3].is_index == false);
    check_identifier(designators[3].identifier, "another_one");

    const struct designation* des2 = &list->inits[1].designation;
    const struct initializer* val2 = &list->inits[1].init;
    ASSERT(val2->is_assign);
    check_assign_expr_int(val2->assign, create_int_value(INT_VALUE_I, 2));

    ASSERT_SIZE_T(des2->designators.len, (size_t)6);

    designators = des2->designators.designators;
    ASSERT(designators[0].is_index == true);
    check_cond_expr_float(&designators[0].arr_index->expr,
                          create_float_value(FLOAT_VALUE_D, 0.5));

    ASSERT(designators[1].is_index == false);
    check_identifier(designators[1].identifier, "blah");

    ASSERT(designators[2].is_index == true);
    check_cond_expr_int(&designators[2].arr_index->expr,
                        create_int_value(INT_VALUE_I, 420));

    ASSERT(designators[3].is_index == false);
    check_identifier(designators[3].identifier, "oof");

    ASSERT(designators[4].is_index == true);
    check_cond_expr_int(&designators[4].arr_index->expr,
                        create_int_value(INT_VALUE_I, 2));

    ASSERT(designators[5].is_index == true);
    check_cond_expr_int(&designators[5].arr_index->expr,
                        create_int_value(INT_VALUE_I, 10));

    free_initializer(init);

    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

TEST(static_assert_declaration) {
    struct preproc_res preproc_res = tokenize_string(
        "_Static_assert(12345, \"This is a string literal\");",
        "slkfjsak");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct static_assert_declaration* res = parse_static_assert_declaration(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_KIND(s.it->kind, INVALID);
    ASSERT(err.kind == PARSER_ERR_NONE);

    ASSERT_STR(str_get_data(&res->err_msg.lit.contents),
               "This is a string literal");
    check_const_expr_int(res->const_expr, create_int_value(INT_VALUE_I, 12345));

    free_preproc_res(&preproc_res);
    free_parser_state(&s);
    free_static_assert_declaration(res);
}

static void check_struct_declaration_non_static_assert(
    const struct struct_declaration* decl,
    enum type_spec_kind type,
    const char* identifier,
    int bit_field) {
    ASSERT(decl->decl_specs->type_specs.kind == type);
    if (identifier == NULL && bit_field < 0) {
        ASSERT_SIZE_T(decl->decls.len, (size_t)0);
        return;
    } else {
        ASSERT_SIZE_T(decl->decls.len, (size_t)1);
    }
    const struct struct_declarator* declarator = &decl->decls.decls[0];
    if (bit_field > 0) {
        check_const_expr_int(declarator->bit_field,
                             create_int_value(INT_VALUE_I, bit_field));
    }
    if (identifier) {
        const struct declarator* inner_decl = declarator->decl;
        ASSERT_NOT_NULL(inner_decl);
        ASSERT_NOT_NULL(inner_decl->direct_decl);
        ASSERT_NULL(inner_decl->ptr);
        ASSERT_SIZE_T(inner_decl->direct_decl->len, (size_t)0);
        ASSERT(inner_decl->direct_decl->is_id);
        ASSERT_NOT_NULL(inner_decl->direct_decl->id);
        ASSERT_STR(str_get_data(&inner_decl->direct_decl->id->spelling),
                   identifier);
    } else {
        ASSERT_NULL(declarator->decl);
    }
}

TEST(struct_declaration_list) {
    struct preproc_res preproc_res = tokenize_string(
        "struct { int n: 20; int: 10; double a_double; int; }",
        "maybe_a_file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct struct_union_spec* su_spec = parse_struct_union_spec(&s);
    ASSERT(su_spec->is_struct);
    ASSERT_NULL(su_spec->identifier);
    ASSERT(s.it->kind == INVALID);
    struct struct_declaration_list* res = &su_spec->decl_list;
    ASSERT_SIZE_T(res->len, (size_t)4);

    check_struct_declaration_non_static_assert(&res->decls[0],
                                               TYPE_SPEC_INT,
                                               "n",
                                               20);
    check_struct_declaration_non_static_assert(&res->decls[1],
                                               TYPE_SPEC_INT,
                                               NULL,
                                               10);
    check_struct_declaration_non_static_assert(&res->decls[2],
                                               TYPE_SPEC_DOUBLE,
                                               "a_double",
                                               -1);
    check_struct_declaration_non_static_assert(&res->decls[3],
                                               TYPE_SPEC_INT,
                                               NULL,
                                               -1);

    free_preproc_res(&preproc_res);
    free_parser_state(&s);
    free_struct_union_spec(su_spec);
}

TEST(redefine_typedef) {
    struct preproc_res preproc_res = tokenize_string("typedef int MyInt;",
                                                     "a file");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    const struct str spell = STR_NON_HEAP("MyInt");
    const struct token dummy_token = create_token(IDENTIFIER,
                                                  &spell,
                                                  (struct file_loc){0, 0},
                                                  0);
    register_typedef_name(&s, &dummy_token);

    bool found_typedef = false;
    struct declaration_specs* res = parse_declaration_specs(&s, &found_typedef);
    ASSERT_NULL(res);
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    ASSERT_STR(str_get_data(&err.redefined_symbol), "MyInt");

    free_parser_state(&s);
    free_preproc_res(&preproc_res);
}

static struct parser_err parse_type_specs_until_fail(const char* code) {
    struct preproc_res preproc_res = tokenize_string(code, "file.c");

    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(preproc_res.toks, &err);

    struct type_specs specs = create_type_specs();

    while (update_type_specs(&s, &specs))
        ;

    free_parser_state(&s);
    free_preproc_res(&preproc_res);
    free_type_specs_children(&specs);
    return err;
}

static void check_too_many_long(const char* code) {
    struct parser_err err = parse_type_specs_until_fail(code);

    ASSERT(err.kind == PARSER_ERR_TOO_MUCH_LONG);
}

static void check_cannot_combine_type_specs(const char* code,
                                            enum token_kind prev_spec,
                                            enum token_kind type_spec,
                                            struct source_loc loc) {

    struct parser_err err = parse_type_specs_until_fail(code);

    ASSERT(err.kind == PARSER_ERR_INCOMPATIBLE_TYPE_SPECS);
    ASSERT_TOKEN_KIND(err.prev_type_spec, prev_spec);
    ASSERT_TOKEN_KIND(err.type_spec, type_spec);
    ASSERT_SIZE_T(loc.file_idx, err.base.loc.file_idx);
    ASSERT_SIZE_T(loc.file_loc.line, err.base.loc.file_loc.line);
    ASSERT_SIZE_T(loc.file_loc.index, err.base.loc.file_loc.index);
}

TEST(type_spec_error) {
    check_too_many_long("long long long");
    check_too_many_long("int long long long");
    check_too_many_long("long long int long");
    check_cannot_combine_type_specs("long short",
                                    LONG,
                                    SHORT,
                                    (struct source_loc){0, {1, 6}});
    check_cannot_combine_type_specs("long short long",
                                    LONG,
                                    SHORT,
                                    (struct source_loc){0, {1, 6}});
    check_cannot_combine_type_specs("short long",
                                    SHORT,
                                    LONG,
                                    (struct source_loc){0, {1, 7}});
    check_cannot_combine_type_specs("unsigned signed",
                                    UNSIGNED,
                                    SIGNED,
                                    (struct source_loc){0, {1, 10}});
    check_cannot_combine_type_specs("signed unsigned",
                                    SIGNED,
                                    UNSIGNED,
                                    (struct source_loc){0, {1, 8}});
    // TODO: DISALLOWED_TYPE_QUALS
}

TEST_SUITE_BEGIN(parser_misc, 7) {
    REGISTER_TEST(enum_list);
    REGISTER_TEST(enum_spec);
    REGISTER_TEST(designation);
    REGISTER_TEST(static_assert_declaration);
    REGISTER_TEST(struct_declaration_list);
    REGISTER_TEST(redefine_typedef);
    REGISTER_TEST(type_spec_error);
}
TEST_SUITE_END()
