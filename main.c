#include <stdlib.h>

#include "frontend/preproc/preproc.h"

#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_serializer.h"

#include "frontend/arg_parse.h"

#include "util/StrBuf.h"
#include "util/paths.h"
#include "util/log.h"

static bool convert_bin_to_text(const CmdArgs* args, CStr filename);

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       CStr filename);

int main(int argc, char** argv) {
    const CmdArgs args = parse_cmd_args(argc, argv);
    const bool is_windows =
#ifdef _WIN32
        true;
#else
        false;
#endif
    const ArchTypeInfo type_info = get_arch_type_info(ARCH_X86_64, is_windows);

    if (args.action == ARG_ACTION_CONVERT_BIN_TO_TEXT) {
        for (uint32_t i = 0; i < args.num_files; ++i) {
            if (!convert_bin_to_text(&args, args.files[i])) {
                goto fail;
            }
        }
    } else {
        for (uint32_t i = 0; i < args.num_files; ++i) {
            if (!output_ast(&args, &type_info, args.files[i])) {
                goto fail;
            }
        }
    }
    CmdArgs_free(&args);
    return EXIT_SUCCESS;
fail:
    CmdArgs_free(&args);
    return EXIT_FAILURE;
}

static StrBuf get_out_filename(Str origin_file, Str suffix) {
    uint32_t last_sep_idx = get_last_file_sep(origin_file);
    Str filename_only = Str_advance(origin_file, last_sep_idx + 1);
    return StrBuf_concat(filename_only, suffix);
}

static bool convert_bin_to_text(const CmdArgs* args, CStr filename) {
    MYCC_LOG("Converting {Str} to human readable file:\n", filename);
    File in_file = File_open(filename, FILE_READ | FILE_BINARY);
    if (!File_valid(in_file)) {
        File_printf(mycc_stderr, "Failed to open file {Str}\n", filename);
        return false;
    }
    DeserializeASTRes res = deserialize_ast(in_file);
    if (res.ast.len == 0) {
        File_printf(mycc_stderr,
                    "Failed to read ast from file {Str}\n",
                    filename);
        File_close(in_file);
        return false;
    }
    File_close(in_file);

    StrBuf out_filename_str;
    CStr out_filename;
    if (args->output_file.data == NULL) {
        out_filename_str = get_out_filename(CStr_as_str(filename),
                                            STR_LIT(".ast"));
        out_filename = StrBuf_c_str(&out_filename_str);
    } else {
        out_filename_str = StrBuf_null();
        out_filename = args->output_file;
    }
    File out_file = File_open(out_filename, FILE_WRITE);
    if (!File_valid(out_file)) {
        File_printf(mycc_stderr, "Failed to open file {Str}\n", out_filename);
        goto fail_with_out_file_closed;
    }
    if (!dump_ast(&res.ast, &res.file_info, out_file)) {
        File_printf(mycc_stderr,
                    "Failed to write ast to textfile {Str}\n",
                    out_filename);
        goto fail_with_out_file_open;
    }
    if (!File_flush(out_file)) {
        File_printf(mycc_stderr,
                    "Failed to flush output file {Str}\n",
                    out_filename);
        goto fail_with_out_file_open;
    }
    File_close(out_file);
    StrBuf_free(&out_filename_str);
    AST_free(&res.ast);
    FileInfo_free(&res.file_info);
    MYCC_LOG_STR("\n");
    return true;
fail_with_out_file_open:
    File_close(out_file);
fail_with_out_file_closed:
    StrBuf_free(&out_filename_str);
    AST_free(&res.ast);
    FileInfo_free(&res.file_info);
    MYCC_LOG_STR("\n");
    return false;
}

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       CStr filename) {
    MYCC_LOG("Generating AST for {Str}:\n", filename);
    PreprocErr preproc_err = PreprocErr_create();
    PreprocRes preproc_res = preproc(filename,
                                     args->num_include_dirs,
                                     args->include_dirs,
                                     type_info,
                                     &preproc_err);
    if (preproc_err.kind != PREPROC_ERR_NONE) {
        PreprocErr_print(mycc_stderr, &preproc_res.file_info, &preproc_res.vals, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_preproc;
    }
    TokenArr tokens = convert_preproc_tokens(&preproc_res.toks, &preproc_res.vals, type_info, &preproc_err);
    if (tokens.len == 0) {
        PreprocErr_print(mycc_stderr, &preproc_res.file_info, &preproc_res.vals, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_preproc;
    }

    ParserErr parser_err = ParserErr_create();
    AST ast = parse_ast(&tokens, &parser_err);
    if (parser_err.kind != PARSER_ERR_NONE) {
        // TODO: tokens are now in tl and need to be freed
        ParserErr_print(mycc_stderr,
                        &preproc_res.file_info,
                        &ast.toks,
                        &parser_err);
        goto fail_parse;
    }

    Str suffix = args->action == ARG_ACTION_OUTPUT_BIN ? STR_LIT(".binast")
                                                       : STR_LIT(".ast");
    StrBuf out_filename_str;
    CStr out_filename;
    if (args->output_file.data == NULL) {
        out_filename_str = get_out_filename(CStr_as_str(filename), suffix);
        out_filename = StrBuf_c_str(&out_filename_str);
    } else {
        out_filename_str = StrBuf_null();
        out_filename = args->output_file;
    }
    File out_file = File_open(out_filename, FILE_WRITE | FILE_BINARY);
    if (!File_valid(out_file)) {
        File_printf(mycc_stderr,
                    "Failed to open output file {Str}\n",
                    out_filename);
        goto fail_out_file_closed;
    }

    const bool success = args->action == ARG_ACTION_OUTPUT_BIN
                             ? serialize_ast(&ast,
                                               &preproc_res.file_info,
                                               out_file)
                             : dump_ast(&ast,
                                          &preproc_res.file_info,
                                          out_file);
    if (!success) {
        File_printf(mycc_stderr,
                    "Failed to write ast to file {Str}\n",
                    out_filename);
        if (!File_flush(out_file)) {
            File_printf(mycc_stderr,
                        "Failed to flush output file {Str}\n",
                        out_filename);
        }
        goto fail_out_file_open;
    }

    if (!File_flush(out_file)) {
        File_printf(mycc_stderr,
                    "Failed to flush output file {Str}\n",
                    out_filename);
        goto fail_out_file_open;
    }
    File_close(out_file);
    StrBuf_free(&out_filename_str);
    AST_free(&ast);
    PreprocRes_free(&preproc_res);
    MYCC_LOG_STR("\n");
    return true;
fail_out_file_open:
    File_close(out_file);
fail_out_file_closed:
    StrBuf_free(&out_filename_str);
fail_parse:
    AST_free(&ast);
fail_preproc:
    PreprocRes_free(&preproc_res);
    MYCC_LOG_STR("\n");
    return false;
}

