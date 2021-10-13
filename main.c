#include <stdio.h>

#include "tokenizer.h"
#include "error.h"
#include "ast/translation_unit.h"

#include "regex.h"

int main() {
    const char* code = 
        "struct typedeftest /* This is a comment \n"
        "that goes over\n"
        "multiple lines\n"
        "*/\n"
        "{\n"
        "\tlong int* n;\n"
        "const long double *m;\n"
        "}; // Line comment\n"
        "const char* lstr = \n"
        "L\"Long string literal to check if long strings work\";\n"
        "int n = 0x123213;\n"
        "const char* str = \"Normal string literal\";\n"
        "int arr[1 ? 100 : 1000];\n";
    Token* tokens = tokenize(code);
    if (get_last_error() != ERR_NONE) {
        printf("%s\n", get_error_string());
        clear_last_error();
    }

    for (Token* it = tokens; it->type != INVALID; ++it) {
        printf("Type: %s, Spelling: %s, line: %lu, idx: %lu\n", get_type_str(it->type), it->spelling, it->source_loc.line, it->source_loc.index);

        free_token(it);
    }

    free(tokens);
    printf("Finished\n");
}
