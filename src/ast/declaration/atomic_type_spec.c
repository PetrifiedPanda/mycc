#include "ast/declaration/atomic_type_spec.h"

#include "util/mem.h"

#include "parser/parser_util.h"

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s) {
    if (!accept(s, ATOMIC)) {
        return NULL;
    }

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct type_name* type_name = parse_type_name(s);
    if (!type_name) {
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_type_name(type_name);
        return NULL;
    }

    struct atomic_type_spec* res = xmalloc(sizeof(struct atomic_type_spec));
    res->type_name = type_name;
    return res;
}

void free_atomic_type_spec(struct atomic_type_spec* s) {
    free_type_name(s->type_name);
    free(s);
}

