#include <stdio.h>

#include "error.h"

#include "preproc/preproc.h"
#include "parser/parser.h"

static void check_error(void);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];

        struct token* tokens = preproc(filename);
        check_error();
        convert_preproc_tokens(tokens);
        
        struct parser_err err = create_parser_err();
        struct translation_unit tl = parse_tokens(tokens, &err);
        if (err.type != PARSER_ERR_NONE) {
            print_parser_err(&err);
            free_parser_err(&err);
            return EXIT_FAILURE;
        }

        printf("Finished parsing %s successfully\n", filename);

        free_translation_unit(&tl);
        free_tokens(tokens);
    }
}

static void check_error(void) {
    if (get_last_error() != ERR_NONE) {
        fprintf(stderr, "%s\n", get_error_string());
        exit(EXIT_FAILURE);
    }
}

