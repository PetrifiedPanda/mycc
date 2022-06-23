#include "ast/identifier.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "ast/ast_visitor.h"

void init_identifier(struct identifier* res, char* spelling) {
    assert(res);
    res->spelling = spelling;
}

struct identifier* create_identifier(char* spelling) {
    struct identifier* res = xmalloc(sizeof(struct identifier));
    init_identifier(res, spelling);
    return res;
}

void free_identifier_children(struct identifier* i) {
    free(i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    free(i);
}

bool visit_identifier(struct ast_visitor* visitor, struct identifier* i) {
    return visitor->visit_identifier(visitor, i);
}

