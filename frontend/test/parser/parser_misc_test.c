#include "frontend/preproc/preproc.h"

#include "testing/asserts.h"

#include "frontend/parser/parser.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void check_enum_list_ids(const EnumList* l,
                                const TokenArr* tokens,
                                const StrBuf* enum_constants,
                                uint32_t len) {
    ASSERT_UINT(l->len, len);
    for (uint32_t i = 0; i < len; ++i) {
        const Identifier* id = l->enums[i].identifier;
        ASSERT_NOT_NULL(id);
        const Str spell = StrBuf_as_str(&tokens->vals[id->info.token_idx].spelling);
        ASSERT_STR(spell,
                   StrBuf_as_str(&enum_constants[i]));
    }
}

TEST(enum_list) {
    enum {
        EXPECTED_LEN = 8
    };
    const StrBuf enum_constants[EXPECTED_LEN] = {
        STR_BUF_NON_HEAP("ENUM_VAL1"),
        STR_BUF_NON_HEAP("enum_VAl2"),
        STR_BUF_NON_HEAP("enum_val_3"),
        STR_BUF_NON_HEAP("enum_val_4"),
        STR_BUF_NON_HEAP("foo"),
        STR_BUF_NON_HEAP("bar"),
        STR_BUF_NON_HEAP("baz"),
        STR_BUF_NON_HEAP("BAD"),
    };

    {
        PreprocRes preproc_res = tokenize_string(
            STR_LIT(
                "enum {ENUM_VAL1, enum_VAl2, enum_val_3, enum_val_4, foo, bar, "
                "baz, BAD}"),
            STR_LIT("saffds"));

        ParserErr err = ParserErr_create();
        ParserState s = ParserState_create(&preproc_res.toks, &err);

        EnumSpec* e_spec = parse_enum_spec(&s);
        ASSERT_NULL(e_spec->identifier);
        ASSERT_NOT_NULL(e_spec);
        const EnumList* res = &e_spec->enum_list;
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);
        ASSERT_UINT(res->len, (uint32_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res->enums);

        for (uint32_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res->enums[i].identifier);
            ASSERT_NULL(res->enums[i].enum_val);
        }
        check_enum_list_ids(res, &s._arr, enum_constants, ARR_LEN(enum_constants));

        for (uint32_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(ParserState_is_enum_constant(&s, StrBuf_as_str(&enum_constants[i])));
        }

        EnumSpec_free(e_spec);
        ParserState_free(&s);
        TokenArr_free(&s._arr);
        PreprocRes_free(&preproc_res);
    }

    {
        PreprocRes preproc_res = tokenize_string(
            STR_LIT("enum {ENUM_VAL1 = 0, enum_VAl2 = 1000.0, enum_val_3 = n, "
                    "enum_val_4 = "
                    "test, foo, bar, baz, BAD}"),
            STR_LIT("saffds"));

        ParserErr err = ParserErr_create();
        ParserState s = ParserState_create(&preproc_res.toks, &err);

        EnumSpec* e_spec = parse_enum_spec(&s);
        ASSERT_NOT_NULL(e_spec);
        ASSERT_NULL(e_spec->identifier);
        const EnumList* res = &e_spec->enum_list;
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);
        ASSERT_UINT(res->len, (uint32_t)EXPECTED_LEN);
        ASSERT_NOT_NULL(res->enums);

        for (uint32_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT_NOT_NULL(res->enums[i].identifier);
        }

        ASSERT_NOT_NULL(res->enums[0].enum_val);
        check_const_expr_val(res->enums[0].enum_val,
                             Value_create_sint(VALUE_INT, 0), &s._arr);

        ASSERT_NOT_NULL(res->enums[1].enum_val);
        check_const_expr_val(res->enums[1].enum_val,
                             Value_create_float(VALUE_DOUBLE, 1000.0), &s._arr);

        ASSERT_NOT_NULL(res->enums[2].enum_val);
        check_const_expr_id(res->enums[2].enum_val, STR_LIT("n"), &s._arr);

        ASSERT_NOT_NULL(res->enums[3].enum_val);
        check_const_expr_id(res->enums[3].enum_val, STR_LIT("test"), &s._arr);

        for (uint32_t i = 4; i < EXPECTED_LEN; ++i) {
            ASSERT_NULL(res->enums[i].enum_val);
        }

        for (uint32_t i = 0; i < EXPECTED_LEN; ++i) {
            ASSERT(ParserState_is_enum_constant(&s, StrBuf_as_str(&enum_constants[i])));
        }

        EnumSpec_free(e_spec);
        ParserState_free(&s);
        TokenArr_free(&s._arr);
        PreprocRes_free(&preproc_res);
    }
}

TEST(enum_spec) {
    const StrBuf enum_constants[] = {
        STR_BUF_NON_HEAP("TEST1"),
        STR_BUF_NON_HEAP("TEST2"),
        STR_BUF_NON_HEAP("TEST3"),
        STR_BUF_NON_HEAP("TEST4"),
    };
    {
        PreprocRes preproc_res = tokenize_string(
            STR_LIT("enum my_enum { TEST1, TEST2, TEST3, TEST4 }"),
            STR_LIT("sfjlfjk"));

        ParserErr err = ParserErr_create();
        ParserState s = ParserState_create(&preproc_res.toks, &err);

        EnumSpec* res = parse_enum_spec(&s);
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);

        ASSERT_NOT_NULL(res->identifier);
        ASSERT_STR(StrBuf_as_str(&s._arr.vals[res->identifier->info.token_idx].spelling),
                   STR_LIT("my_enum"));

        check_enum_list_ids(&res->enum_list,
                            &s._arr,
                            enum_constants,
                            ARR_LEN(enum_constants));

        PreprocRes_free(&preproc_res);
        ParserState_free(&s);
        TokenArr_free(&s._arr);
        EnumSpec_free(res);
    }

    {
        PreprocRes preproc_res = tokenize_string(
            STR_LIT("enum {TEST1, TEST2, TEST3, TEST4, }"),
            STR_LIT("jsfjsf"));

        ParserErr err = ParserErr_create();
        ParserState s = ParserState_create(&preproc_res.toks, &err);

        EnumSpec* res = parse_enum_spec(&s);
        ASSERT(err.kind == PARSER_ERR_NONE);
        ASSERT_NOT_NULL(res);
        ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);

        ASSERT_NULL(res->identifier);

        check_enum_list_ids(&res->enum_list,
                            &s._arr,
                            enum_constants,
                            ARR_LEN(enum_constants));

        PreprocRes_free(&preproc_res);
        ParserState_free(&s);
        TokenArr_free(&s._arr);
        EnumSpec_free(res);
    }
}

TEST(designation) {
    PreprocRes preproc_res = tokenize_string(
        STR_LIT("{ .test[19].what_is_this.another_one = a, "
                "[0.5].blah[420].oof[2][10] "
                "= 2 }"),
        STR_LIT("kdsjflkf"));
    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);
    Initializer* init = parse_initializer(&s);
    ASSERT_NOT_NULL(init);
    ASSERT(!init->is_assign);

    const InitList* list = &init->init_list;
    ASSERT_UINT(list->len, 2);
    const Designation* des1 = &list->inits[0].designation;
    const Initializer* val1 = &list->inits[0].init;
    ASSERT(val1->is_assign);
    check_assign_expr_id(val1->assign, STR_LIT("a"), &s._arr);

    ASSERT_UINT(des1->designators.len, (uint32_t)4);

    struct Designator* designators = des1->designators.designators;
    ASSERT(designators[0].is_index == false);
    check_identifier(designators[0].identifier, STR_LIT("test"), &s._arr);

    ASSERT(designators[1].is_index == true);
    check_cond_expr_val(&designators[1].arr_index->expr,
                        Value_create_sint(VALUE_INT, 19), &s._arr);

    ASSERT(designators[2].is_index == false);
    check_identifier(designators[2].identifier, STR_LIT("what_is_this"), &s._arr);

    ASSERT(designators[3].is_index == false);
    check_identifier(designators[3].identifier, STR_LIT("another_one"), &s._arr);

    const Designation* des2 = &list->inits[1].designation;
    const Initializer* val2 = &list->inits[1].init;
    ASSERT(val2->is_assign);
    check_assign_expr_val(val2->assign, Value_create_sint(VALUE_INT, 2), &s._arr);

    ASSERT_UINT(des2->designators.len, (uint32_t)6);

    designators = des2->designators.designators;
    ASSERT(designators[0].is_index == true);
    check_cond_expr_val(&designators[0].arr_index->expr,
                        Value_create_float(VALUE_DOUBLE, 0.5), &s._arr);

    ASSERT(designators[1].is_index == false);
    check_identifier(designators[1].identifier, STR_LIT("blah"), &s._arr);

    ASSERT(designators[2].is_index == true);
    check_cond_expr_val(&designators[2].arr_index->expr,
                        Value_create_sint(VALUE_INT, 420), &s._arr);

    ASSERT(designators[3].is_index == false);
    check_identifier(designators[3].identifier, STR_LIT("oof"), &s._arr);

    ASSERT(designators[4].is_index == true);
    check_cond_expr_val(&designators[4].arr_index->expr,
                        Value_create_sint(VALUE_INT, 2), &s._arr);

    ASSERT(designators[5].is_index == true);
    check_cond_expr_val(&designators[5].arr_index->expr,
                        Value_create_sint(VALUE_INT, 10), &s._arr);

    Initializer_free(init);

    ParserState_free(&s);
    TokenArr_free(&s._arr);
    PreprocRes_free(&preproc_res);
}

TEST(static_assert_declaration) {
    PreprocRes preproc_res = tokenize_string(
        STR_LIT("_Static_assert(12345, \"This is a string literal\");"),
        STR_LIT("slkfjsak"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    StaticAssertDeclaration* res = parse_static_assert_declaration(&s);
    ASSERT_NOT_NULL(res);
    ASSERT_TOKEN_KIND(ParserState_curr_kind(&s), TOKEN_INVALID);
    ASSERT(err.kind == PARSER_ERR_NONE);
    
    const Str contents = StrBuf_as_str(&s._arr.vals[res->err_msg.info.token_idx].str_lit.contents);
    ASSERT_STR(contents,
               STR_LIT("This is a string literal"));
    check_const_expr_val(res->const_expr, Value_create_sint(VALUE_INT, 12345), &s._arr);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
    StaticAssertDeclaration_free(res);
}

static void check_struct_declaration_non_static_assert(
    const StructDeclaration* decl,
    const TokenArr* toks,
    TypeSpecKind type,
    Str identifier,
    int bit_field,
    const TokenArr* arr) {
    ASSERT(decl->decl_specs.type_specs.kind == type);
    if (!Str_valid(identifier) && bit_field < 0) {
        ASSERT_UINT(decl->decls.len, (uint32_t)0);
        return;
    } else {
        ASSERT_UINT(decl->decls.len, (uint32_t)1);
    }
    const StructDeclarator* declarator = &decl->decls.decls[0];
    if (bit_field > 0) {
        check_const_expr_val(declarator->bit_field,
                             Value_create_sint(VALUE_INT, bit_field), arr);
    }
    if (Str_valid(identifier)) {
        const Declarator* inner_decl = declarator->decl;
        ASSERT_NOT_NULL(inner_decl);
        ASSERT_NOT_NULL(inner_decl->direct_decl);
        ASSERT_NULL(inner_decl->ptr);
        ASSERT_UINT(inner_decl->direct_decl->len, (uint32_t)0);
        ASSERT(inner_decl->direct_decl->is_id);
        ASSERT_NOT_NULL(inner_decl->direct_decl->id);
        const Str spell = StrBuf_as_str(&toks->vals[inner_decl->direct_decl->id->info.token_idx].spelling);
        ASSERT_STR(spell,
                   identifier);
    } else {
        ASSERT_NULL(declarator->decl);
    }
}

TEST(struct_declaration_list) {
    PreprocRes preproc_res = tokenize_string(
        STR_LIT("struct { int n: 20; int: 10; double a_double; int; }"),
        STR_LIT("maybe_a_file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    StructUnionSpec* su_spec = parse_struct_union_spec(&s);
    ASSERT(su_spec->is_struct);
    ASSERT_NULL(su_spec->identifier);
    ASSERT(ParserState_curr_kind(&s) == TOKEN_INVALID);
    StructDeclarationList* res = &su_spec->decl_list;
    ASSERT_UINT(res->len, (uint32_t)4);

    check_struct_declaration_non_static_assert(&res->decls[0],
                                               &s._arr,
                                               TYPE_SPEC_INT,
                                               STR_LIT("n"),
                                               20,
                                               &s._arr);
    check_struct_declaration_non_static_assert(&res->decls[1],
                                               &s._arr,
                                               TYPE_SPEC_INT,
                                               Str_null(),
                                               10,
                                               &s._arr);
    check_struct_declaration_non_static_assert(&res->decls[2],
                                               &s._arr,
                                               TYPE_SPEC_DOUBLE,
                                               STR_LIT("a_double"),
                                               -1,
                                               &s._arr);
    check_struct_declaration_non_static_assert(&res->decls[3],
                                               &s._arr,
                                               TYPE_SPEC_INT,
                                               Str_null(),
                                               -1,
                                               &s._arr);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
    StructUnionSpec_free(su_spec);
}

TEST(redefine_typedef) {
    PreprocRes preproc_res = tokenize_string(STR_LIT("typedef int MyInt;"),
                                             STR_LIT("a file"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    const StrBuf spell = STR_BUF_NON_HEAP("MyInt");
    ParserState_register_typedef(&s, &spell, (uint32_t)-1);

    bool found_typedef = false;
    DeclarationSpecs res; 
    ASSERT(!parse_declaration_specs(&s, &res, &found_typedef));
    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT(err.was_typedef_name);
    const Str got_spell = StrBuf_as_str(&s._arr.vals[err.err_token_idx].spelling);
    ASSERT_STR(got_spell, STR_LIT("MyInt"));

    ParserState_free(&s);
    TokenArr_free(&s._arr);
    PreprocRes_free(&preproc_res);
}

typedef struct {
    ParserErr err;
    TokenArr toks;
} ParserErrAndToks;

static ParserErrAndToks parse_type_specs_until_fail(Str code) {
    PreprocRes preproc_res = tokenize_string(code, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    TypeSpecs specs = TypeSpecs_create();

    while (update_type_specs(&s, &specs))
        ;

    ParserState_free(&s);
    PreprocRes_free(&preproc_res);
    TypeSpecs_free_children(&specs);
    return (ParserErrAndToks){err, s._arr};
}

static void check_too_many_long(Str code) {
    ParserErrAndToks res = parse_type_specs_until_fail(code);

    ASSERT(res.err.kind == PARSER_ERR_TOO_MUCH_LONG);
    TokenArr_free(&res.toks);
}

static void check_cannot_combine_type_specs(Str code,
                                            TokenKind prev_spec,
                                            TokenKind type_spec,
                                            SourceLoc ex_loc) {
    ParserErrAndToks res = parse_type_specs_until_fail(code);

    ASSERT(res.err.kind == PARSER_ERR_INCOMPATIBLE_TYPE_SPECS);
    ASSERT_TOKEN_KIND(res.err.prev_type_spec, prev_spec);
    ASSERT_TOKEN_KIND(res.err.type_spec, type_spec);
    const SourceLoc loc = res.toks.locs[res.err.err_token_idx];
    ASSERT_UINT(loc.file_idx, ex_loc.file_idx);
    ASSERT_UINT(loc.file_loc.line, ex_loc.file_loc.line);
    ASSERT_UINT(loc.file_loc.index, ex_loc.file_loc.index);

    TokenArr_free(&res.toks);
}

TEST(type_spec_error) {
    check_too_many_long(STR_LIT("long long long"));
    check_too_many_long(STR_LIT("int long long long"));
    check_too_many_long(STR_LIT("long long int long"));
    check_cannot_combine_type_specs(STR_LIT("long short"),
                                    TOKEN_LONG,
                                    TOKEN_SHORT,
                                    (SourceLoc){0, {1, 6}});
    check_cannot_combine_type_specs(STR_LIT("long short long"),
                                    TOKEN_LONG,
                                    TOKEN_SHORT,
                                    (SourceLoc){0, {1, 6}});
    check_cannot_combine_type_specs(STR_LIT("short long"),
                                    TOKEN_SHORT,
                                    TOKEN_LONG,
                                    (SourceLoc){0, {1, 7}});
    check_cannot_combine_type_specs(STR_LIT("unsigned signed"),
                                    TOKEN_UNSIGNED,
                                    TOKEN_SIGNED,
                                    (SourceLoc){0, {1, 10}});
    check_cannot_combine_type_specs(STR_LIT("signed unsigned"),
                                    TOKEN_SIGNED,
                                    TOKEN_UNSIGNED,
                                    (SourceLoc){0, {1, 8}});
    // TODO: DISALLOWED_TYPE_QUALS
}

static void check_expected_semicolon_jump_statement(Str spell) {
    PreprocRes preproc_res = tokenize_string(spell, STR_LIT("file.c"));

    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&preproc_res.toks, &err);

    Statement* res = parse_statement(&s);
    ASSERT_NULL(res);

    ASSERT(err.kind == PARSER_ERR_EXPECTED_TOKENS);
    const ExpectedTokensErr* ex_tokens_err = &err.expected_tokens_err;
    ASSERT_UINT(ex_tokens_err->num_expected, (uint32_t)1);
    ASSERT_TOKEN_KIND(ex_tokens_err->expected[0], TOKEN_SEMICOLON);
    ASSERT_TOKEN_KIND(ex_tokens_err->got, TOKEN_INVALID);

    PreprocRes_free(&preproc_res);
    ParserState_free(&s);
    TokenArr_free(&s._arr);
}

TEST(jump_statement_error) {
    check_expected_semicolon_jump_statement(STR_LIT("continue"));
    check_expected_semicolon_jump_statement(STR_LIT("break"));
    check_expected_semicolon_jump_statement(STR_LIT("return an_identifier"));
    check_expected_semicolon_jump_statement(STR_LIT("return *id += (int)100"));
}

TEST_SUITE_BEGIN(parser_misc){
    REGISTER_TEST(enum_list),
    REGISTER_TEST(enum_spec),
    REGISTER_TEST(designation),
    REGISTER_TEST(static_assert_declaration),
    REGISTER_TEST(struct_declaration_list),
    REGISTER_TEST(redefine_typedef),
    REGISTER_TEST(type_spec_error),
    REGISTER_TEST(jump_statement_error),
} TEST_SUITE_END()
