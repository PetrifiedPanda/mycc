#include "frontend/ast/declaration/AtomicTypeSpec.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/TypeName.h"

AtomicTypeSpec* parse_atomic_type_spec(ParserState* s) {
    const SourceLoc loc = ParserState_curr_loc(s);
    if (!ParserState_accept(s, TOKEN_ATOMIC)) {
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    TypeName* type_name = parse_type_name(s);
    if (!type_name) {
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        TypeName_free(type_name);
        return NULL;
    }

    AtomicTypeSpec* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->type_name = type_name;
    return res;
}

void AtomicTypeSpec_free(AtomicTypeSpec* s) {
    TypeName_free(s->type_name);
    mycc_free(s);
}

