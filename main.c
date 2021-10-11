#include <stdio.h>

#include "tokenizer.h"
#include "error.h"
#include "ast/assign_expr.h"

#include "regex.h"

int main() {
    const char* test = "struct typedeftest /* this is a comment \n\n */ {long int* n; const long double *m;}; // Test Comment\n const char* name = \"Hello World\\n\"; int* nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn = L\"Super long string is this 30 characters yet please help\";";
    Token* tokens = tokenize(test);
    if (get_last_error() != NO_ERROR) {
        printf("%s\n", get_error_string());
        clear_last_error();
    }

    for (Token* it = tokens; it->type != INVALID; ++it) {
        printf("Type: %s, Spelling: %s, line: %lu, idx: %lu\n", get_type_str(it->type), it->spelling, it->source_line, it->source_char);

        free_token(it);
    }

    free(tokens);
    printf("Finished\n");
}