#include <stdio.h>
#include <assert.h>

#include "tokenizer.h"
#include "error.h"
#include "util.h"

#include "parser/parser.h"

static void check_error();

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];

    char* file_contents = read_file(filename);
    assert(file_contents);

    struct token* tokens = tokenize(file_contents, filename);
    check_error();
    
    struct translation_unit tl = parse_tokens(tokens);
    check_error();
    
    printf("Finished tokenization successfully\n");

    free_translation_unit(&tl);
    free_tokenizer_result(tokens);
    free(file_contents);
}

static void check_error() {
    if (get_last_error() != ERR_NONE) {
        fprintf(stderr, "%s\n", get_error_string());
        exit(EXIT_FAILURE);
    }
}

