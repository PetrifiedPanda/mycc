#include "ast/string_literal.h"

#include <stdlib.h>

StringLiteral create_string_literal(char* spelling) {
    return (StringLiteral){spelling};
}

void free_string_literal(StringLiteral* l) {
    free(l->spelling);
}
