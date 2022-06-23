#include "ast/string_literal.h"

#include <stdlib.h>

#include "ast/ast_visitor.h"

void free_string_literal(struct string_literal* l) {
    free(l->spelling);
}

bool visit_string_literal(struct ast_visitor* visitor, struct string_literal* l) {
    return visitor->visit_string_literal(visitor, l);
}
