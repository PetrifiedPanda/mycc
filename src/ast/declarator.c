#include "ast/declarator.h"

#include <stdlib.h>

#include "util.h"

struct declarator* parse_declarator(struct parser_state* s) {
    struct declarator* res = xmalloc(sizeof(struct declarator));
    if (s->it->type == ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    res->direct_decl = parse_direct_declarator(s);
    if (!res->direct_decl) {
        if (res->ptr) {
            free_pointer(res->ptr);
        }
        free(res);
        return NULL;
    }

    return res;
}

static void free_children(struct declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(struct declarator* d) {
    free_children(d);
    free(d);
}

