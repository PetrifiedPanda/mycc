#include "ast/abs_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct abs_declarator* parse_abs_declarator(struct parser_state* s) {
    struct abs_declarator* res = xmalloc(sizeof(struct abs_declarator));
    if (s->it->type == ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    res->direct_abs_decl = parse_direct_abs_declarator(s);
    if (!res->direct_abs_decl) {
        if (res->ptr) {
            free_pointer(res->ptr);
        }
        free(res);
        return NULL;
    }

    return res;
}

static void free_children(struct abs_declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abs_declarator(d->direct_abs_decl);
    }
}

void free_abs_declarator(struct abs_declarator* d) {
    free_children(d);
    free(d);
}

