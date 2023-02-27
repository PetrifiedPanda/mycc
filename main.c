#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "frontend/ast/ast_dumper.h"
#include "frontend/ast/ast_serializer.h"
#include "frontend/ast/ast_deserializer.h"

#include "frontend/arg_parse.h"

#include "util/str.h"

static struct str get_out_filename(const char* origin_file, const char* suffix);

int main(int argc, char** argv) {
    const struct cmd_args args = parse_cmd_args(argc, argv);
    const bool is_windows =
#ifdef _WIN32
        true;
#else
        false;
#endif
    const struct arch_type_info type_info = get_arch_type_info(ARCH_X86_64,
                                                               is_windows);

    for (int i = 0; i < args.num_files; ++i) {
        const char* filename = args.files[i];
        if (args.action == ARG_ACTION_CONVERT_BIN_TO_TEXT) {
            FILE* in_file = fopen(filename, "rb");
            if (!in_file) {
                fprintf(stderr, "Failed to open file %s\n", filename);
                return EXIT_FAILURE;
            }
            struct deserialize_ast_res res = deserialize_ast(in_file);
            if (!res.is_valid) {
                fclose(in_file);
                fprintf(stderr, "Failed to read ast from file %s\n", filename);
                return EXIT_FAILURE;
            }
            if (fclose(in_file) != 0) {
                free_translation_unit(&res.tl);
                free_file_info(&res.file_info);
                fprintf(stderr, "Failed to close file %s\n", filename);
                return EXIT_FAILURE;
            }

            struct str out_filename_str = args.output_file == NULL
                                              ? get_out_filename(filename,
                                                                 ".ast")
                                              : create_null_str();
            const char* out_filename = str_is_valid(&out_filename_str)
                                           ? str_get_data(&out_filename_str)
                                           : args.output_file;
            FILE* out_file = fopen(out_filename, "w");
            if (!out_file) {
                fprintf(stderr, "Failed to open file %s\n", out_filename);
                return EXIT_FAILURE;
            }
            if (!dump_ast(&res.tl, &res.file_info, out_file)) {
                fprintf(stderr,
                        "Failed to write ast to textfile %s\n",
                        out_filename);
                fclose(out_file);
                free_str(&out_filename_str);
                free_translation_unit(&res.tl);
                free_file_info(&res.file_info);
                return EXIT_FAILURE;
            }
            fflush(out_file);
            if (fclose(out_file) != 0) {
                fprintf(stderr,
                        "Failed to close output file %s\n",
                        out_filename);
                free_str(&out_filename_str);
                free_translation_unit(&res.tl);
                free_file_info(&res.file_info);
                return EXIT_FAILURE;
            }
            free_str(&out_filename_str);
            free_translation_unit(&res.tl);
            free_file_info(&res.file_info);
        } else {
            struct preproc_err preproc_err = create_preproc_err();
            struct preproc_res preproc_res = preproc(filename, &preproc_err);
            if (preproc_err.type != PREPROC_ERR_NONE) {
                print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
                free_preproc_err(&preproc_err);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }
            if (!convert_preproc_tokens(preproc_res.toks,
                                        &type_info,
                                        &preproc_err)) {
                print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
                free_preproc_err(&preproc_err);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }

            struct parser_err parser_err = create_parser_err();
            struct translation_unit tl = parse_tokens(preproc_res.toks,
                                                      &parser_err);
            if (parser_err.type != PARSER_ERR_NONE) {
                print_parser_err(stderr, &preproc_res.file_info, &parser_err);
                free_parser_err(&parser_err);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }

            const char* suffix = args.action == ARG_ACTION_OUTPUT_BIN
                                     ? ".binast"
                                     : ".ast";
            struct str out_filename_str = args.output_file == NULL
                                              ? get_out_filename(filename,
                                                                 suffix)
                                              : create_null_str();
            const char* out_filename = str_is_valid(&out_filename_str)
                                           ? str_get_data(&out_filename_str)
                                           : args.output_file;
            FILE* outfile = fopen(out_filename, "wb");
            if (!outfile) {
                fprintf(stderr,
                        "Failed to open output file %s\n",
                        out_filename);
                free_str(&out_filename_str);
                free_translation_unit(&tl);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }

            const bool success = args.action == ARG_ACTION_OUTPUT_BIN
                                     ? serialize_ast(&tl,
                                                     &preproc_res.file_info,
                                                     outfile)
                                     : dump_ast(&tl,
                                                &preproc_res.file_info,
                                                outfile);
            if (!success) {
                fprintf(stderr,
                        "Failed to write ast to file %s\n",
                        out_filename);

                if (fclose(outfile) != 0) {
                    fprintf(stderr,
                            "Failed to close output file %s\n",
                            out_filename);
                }

                free_str(&out_filename_str);
                free_translation_unit(&tl);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }

            fflush(outfile);
            if (fclose(outfile) != 0) {
                fprintf(stderr,
                        "Failed to close output file %s\n",
                        out_filename);
                free_str(&out_filename_str);
                free_translation_unit(&tl);
                free_preproc_res(&preproc_res);
                return EXIT_FAILURE;
            }
            free_str(&out_filename_str);

            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
        }
    }
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

static struct str get_out_filename(const char* origin_file,
                                   const char* suffix) {
    const char* filename_only = strip_file_location(origin_file);
    return str_concat(strlen(filename_only),
                      filename_only,
                      strlen(suffix),
                      suffix);
}

