#include <stdio.h>
#include <string.h>

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "frontend/ast/ast_dumper.h"

#include "util/str.h"

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

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    const bool is_windows = true;
#else
    const bool is_windows = false;
#endif

    const struct arch_type_info type_info = get_arch_type_info(ARCH_X86_64,
                                                               is_windows);

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];

        struct preproc_err preproc_err = create_preproc_err();
        struct preproc_res preproc_res = preproc(filename, &preproc_err);
        if (preproc_err.type != PREPROC_ERR_NONE) {
            print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
            free_preproc_err(&preproc_err);
            return EXIT_FAILURE;
        }
        if (!convert_preproc_tokens(preproc_res.toks,
                                    &type_info.int_info,
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

        const char* filename_only = strip_file_location(filename);

        const char suffix_str[] = ".ast";
        const struct str out_filename_str = str_concat(strlen(filename_only),
                                                       filename_only,
                                                       sizeof suffix_str
                                                           / sizeof *suffix_str,
                                                       suffix_str);
        const char* out_filename = str_get_const_data(&out_filename_str);
        FILE* outfile = fopen(out_filename, "w");
        if (!outfile) {
            fprintf(stderr, "Failed to open output file %s\n", out_filename);
            free_str(&out_filename_str);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        if (!dump_ast(&tl, &preproc_res.file_info, outfile)) {
            fprintf(stderr, "Failed to write ast to file %s\n", out_filename);

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
            fprintf(stderr, "Failed to close output file %s\n", out_filename);
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

