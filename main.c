#include <stdio.h>

#include "error.h"

#include "preproc/preproc.h"
#include "parser/parser.h"

static void check_error();

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];

    struct token* tokens = preproc(filename);
    check_error();
    
    struct translation_unit tl = parse_tokens(tokens);
    check_error();
    
    printf("Finished tokenization successfully\n");

    free_translation_unit(&tl);
    free_tokens(tokens);
}

static void check_error() {
    if (get_last_error() != ERR_NONE) {
        fprintf(stderr, "%s\n", get_error_string());
        exit(EXIT_FAILURE);
    }
}

