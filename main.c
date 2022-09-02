#include <stdio.h>
#include <string.h>

#include "frontend/preproc/preproc.h"

#include "frontend/parser/parser.h"

#include "frontend/ast/ast_dumper.h"

#include "util/mem.h"

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

static char* str_concat(const char* s1, const char* s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    const size_t len = len1 + len2;
    char* res = xmalloc(sizeof(char) * (len + 1));
    
    memcpy(res, s1, len1 * sizeof(char));
    memcpy(res + len1, s2, len2 * sizeof(char));
    res[len] = '\0';
    return res;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];

        struct preproc_err preproc_err = create_preproc_err();
        struct preproc_res preproc_res = preproc(filename, &preproc_err);
        if (preproc_err.type != PREPROC_ERR_NONE) {
            print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
            free_preproc_err(&preproc_err);
            return EXIT_FAILURE;
        }
        if (!convert_preproc_tokens(preproc_res.toks, &preproc_err)) {
            print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
            free_preproc_err(&preproc_err);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        struct parser_err parser_err = create_parser_err();
        struct translation_unit tl = parse_tokens(preproc_res.toks, &parser_err);
        if (parser_err.type != PARSER_ERR_NONE) {
            print_parser_err(stderr, &preproc_res.file_info, &parser_err);
            free_parser_err(&parser_err);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }
        
        const char* filename_only = strip_file_location(filename);

        char* out_filename = str_concat(filename_only, ".ast");

        FILE* outfile = fopen(out_filename, "w");
        if (!outfile) {
            fprintf(stderr, "Failed to open output file %s\n", out_filename);
            free(out_filename);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        if (!dump_ast(&tl, &preproc_res.file_info, outfile)) {
            fprintf(stderr, "Failed to write ast to file %s\n", out_filename);

            if (fclose(outfile) != 0) {
                fprintf(stderr, "Failed to close output file %s\n", out_filename);
            }

            free(out_filename);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        fflush(outfile);
        if (fclose(outfile) != 0) {
            fprintf(stderr, "Failed to close output file %s\n", out_filename);
            free(out_filename);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }
        free(out_filename);

        free_translation_unit(&tl);
        free_preproc_res(&preproc_res);
    }
}

