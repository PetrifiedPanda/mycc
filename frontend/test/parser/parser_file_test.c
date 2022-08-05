#include <stdio.h>

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast_dumper.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void check_func_specs(const struct func_specs* got,
                             const struct func_specs* expected) {
    ASSERT_BOOL(got->is_inline, expected->is_inline);
    ASSERT_BOOL(got->is_noreturn, expected->is_noreturn);
}

static void check_storage_class(const struct storage_class* got,
                                const struct storage_class* expected) {
    ASSERT_BOOL(got->is_typedef, expected->is_typedef);
    ASSERT_BOOL(got->is_extern, expected->is_extern);
    ASSERT_BOOL(got->is_static, expected->is_static);
    ASSERT_BOOL(got->is_thread_local, expected->is_thread_local);
    ASSERT_BOOL(got->is_auto, expected->is_auto);
    ASSERT_BOOL(got->is_register, expected->is_register);
}

static void check_external_decl_struct(struct external_declaration* d,
                                       bool is_typedef,
                                       bool is_struct,
                                       const char* id_spell,
                                       size_t decl_list_len) {
    ASSERT(d->is_func_def == false);
    ASSERT(d->decl.is_normal_decl);
    ASSERT(d->decl.decl_specs->type_specs.type == TYPESPEC_STRUCT);
    const struct func_specs none = {false, false};
    check_func_specs(&d->decl.decl_specs->func_specs, &none);

    struct storage_class sc = {.is_typedef = is_typedef};
    check_storage_class(&d->decl.decl_specs->storage_class, &sc);

    ASSERT_BOOL(d->decl.decl_specs->type_specs.struct_union_spec->is_struct,
                is_struct);
    check_identifier(
        d->decl.decl_specs->type_specs.struct_union_spec->identifier,
        id_spell);

    ASSERT_SIZE_T(
        d->decl.decl_specs->type_specs.struct_union_spec->decl_list.len,
        decl_list_len);
}

static void check_external_decl_enum(struct external_declaration* d,
                                     bool is_typedef,
                                     const char* id_spell,
                                     size_t enum_list_len) {
    ASSERT(d->is_func_def == false);
    ASSERT(d->decl.is_normal_decl);
    ASSERT(d->decl.decl_specs->type_specs.type == TYPESPEC_ENUM);
    const struct func_specs none = {false, false};
    check_func_specs(&d->decl.decl_specs->func_specs, &none);
    struct storage_class sc = {.is_typedef = is_typedef};
    check_storage_class(&d->decl.decl_specs->storage_class, &sc);
    check_identifier(d->decl.decl_specs->type_specs.enum_spec->identifier,
                     id_spell);

    ASSERT_SIZE_T(d->decl.decl_specs->type_specs.enum_spec->enum_list.len,
                  enum_list_len);
}

static void check_pointer_indirs(struct pointer* ptr, size_t num_indirs) {
    if (num_indirs == 0) {
        ASSERT_NULL(ptr);
    } else {
        ASSERT_NOT_NULL(ptr);
        ASSERT_SIZE_T(ptr->num_indirs, num_indirs);
    }
}

static void check_external_decl_func_def(
    struct external_declaration* d,
    const struct storage_class* storage_class,
    const struct func_specs* func_specs,
    size_t num_indirs,
    const char* id_spell,
    size_t body_len) {
    ASSERT(d->is_func_def);
    ASSERT(d->func_def.decl->direct_decl->is_id);
    check_identifier(d->func_def.decl->direct_decl->id, id_spell);
    check_pointer_indirs(d->func_def.decl->ptr, num_indirs);

    check_storage_class(&d->func_def.specs->storage_class, storage_class);
    check_func_specs(&d->func_def.specs->func_specs, func_specs);
    ASSERT_SIZE_T(d->func_def.comp->len, body_len);
}

static void check_external_decl_func_def_enum(
    struct external_declaration* d,
    struct storage_class storage_class,
    struct func_specs func_specs,
    const char* enum_name,
    size_t num_indirs,
    const char* func_name,
    size_t body_len) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len);

    ASSERT(d->func_def.specs->type_specs.has_specifier);
    ASSERT(d->func_def.specs->type_specs.type == TYPESPEC_ENUM);
    check_identifier(d->func_def.specs->type_specs.enum_spec->identifier,
                     enum_name);
    ASSERT_SIZE_T(d->func_def.specs->type_specs.enum_spec->enum_list.len,
                  (size_t)0);
}

static void check_external_decl_func_def_struct(
    struct external_declaration* d,
    struct storage_class storage_class,
    struct func_specs func_specs,
    const char* struct_name,
    size_t num_indirs,
    const char* func_name,
    size_t body_len) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len);

    ASSERT(d->func_def.specs->type_specs.has_specifier);
    ASSERT(d->func_def.specs->type_specs.type == TYPESPEC_STRUCT);
    ASSERT(d->func_def.specs->type_specs.struct_union_spec->is_struct);
    check_identifier(
        d->func_def.specs->type_specs.struct_union_spec->identifier,
        struct_name);
    ASSERT_SIZE_T(
        d->func_def.specs->type_specs.struct_union_spec->decl_list.len,
        (size_t)0);
}

static void check_external_decl_func_def_predef(
    struct external_declaration* d,
    struct storage_class storage_class,
    struct func_specs func_specs,
    size_t num_indirs,
    enum token_type ret_type,
    const char* id_spell,
    size_t body_len) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 id_spell,
                                 body_len);

    ASSERT(d->func_def.specs->type_specs.has_specifier);
    ASSERT(d->func_def.specs->type_specs.type == TYPESPEC_PREDEF);
    ASSERT_TOKEN_TYPE(d->func_def.specs->type_specs.type_spec, ret_type);
}

static void check_external_decl_func_def_typedef(
    struct external_declaration* d,
    struct storage_class storage_class,
    struct func_specs func_specs,
    size_t num_indirs,
    const char* ret_type,
    const char* func_name,
    size_t body_len) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len);

    ASSERT(d->func_def.specs->type_specs.has_specifier);
    ASSERT(d->func_def.specs->type_specs.type == TYPESPEC_TYPENAME);
    ASSERT_STR(d->func_def.specs->type_specs.typedef_name->spelling, ret_type);
}

static void compare_with_ex_file(const struct translation_unit* tl, const char* ex_file) {
    const char* tmp_filename = "tmp.ast";

    FILE* tmp_file = fopen(tmp_filename, "w");
    ASSERT(dump_ast(tl, tmp_file));

    ASSERT_INT(fclose(tmp_file), 0);

    test_compare_files(tmp_filename, ex_file);

    remove(tmp_filename);
}

TEST(no_preproc) {
    const char* file = "../frontend/test/files/no_preproc.c";
    struct token* tokens = tokenize(file);
    
    struct parser_err err = create_parser_err();
    struct translation_unit tl = parse_tokens(tokens, &err);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_SIZE_T(tl.len, (size_t)10);

    compare_with_ex_file(&tl, "../frontend/test/files/no_preproc.c.ast");

    const struct storage_class sc = {false, false, false, false, false, false};
    const struct storage_class sc_static = {.is_static = true};
    const struct func_specs fs = (struct func_specs){false, false};

    check_external_decl_struct(&tl.external_decls[0], true, true, NULL, 2);
    check_external_decl_struct(&tl.external_decls[1],
                               false,
                               false,
                               "my_union",
                               2);
    check_external_decl_enum(&tl.external_decls[2], false, "my_enum", 3);
    check_external_decl_func_def_predef(&tl.external_decls[6],
                                        sc,
                                        fs,
                                        0,
                                        INT,
                                        "main",
                                        15);
    check_external_decl_func_def_predef(&tl.external_decls[7],
                                        sc_static,
                                        fs,
                                        0,
                                        INT,
                                        "do_shit",
                                        13);
    check_external_decl_func_def_predef(
        &tl.external_decls[8],
        sc_static,
        (struct func_specs){.is_noreturn = true},
        0,
        VOID,
        "variadic",
        8);

    free_translation_unit(&tl);
    free_tokens(tokens);
}

TEST(parser_testfile) {
    const char* file = "../frontend/test/files/parser_testfile.c";
    struct token* tokens = tokenize(file);
    
    struct parser_err err = create_parser_err();
    struct translation_unit tl = parse_tokens(tokens, &err);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_SIZE_T(tl.len, (size_t)15);
    
    compare_with_ex_file(&tl, "../frontend/test/files/parser_testfile.c.ast");

    const char* tmp_filename = "tmp.ast";
    FILE* tmp_file = fopen(tmp_filename, "w");

    dump_ast(&tl, tmp_file);

    fclose(tmp_file);

    test_compare_files(tmp_filename, "../frontend/test/files/parser_testfile.c.ast");

    const struct storage_class sc = {false, false, false, false, false, false};
    const struct storage_class sc_static = {.is_static = true};
    const struct func_specs fs = {false, false};

    check_external_decl_struct(&tl.external_decls[0], true, true, NULL, 2);

    ASSERT(tl.external_decls[1].is_func_def == false);
    ASSERT(tl.external_decls[1].decl.is_normal_decl);
    ASSERT(tl.external_decls[1].decl.decl_specs->type_specs.type
           == TYPESPEC_PREDEF);
    ASSERT_SIZE_T(tl.external_decls[1].decl.init_decls.len, (size_t)1);

    check_external_decl_struct(&tl.external_decls[2],
                               false,
                               false,
                               "my_union",
                               3);
    check_external_decl_enum(&tl.external_decls[3], false, "my_enum", 3);
    check_external_decl_func_def_predef(&tl.external_decls[7],
                                        sc,
                                        fs,
                                        0,
                                        INT,
                                        "main",
                                        22);
    check_external_decl_func_def_predef(&tl.external_decls[8],
                                        sc_static,
                                        fs,
                                        3,
                                        INT,
                                        "do_shit",
                                        18);
    check_external_decl_func_def_predef(
        &tl.external_decls[9],
        sc_static,
        (struct func_specs){.is_noreturn = true},
        0,
        VOID,
        "variadic",
        6);
    check_external_decl_func_def_predef(&tl.external_decls[10],
                                        sc,
                                        fs,
                                        0,
                                        VOID,
                                        "strcpy_for_some_reason",
                                        2);

    free_translation_unit(&tl);
    free_tokens(tokens);
}

TEST(large_testfile) {
    const char* file = "../frontend/test/files/large_testfile.c";
    struct token* tokens = tokenize(file);
    
    struct parser_err err = create_parser_err();
    struct translation_unit tl = parse_tokens(tokens, &err);
    ASSERT(err.type == PARSER_ERR_NONE);
    ASSERT_SIZE_T(tl.len, (size_t)88);

    compare_with_ex_file(&tl, "../frontend/test/files/large_testfile.c.ast");

    const struct storage_class sc = {false, false, false, false, false, false};
    const struct storage_class sc_static = {.is_static = true};
    const struct func_specs fs = {false, false};

    check_external_decl_enum(&tl.external_decls[21], false, "token_type", 97);

    check_external_decl_struct(&tl.external_decls[35],
                               false,
                               true,
                               "source_location",
                               1);
    check_external_decl_struct(&tl.external_decls[36], false, true, "token", 4);

    check_external_decl_enum(&tl.external_decls[40], false, "error_type", 3);

    check_external_decl_struct(&tl.external_decls[53],
                               false,
                               true,
                               "token_arr",
                               3);
    check_external_decl_struct(&tl.external_decls[54],
                               false,
                               true,
                               "tokenizer_state",
                               5);

    check_external_decl_func_def_struct(&tl.external_decls[68],
                                        sc,
                                        fs,
                                        "token",
                                        1,
                                        "tokenize",
                                        10);
    check_external_decl_func_def_predef(&tl.external_decls[69],
                                        sc,
                                        fs,
                                        0,
                                        VOID,
                                        "free_tokenizer_result",
                                        2);
    check_external_decl_func_def_typedef(
        &tl.external_decls[70],
        sc_static,
        (struct func_specs){.is_inline = true, .is_noreturn = false},
        0,
        "bool",
        "is_spelling",
        1);
    check_external_decl_func_def_enum(&tl.external_decls[71],
                                      sc_static,
                                      fs,
                                      "token_type",
                                      0,
                                      "multic_token_type",
                                      1);
    check_external_decl_func_def_enum(&tl.external_decls[72],
                                      sc_static,
                                      fs,
                                      "token_type",
                                      0,
                                      "singlec_token_type",
                                      1);
    check_external_decl_func_def_typedef(&tl.external_decls[73],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "check_type",
                                         5);
    check_external_decl_func_def_enum(&tl.external_decls[74],
                                      sc_static,
                                      fs,
                                      "token_type",
                                      0,
                                      "check_next",
                                      3);
    check_external_decl_func_def_typedef(&tl.external_decls[75],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "is_valid_singlec_token",
                                         2);
    check_external_decl_func_def_predef(&tl.external_decls[76],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "advance",
                                        4);
    check_external_decl_func_def_predef(&tl.external_decls[77],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "advance_one",
                                        4);
    check_external_decl_func_def_predef(&tl.external_decls[78],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "advance_newline",
                                        4);
    check_external_decl_func_def_predef(&tl.external_decls[79],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "realloc_tokens_if_needed",
                                        1);
    check_external_decl_func_def_predef(&tl.external_decls[80],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "add_token_copy",
                                        3);
    check_external_decl_func_def_predef(&tl.external_decls[81],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "add_token",
                                        3);
    check_external_decl_func_def_typedef(&tl.external_decls[82],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "handle_comments",
                                         1);
    check_external_decl_func_def_enum(&tl.external_decls[83],
                                      sc_static,
                                      fs,
                                      "token_type",
                                      0,
                                      "get_char_lit_type",
                                      1);
    check_external_decl_func_def_predef(&tl.external_decls[84],
                                        sc_static,
                                        fs,
                                        0,
                                        VOID,
                                        "unterminated_literal_err",
                                        3);
    check_external_decl_func_def_typedef(&tl.external_decls[85],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "handle_character_literal",
                                         16);
    check_external_decl_func_def_typedef(&tl.external_decls[86],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "token_is_over",
                                         2);
    check_external_decl_func_def_typedef(&tl.external_decls[87],
                                         sc_static,
                                         fs,
                                         0,
                                         "bool",
                                         "handle_other",
                                         11);

    free_translation_unit(&tl);
    free_tokens(tokens);
}

TEST_SUITE_BEGIN(parser_file, 3) {
    REGISTER_TEST(no_preproc);
    REGISTER_TEST(parser_testfile);
    REGISTER_TEST(large_testfile);
}
TEST_SUITE_END()