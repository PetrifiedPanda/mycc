#include "ast/identifier.h"

#include <stdlib.h>

#include "util.h"

struct identifier* create_identifier(char* spelling) {
    struct identifier* res = xmalloc(sizeof(struct identifier));
    res->spelling = spelling;
    return res;
}

void free_identifier_children(struct identifier* i) {
    free(i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    free(i);
}

