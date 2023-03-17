#include "frontend/ast/declaration/atomic_type_spec.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!parser_accept(s, TOKEN_ATOMIC)) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    struct type_name* type_name = parse_type_name(s);
    if (!type_name) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        free_type_name(type_name);
        return NULL;
    }

    struct atomic_type_spec* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->type_name = type_name;
    return res;
}

void free_atomic_type_spec(struct atomic_type_spec* s) {
    free_type_name(s->type_name);
    mycc_free(s);
}

