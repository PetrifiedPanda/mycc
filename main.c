#include <stdio.h>

#include "preproc/preproc.h"
#include "parser/parser.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];
        
        struct preproc_err preproc_err = create_preproc_err();
        struct token* tokens = preproc(filename, &preproc_err);
        if (preproc_err.type != PREPROC_ERR_NONE) {
            print_preproc_err(&preproc_err);
            free_preproc_err(&preproc_err);
            return EXIT_FAILURE;
        }
        convert_preproc_tokens(tokens);
        
        struct parser_err parser_err = create_parser_err();
        struct translation_unit tl = parse_tokens(tokens, &parser_err);
        if (parser_err.type != PARSER_ERR_NONE) {
            print_parser_err(&parser_err);
            free_parser_err(&parser_err);
            return EXIT_FAILURE;
        }

        printf("Finished parsing %s successfully\n", filename);

        free_translation_unit(&tl);
        free_tokens(tokens);
    }
}

