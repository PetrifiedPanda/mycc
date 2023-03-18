#include "frontend/ast/declaration/type_quals.h"

#include <assert.h>

#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

struct type_quals create_type_quals(void) {
    return (struct type_quals){
        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_atomic = false,
    };
}

void update_type_quals(struct parser_state* s, struct type_quals* quals) {
    assert(is_type_qual(s->it->kind));

    switch (s->it->kind) {
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

bool parse_type_qual_list(struct parser_state* s, struct type_quals* res) {
    assert(res);
    *res = create_type_quals();

    if (!is_type_qual(s->it->kind)) {
        static const enum token_kind expected[] = {
            TOKEN_CONST,
            TOKEN_RESTRICT,
            TOKEN_VOLATILE,
            TOKEN_ATOMIC,
        };

        expected_tokens_error(s, expected, ARR_LEN(expected));
        return false;
    }

    while (is_type_qual(s->it->kind)) {
        update_type_quals(s, res);
    }

    return true;
}

