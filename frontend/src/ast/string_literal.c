#include "ast/string_literal.h"

#include <stdlib.h>

void free_string_literal(struct string_literal* l) {
    free(l->spelling);
}
