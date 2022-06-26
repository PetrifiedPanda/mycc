#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include "parser/parser_state.h"

struct identifier;
struct const_expr;

struct enumerator {
    struct identifier* identifier;
    struct const_expr* enum_val;
};

bool parse_enumerator_inplace(struct parser_state* s, struct enumerator* res);

void free_enumerator_children(struct enumerator* e);

#include "ast/identifier.h"

#include "ast/expr/const_expr.h"

#endif

