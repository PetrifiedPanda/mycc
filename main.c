#include <string.h>
#include <stdlib.h>

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_serializer.h"
#include "frontend/ast/ast_deserializer.h"

#include "frontend/arg_parse.h"

#include "util/StrBuf.h"

static bool convert_bin_to_text(const CmdArgs* args, Str filename);

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       Str filename);

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
        Str filename = args.files[i];
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

static Str strip_file_location(Str filename) {
    Str res = filename;
    size_t i = 0;
    while (i != filename.len) {
        if (is_file_sep(Str_at(filename, i))) {
            res = Str_advance(filename, i + 1);
        }
        ++i;
    }
    return res;
}

static StrBuf get_out_filename(Str origin_file, Str suffix) {
    Str filename_only = strip_file_location(origin_file);
    return StrBuf_concat(filename_only, suffix);
}

static bool convert_bin_to_text(const CmdArgs* args, Str filename) {
    File in_file = File_open(Str_c_str(filename), FILE_READ | FILE_BINARY);
    if (!File_valid(in_file)) {
        File_printf(mycc_stderr(), "Failed to open file {Str}\n", filename);
        return false;
    }
    DeserializeAstRes res = deserialize_ast(in_file);
    if (!res.is_valid) {
        File_printf(mycc_stderr(),
                    "Failed to read ast from file {Str}\n",
                    filename);
        File_close(in_file);
        return false;
    }
    File_close(in_file);

    StrBuf out_filename_str = args->output_file.data == NULL
                                  ? get_out_filename(filename, STR_LIT(".ast"))
                                  : StrBuf_null();
    CStr out_filename = StrBuf_valid(&out_filename_str)
                            ? StrBuf_c_str(&out_filename_str)
                            : args->output_file;
    File out_file = File_open(out_filename, FILE_WRITE);
    if (!File_valid(out_file)) {
        File_printf(mycc_stderr(), "Failed to open file {Str}\n", out_filename);
        goto fail_with_out_file_closed;
    }
    if (!dump_ast(&res.tl, &res.file_info, out_file)) {
        File_printf(mycc_stderr(),
                    "Failed to write ast to textfile {Str}\n",
                    out_filename);
        goto fail_with_out_file_open;
    }
    if (!File_flush(out_file)) {
        File_printf(mycc_stderr(),
                    "Failed to flush output file {Str}\n",
                    out_filename);
        goto fail_with_out_file_open;
    }
    File_close(out_file);
    StrBuf_free(&out_filename_str);
    TranslationUnit_free(&res.tl);
    FileInfo_free(&res.file_info);
    return true;

fail_with_out_file_open:
    File_close(out_file);
fail_with_out_file_closed:
    StrBuf_free(&out_filename_str);
    TranslationUnit_free(&res.tl);
    FileInfo_free(&res.file_info);
    return false;
}

static bool output_ast(const CmdArgs* args,
                       const ArchTypeInfo* type_info,
                       Str filename) {
    PreprocErr preproc_err = PreprocErr_create();
    PreprocRes preproc_res = preproc(filename, &preproc_err);
    if (preproc_err.kind != PREPROC_ERR_NONE) {
        PreprocErr_print(mycc_stderr(), &preproc_res.file_info, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_before_ast_generated;
    }
    if (!convert_preproc_tokens(preproc_res.toks, type_info, &preproc_err)) {
        PreprocErr_print(mycc_stderr(), &preproc_res.file_info, &preproc_err);
        PreprocErr_free(&preproc_err);
        goto fail_before_ast_generated;
    }

    ParserErr parser_err = ParserErr_create();
    TranslationUnit tl = parse_tokens(preproc_res.toks, &parser_err);
    if (parser_err.kind != PARSER_ERR_NONE) {
        ParserErr_print(mycc_stderr(), &preproc_res.file_info, &parser_err);
        ParserErr_free(&parser_err);
        goto fail_before_ast_generated;
    }

    Str suffix = args->action == ARG_ACTION_OUTPUT_BIN ? STR_LIT(".binast")
                                                       : STR_LIT(".ast");
    StrBuf out_filename_str = args->output_file.data == NULL
                                  ? get_out_filename(filename, suffix)
                                  : StrBuf_null();
    CStr out_filename = StrBuf_valid(&out_filename_str)
                            ? StrBuf_c_str(&out_filename_str)
                            : args->output_file;
    File out_file = File_open(out_filename, FILE_WRITE | FILE_BINARY);
    if (!File_valid(out_file)) {
        File_printf(mycc_stderr(),
                    "Failed to open output file {Str}\n",
                    out_filename);
        goto fail_with_out_file_closed;
    }

    const bool success = args->action == ARG_ACTION_OUTPUT_BIN
                             ? serialize_ast(&tl,
                                             &preproc_res.file_info,
                                             out_file)
                             : dump_ast(&tl, &preproc_res.file_info, out_file);
    if (!success) {
        File_printf(mycc_stderr(),
                    "Failed to write ast to file {Str}\n",
                    out_filename);
        if (!File_flush(out_file)) {
            File_printf(mycc_stderr(),
                        "Failed to flush output file {Str}\n",
                        out_filename);
        }
        goto fail_with_out_file_open;
    }

    if (!File_flush(out_file)) {
        File_printf(mycc_stderr(),
                    "Failed to flush output file {Str}\n",
                    out_filename);
        goto fail_with_out_file_open;
    }
    File_close(out_file);
    StrBuf_free(&out_filename_str);
    TranslationUnit_free(&tl);
    PreprocRes_free(&preproc_res);
    return true;
fail_with_out_file_open:
    File_close(out_file);
fail_with_out_file_closed:
    StrBuf_free(&out_filename_str);
    TranslationUnit_free(&tl);
fail_before_ast_generated:
    PreprocRes_free(&preproc_res);
    return false;
}

