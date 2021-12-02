#include "ast/identifier.h"

#include <stdlib.h>

#include "util.h"

Identifier* create_identifier(char* spelling) {
    Identifier* res = xmalloc(sizeof(Identifier));
    res->spelling = spelling;
    return res;
}

void free_identifier(Identifier* i) {
    free(i->spelling);
}

