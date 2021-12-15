#include "ast/string_literal.h"

#include <stdlib.h>
#include <assert.h>

struct string_literal create_string_literal(char* spelling) {
    assert(spelling);
    return (struct string_literal){.is_func = false, .spelling = spelling};
}

struct string_literal create_func_name() {
    return (struct string_literal){.is_func = true, .spelling = NULL};
}

void free_string_literal(struct string_literal* l) {
    free(l->spelling);
}
