#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_deserializer.h"
#include "frontend/ast/compare_asts.h"

#include "../test_helpers.h"

#include "parser_test_util.h"

static void check_func_specs(const FuncSpecs* got, const FuncSpecs* expected) {
    ASSERT_BOOL(got->is_inline, expected->is_inline);
    ASSERT_BOOL(got->is_noreturn, expected->is_noreturn);
}

static void check_storage_class(const StorageClass* got,
                                const StorageClass* expected) {
    ASSERT_BOOL(got->is_typedef, expected->is_typedef);
    ASSERT_BOOL(got->is_extern, expected->is_extern);
    ASSERT_BOOL(got->is_static, expected->is_static);
    ASSERT_BOOL(got->is_thread_local, expected->is_thread_local);
    ASSERT_BOOL(got->is_auto, expected->is_auto);
    ASSERT_BOOL(got->is_register, expected->is_register);
}

static void check_external_decl_struct(ExternalDeclaration* d,
                                       bool is_typedef,
                                       bool is_struct,
                                       Str id_spell,
                                       uint32_t decl_list_len,
                                       const TokenArr* arr) {
    ASSERT(d->is_func_def == false);
    ASSERT(d->decl.is_normal_decl);
    ASSERT(d->decl.decl_specs.type_specs.kind == TYPE_SPEC_STRUCT);
    const FuncSpecs none = {false, false};
    check_func_specs(&d->decl.decl_specs.func_specs, &none);

    StorageClass sc = {.is_typedef = is_typedef};
    check_storage_class(&d->decl.decl_specs.storage_class, &sc);

    ASSERT_BOOL(d->decl.decl_specs.type_specs.struct_union_spec->is_struct,
                is_struct);
    check_identifier(
        d->decl.decl_specs.type_specs.struct_union_spec->identifier,
        id_spell, arr);

    ASSERT_UINT(
        d->decl.decl_specs.type_specs.struct_union_spec->decl_list.len,
        decl_list_len);
}

static void check_external_decl_enum(ExternalDeclaration* d,
                                     bool is_typedef,
                                     Str id_spell,
                                     uint32_t enum_list_len,
                                     const TokenArr* arr) {
    ASSERT(d->is_func_def == false);
    ASSERT(d->decl.is_normal_decl);
    ASSERT(d->decl.decl_specs.type_specs.kind == TYPE_SPEC_ENUM);
    const FuncSpecs none = {false, false};
    check_func_specs(&d->decl.decl_specs.func_specs, &none);
    StorageClass sc = {.is_typedef = is_typedef};
    check_storage_class(&d->decl.decl_specs.storage_class, &sc);
    check_identifier(d->decl.decl_specs.type_specs.enum_spec->identifier,
                     id_spell, arr);

    ASSERT_UINT(d->decl.decl_specs.type_specs.enum_spec->enum_list.len,
                  enum_list_len);
}

static void check_pointer_indirs(Pointer* ptr, uint32_t num_indirs) {
    if (num_indirs == 0) {
        ASSERT_NULL(ptr);
    } else {
        ASSERT_NOT_NULL(ptr);
        ASSERT_UINT(ptr->num_indirs, num_indirs);
    }
}

static void check_external_decl_func_def(ExternalDeclaration* d,
                                         const StorageClass* storage_class,
                                         const FuncSpecs* func_specs,
                                         uint32_t num_indirs,
                                         Str id_spell,
                                         uint32_t body_len,
                                         const TokenArr* arr) {
    ASSERT(d->is_func_def);
    ASSERT(d->func_def.decl->direct_decl->is_id);
    check_identifier(d->func_def.decl->direct_decl->id, id_spell, arr);
    check_pointer_indirs(d->func_def.decl->ptr, num_indirs);

    check_storage_class(&d->func_def.specs.storage_class, storage_class);
    check_func_specs(&d->func_def.specs.func_specs, func_specs);
    ASSERT_UINT(d->func_def.comp.len, body_len);
}

static void check_external_decl_func_def_enum(ExternalDeclaration* d,
                                              StorageClass storage_class,
                                              FuncSpecs func_specs,
                                              Str enum_name,
                                              uint32_t num_indirs,
                                              Str func_name,
                                              uint32_t body_len,
                                              const TokenArr* arr) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len,
                                 arr);

    ASSERT(d->func_def.specs.type_specs.kind == TYPE_SPEC_ENUM);
    check_identifier(d->func_def.specs.type_specs.enum_spec->identifier,
                     enum_name, arr);
    ASSERT_UINT(d->func_def.specs.type_specs.enum_spec->enum_list.len,
                  (uint32_t)0);
}

static void check_external_decl_func_def_struct(ExternalDeclaration* d,
                                                StorageClass storage_class,
                                                FuncSpecs func_specs,
                                                Str struct_name,
                                                uint32_t num_indirs,
                                                Str func_name,
                                                uint32_t body_len,
                                                const TokenArr* arr) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len,
                                 arr);

    ASSERT(d->func_def.specs.type_specs.kind == TYPE_SPEC_STRUCT);
    ASSERT(d->func_def.specs.type_specs.struct_union_spec->is_struct);
    check_identifier(
        d->func_def.specs.type_specs.struct_union_spec->identifier,
        struct_name, arr);
    ASSERT_UINT(
        d->func_def.specs.type_specs.struct_union_spec->decl_list.len,
        (uint32_t)0);
}

static void check_external_decl_func_def_predef(ExternalDeclaration* d,
                                                StorageClass storage_class,
                                                FuncSpecs func_specs,
                                                uint32_t num_indirs,
                                                TypeSpecKind ret_type,
                                                Str id_spell,
                                                uint32_t body_len,
                                                const TokenArr* arr) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 id_spell,
                                 body_len,
                                 arr);

    ASSERT(d->func_def.specs.type_specs.kind == ret_type);
}

static void check_external_decl_func_def_typedef(ExternalDeclaration* d,
                                                 StorageClass storage_class,
                                                 FuncSpecs func_specs,
                                                 uint32_t num_indirs,
                                                 Str ret_type,
                                                 Str func_name,
                                                 uint32_t body_len,
                                                 const TokenArr* arr) {
    check_external_decl_func_def(d,
                                 &storage_class,
                                 &func_specs,
                                 num_indirs,
                                 func_name,
                                 body_len,
                                 arr);

    ASSERT(d->func_def.specs.type_specs.kind == TYPE_SPEC_TYPENAME);
    ASSERT_STR(
        StrBuf_as_str(&arr->vals[d->func_def.specs.type_specs.typedef_name->info.token_idx].spelling),
        ret_type);
}

static void compare_with_ex_file(const TranslationUnit* got,
                                 const FileInfo* file_info,
                                 Str ex_filename) {
    FILE* ex_file = fopen(ex_filename.data, "rb");
    ASSERT(ex_file);
    DeserializeAstRes expected = deserialize_ast((File){ex_file});
    ASSERT(expected.is_valid);
    ASSERT(fclose(ex_file) == 0);

    ASSERT(expected.file_info.len == file_info->len);
    ASSERT(compare_asts(got, file_info, &expected.tl, &expected.file_info));
    TranslationUnit_free(&expected.tl);
    FileInfo_free(&expected.file_info);
}

TEST(no_preproc) {
    CStr file = CSTR_LIT("../frontend/test/files/no_preproc.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_UINT(tl.len, (uint32_t)10);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(&tl,
                         &res.file_info,
                         STR_LIT("../frontend/test/files/no_preproc.c.binast"));

    const StorageClass sc = {false, false, false, false, false, false};
    const StorageClass sc_static = {.is_static = true};
    const FuncSpecs fs = (FuncSpecs){false, false};

    check_external_decl_struct(&tl.external_decls[0],
                               true,
                               true,
                               Str_null(),
                               2,
                               &tl.tokens);
    check_external_decl_struct(&tl.external_decls[1],
                               false,
                               false,
                               STR_LIT("my_union"),
                               2,
                               &tl.tokens);
    check_external_decl_enum(&tl.external_decls[2],
                             false,
                             STR_LIT("my_enum"),
                             3,
                             &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[6],
                                        sc,
                                        fs,
                                        0,
                                        TYPE_SPEC_INT,
                                        STR_LIT("main"),
                                        15,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[7],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_INT,
                                        STR_LIT("do_shit"),
                                        13,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[8],
                                        sc_static,
                                        (FuncSpecs){.is_noreturn = true},
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("variadic"),
                                        8,
                                        &tl.tokens);

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST(parser_testfile) {
    CStr file = CSTR_LIT("../frontend/test/files/parser_testfile.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_UINT(tl.len, (uint32_t)19);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(
        &tl,
        &res.file_info,
        STR_LIT("../frontend/test/files/parser_testfile.c.binast"));

    Str tmp_filename = STR_LIT("tmp.ast");
    FILE* tmp_file = fopen(tmp_filename.data, "w");
    ASSERT_NOT_NULL(tmp_file);
    dump_ast(&tl, &res.file_info, (File){tmp_file});

    ASSERT_INT(fclose(tmp_file), 0);

    test_compare_files(
        Str_c_str(tmp_filename),
        Str_c_str(STR_LIT("../frontend/test/files/parser_testfile.c.ast")));

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST(large_testfile) {
    CStr file = CSTR_LIT("../frontend/test/files/large_testfile.c");
    PreprocRes res = tokenize(file);

    ParserErr err = ParserErr_create();
    TranslationUnit tl = parse_tokens(&res.toks, &err);
    ASSERT(err.kind == PARSER_ERR_NONE);
    ASSERT_UINT(tl.len, (uint32_t)88);
    ASSERT(compare_asts(&tl, &res.file_info, &tl, &res.file_info));

    compare_with_ex_file(
        &tl,
        &res.file_info,
        STR_LIT("../frontend/test/files/large_testfile.c.binast"));

    const StorageClass sc = {false, false, false, false, false, false};
    const StorageClass sc_static = {.is_static = true};
    const FuncSpecs fs = {false, false};

    check_external_decl_enum(&tl.external_decls[21],
                             false,
                             STR_LIT("token_type"),
                             97,
                             &tl.tokens);

    check_external_decl_struct(&tl.external_decls[35],
                               false,
                               true,
                               STR_LIT("source_location"),
                               1,
                               &tl.tokens);
    check_external_decl_struct(&tl.external_decls[36],
                               false,
                               true,
                               STR_LIT("token"),
                               4,
                               &tl.tokens);

    check_external_decl_enum(&tl.external_decls[40],
                             false,
                             STR_LIT("error_type"),
                             3,
                             &tl.tokens);

    check_external_decl_struct(&tl.external_decls[53],
                               false,
                               true,
                               STR_LIT("token_arr"),
                               3,
                               &tl.tokens);
    check_external_decl_struct(&tl.external_decls[54],
                               false,
                               true,
                               STR_LIT("tokenizer_state"),
                               5,
                               &tl.tokens);

    check_external_decl_func_def_struct(&tl.external_decls[68],
                                        sc,
                                        fs,
                                        STR_LIT("token"),
                                        1,
                                        STR_LIT("tokenize"),
                                        10,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[69],
                                        sc,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("free_tokenizer_result"),
                                        2,
                                        &tl.tokens);
    check_external_decl_func_def_typedef(
        &tl.external_decls[70],
        sc_static,
        (FuncSpecs){.is_inline = true, .is_noreturn = false},
        0,
        STR_LIT("bool"),
        STR_LIT("is_spelling"),
        1,
        &tl.tokens);
    check_external_decl_func_def_enum(&tl.external_decls[71],
                                      sc_static,
                                      fs,
                                      STR_LIT("token_type"),
                                      0,
                                      STR_LIT("multic_token_type"),
                                      1,
                                      &tl.tokens);
    check_external_decl_func_def_enum(&tl.external_decls[72],
                                      sc_static,
                                      fs,
                                      STR_LIT("token_type"),
                                      0,
                                      STR_LIT("singlec_token_type"),
                                      1,
                                      &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[73],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("check_type"),
                                         5,
                                         &tl.tokens);
    check_external_decl_func_def_enum(&tl.external_decls[74],
                                      sc_static,
                                      fs,
                                      STR_LIT("token_type"),
                                      0,
                                      STR_LIT("check_next"),
                                      3,
                                      &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[75],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("is_valid_singlec_token"),
                                         2,
                                         &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[76],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("advance"),
                                        4,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[77],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("advance_one"),
                                        4,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[78],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("advance_newline"),
                                        4,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[79],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("realloc_tokens_if_needed"),
                                        1,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[80],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("add_token_copy"),
                                        3,
                                        &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[81],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("add_token"),
                                        3,
                                        &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[82],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("handle_comments"),
                                         1,
                                         &tl.tokens);
    check_external_decl_func_def_enum(&tl.external_decls[83],
                                      sc_static,
                                      fs,
                                      STR_LIT("token_type"),
                                      0,
                                      STR_LIT("get_char_lit_type"),
                                      1,
                                      &tl.tokens);
    check_external_decl_func_def_predef(&tl.external_decls[84],
                                        sc_static,
                                        fs,
                                        0,
                                        TYPE_SPEC_VOID,
                                        STR_LIT("unterminated_literal_err"),
                                        3,
                                        &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[85],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("handle_character_literal"),
                                         16,
                                         &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[86],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("token_is_over"),
                                         2,
                                         &tl.tokens);
    check_external_decl_func_def_typedef(&tl.external_decls[87],
                                         sc_static,
                                         fs,
                                         0,
                                         STR_LIT("bool"),
                                         STR_LIT("handle_other"),
                                         11,
                                         &tl.tokens);

    TranslationUnit_free(&tl);
    PreprocRes_free(&res);
}

TEST_SUITE_BEGIN(parser_file){
    REGISTER_TEST(no_preproc),
    REGISTER_TEST(parser_testfile),
    REGISTER_TEST(large_testfile),
} TEST_SUITE_END()
