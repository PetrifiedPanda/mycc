#include "ast/declaration/atomic_type_spec.h"

#include "util/mem.h"

#include "parser/parser_util.h"

#include "ast/ast_visitor.h"

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

static bool visit_children(struct ast_visitor* visitor, struct atomic_type_spec* s) {
    return visit_type_name(visitor, s->type_name);
}

bool visit_atomic_type_spec(struct ast_visitor* visitor,
                            struct atomic_type_spec* s) {
    AST_VISITOR_VISIT_TEMPLATE(visitor, s, visit_children, visitor->visit_atomic_type_spec);
}

