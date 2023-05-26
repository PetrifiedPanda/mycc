#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_serializer.h"
#include "frontend/ast/ast_deserializer.h"

#include "frontend/arg_parse.h"

#include "util/Str.h"

static bool convert_bin_to_text(const CmdArgs* args, const char* filename);

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       const char* filename);

int main(int argc, char** argv) {
    const CmdArgs args = parse_cmd_args(argc, argv);
    const bool is_windows =
#ifdef _WIN32
        true;
#else
        false;
#endif
    const ArchTypeInfo type_info = get_arch_type_info(ARCH_X86_64, is_windows);

    for (int i = 0; i < args.num_files; ++i) {
        const char* filename = args.files[i];
        if (args.action == ARG_ACTION_CONVERT_BIN_TO_TEXT) {
            if (!convert_bin_to_text(&args, filename)) {
                CmdArgs_free(&args);
                return EXIT_FAILURE;
            }
        } else {
            if (!output_ast(&args, &type_info, filename)) {
                CmdArgs_free(&args);
                return EXIT_FAILURE;
            }
        }
    }
    CmdArgs_free(&args);
    return EXIT_SUCCESS;
}

static bool is_file_sep(char c) {
    switch (c) {
        case '/':
#ifdef _WIN32
        case '\\':
#endif
            return true;
        default:
            return false;
    }
}

static const char* strip_file_location(const char* filename) {
    const char* res = filename;

    const char* it = filename;
    while (*it != '\0') {
        if (is_file_sep(*it)) {
            res = it + 1;
        }
        ++it;
    }
    return res;
}

static Str get_out_filename(const char* origin_file, const char* suffix) {
    const char* filename_only = strip_file_location(origin_file);
    return Str_concat(strlen(filename_only),
                      filename_only,
                      strlen(suffix),
                      suffix);
}

static bool convert_bin_to_text(const CmdArgs* args, const char* filename) {
    FILE* in_file = fopen(filename, "rb");
    if (!in_file) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        return false;
    }
    DeserializeAstRes res = deserialize_ast(in_file);
    if (!res.is_valid) {
        fprintf(stderr, "Failed to read ast from file %s\n", filename);
        fclose(in_file);
        return false;
    }
    fclose(in_file);

    Str out_filename_str = args->output_file == NULL
                               ? get_out_filename(filename, ".ast")
                               : Str_create_null();
    const char* out_filename = Str_is_valid(&out_filename_str)
                                   ? Str_get_data(&out_filename_str)
                                   : args->output_file;
    FILE* out_file = fopen(out_filename, "w");
    if (!out_file) {
        fprintf(stderr, "Failed to open file %s\n", out_filename);
        goto fail_with_out_file_closed;
    }
    if (!dump_ast(&res.tl, &res.file_info, out_file)) {
        fprintf(stderr, "Failed to write ast to textfile %s\n", out_filename);
        goto fail_with_out_file_open;
    }
    if (fflush(out_file) != 0) {
        fprintf(stderr, "Failed to flush output file %s\n", out_filename);
        goto fail_with_out_file_open;
    }
    fclose(out_file);
    Str_free(&out_filename_str);
    TranslationUnit_free(&res.tl);
    FileInfo_free(&res.file_info);
    return true;

fail_with_out_file_open:
    fclose(out_file);
fail_with_out_file_closed:
    Str_free(&out_filename_str);
    TranslationUnit_free(&res.tl);
    FileInfo_free(&res.file_info);
    return false;
}

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       const char* filename) {
    PreprocErr preproc_err = PreprocErr_create();
    PreprocRes preproc_res = preproc(filename, &preproc_err);
    if (preproc_err.kind != PREPROC_ERR_NONE) {
        PreprocErr_print(stderr, &preproc_res.file_info, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_before_ast_generated;
    }
    if (!convert_preproc_tokens(preproc_res.toks, type_info, &preproc_err)) {
        PreprocErr_print(stderr, &preproc_res.file_info, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_before_ast_generated;
    }

    ParserErr parser_err = ParserErr_create();
    TranslationUnit tl = parse_tokens(preproc_res.toks, &parser_err);
    if (parser_err.kind != PARSER_ERR_NONE) {
        ParserErr_print(stderr, &preproc_res.file_info, &parser_err);
        ParserErr_free(&parser_err);
        goto fail_before_ast_generated;
    }

    const char* suffix = args->action == ARG_ACTION_OUTPUT_BIN ? ".binast"
                                                               : ".ast";
    Str out_filename_str = args->output_file == NULL
                               ? get_out_filename(filename, suffix)
                               : Str_create_null();
    const char* out_filename = Str_is_valid(&out_filename_str)
                                   ? Str_get_data(&out_filename_str)
                                   : args->output_file;
    FILE* out_file = fopen(out_filename, "wb");
    if (!out_file) {
        fprintf(stderr, "Failed to open output file %s\n", out_filename);
        goto fail_with_out_file_closed;
    }

    const bool success = args->action == ARG_ACTION_OUTPUT_BIN
                             ? serialize_ast(&tl,
                                             &preproc_res.file_info,
                                             out_file)
                             : dump_ast(&tl, &preproc_res.file_info, out_file);
    if (!success) {
        fprintf(stderr, "Failed to write ast to file %s\n", out_filename);
        if (fflush(out_file) != 0) {
            fprintf(stderr, "Failed to flush output file %s\n", out_filename);
        }
        goto fail_with_out_file_open;
    }

    if (fflush(out_file) != 0) {
        fprintf(stderr, "Failed to flush output file %s\n", out_filename);
        goto fail_with_out_file_open;
    }
    fclose(out_file);
    Str_free(&out_filename_str);
    TranslationUnit_free(&tl);
    PreprocRes_free(&preproc_res);
    return true;
fail_with_out_file_open:
    fclose(out_file);
fail_with_out_file_closed:
    Str_free(&out_filename_str);
    TranslationUnit_free(&tl);
fail_before_ast_generated:
    PreprocRes_free(&preproc_res);
    return false;
}

