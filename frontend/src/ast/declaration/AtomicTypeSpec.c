#include "frontend/ast/declaration/AtomicTypeSpec.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

AtomicTypeSpec* parse_atomic_type_spec(ParserState* s) {
    const SourceLoc loc = s->it->loc;
    if (!parser_accept(s, TOKEN_ATOMIC)) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    TypeName* type_name = parse_type_name(s);
    if (!type_name) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        free_type_name(type_name);
        return NULL;
    }

    AtomicTypeSpec* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->type_name = type_name;
    return res;
}

void free_atomic_type_spec(AtomicTypeSpec* s) {
    free_type_name(s->type_name);
    mycc_free(s);
}

