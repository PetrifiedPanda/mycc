#include "ast/string_literal.h"

#include <stdlib.h>

struct string_literal create_string_literal(char* spelling) {
    return (struct string_literal){spelling};
}

void free_string_literal(struct string_literal* l) {
    free(l->spelling);
}
