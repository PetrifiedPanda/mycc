#include "ast/identifier.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

void create_identifier_inplace(struct identifier* res, char* spelling) {
    assert(res);
    res->spelling = spelling;
}

struct identifier* create_identifier(char* spelling) {
    struct identifier* res = xmalloc(sizeof(struct identifier));
    create_identifier_inplace(res, spelling);
    return res;
}

void free_identifier_children(struct identifier* i) {
    free(i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    free(i);
}

