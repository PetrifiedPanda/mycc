#include <stdio.h>

#include "tokenizer.h"
#include "parser/parser.h"
#include "error.h"

int main() {
    const char* code = 
        "typedef struct typedeftest /* This is a comment \n"
        "that goes over\n"
        "multiple lines\n"
        "*/\n"
        "{\n"
        "\tlong int* n;\n"
        "const long double *m;\n"
        "} Typedeftest; // Line comment\n"
        "const char* lstr = \n"
        "L\"Long string literal to check if long strings work\";\n"
        "int n = 0x123213 + 132 << 32 >> 0x123 - 0123 / 12;\n"
        "const char* str = \"Normal string literal\";\n"
        "int arr[1 ? 100 : 1000];\n";
    printf("Tokenizing...\n");
    struct token* tokens = tokenize(code, "this_is_not_an_actual_file.c");
    if (get_last_error() != ERR_NONE) {
        fprintf(stderr, "%s\n", get_error_string());
        clear_last_error();
    } else {
        printf("Tokenizer finished successfully\n");
    }

    for (const struct token* it = tokens; it->type != INVALID; ++it) {
        printf("Type: %s, Spelling: %s, line: %zu, idx: %zu\n", get_type_str(it->type), it->spelling, it->source_loc.line, it->source_loc.index);
    }

    free_tokenizer_result(tokens);
    printf("Finished\n");
}

