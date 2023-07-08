#include "frontend/ast/declaration/TypeQuals.h"

#include <assert.h>

#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

TypeQuals TypeQuals_create(void) {
    return (TypeQuals){
        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_atomic = false,
    };
}

void update_type_quals(ParserState* s, TypeQuals* quals) {
    const TokenKind kind = ParserState_curr_kind(s);
    assert(is_type_qual(kind));

    switch (kind) {
        case TOKEN_CONST:
            quals->is_const = true;
            break;
        case TOKEN_RESTRICT:
            quals->is_restrict = true;
            break;
        case TOKEN_VOLATILE:
            quals->is_volatile = true;
            break;
        case TOKEN_ATOMIC:
            quals->is_atomic = true;
            break;
        default:
            UNREACHABLE();
    }
    parser_accept_it(s);
}

bool parse_type_qual_list(ParserState* s, TypeQuals* res) {
    assert(res);
    *res = TypeQuals_create();

    if (!is_type_qual(ParserState_curr_kind(s))) {
        static const TokenKind expected[] = {
            TOKEN_CONST,
            TOKEN_RESTRICT,
            TOKEN_VOLATILE,
            TOKEN_ATOMIC,
        };

        expected_tokens_error(s, expected, ARR_LEN(expected));
        return false;
    }

    while (is_type_qual(ParserState_curr_kind(s))) {
        update_type_quals(s, res);
    }

    return true;
}

