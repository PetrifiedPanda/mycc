#include "ast/declaration/type_quals.h"

#include <assert.h>

#include "parser/parser_util.h"

struct type_quals create_type_quals(void) {
    return (struct type_quals){
        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_atomic = false,
    };
}

void update_type_quals(struct parser_state* s, struct type_quals* quals) {
    assert(is_type_qual(s->it->type));

    switch (s->it->type) {
        case CONST:
            quals->is_const = true;
            break;
        case RESTRICT:
            quals->is_restrict = true;
            break;
        case VOLATILE:
            quals->is_volatile = true;
            break;
        case ATOMIC:
            quals->is_atomic = true;
            break;
        default:
            assert(false);
    }
    accept_it(s);
}

struct type_quals parse_type_qual_list(struct parser_state* s) {
    struct type_quals res = create_type_quals();

    if (!is_type_qual(s->it->type)) {
        enum token_type expected[] = {CONST, RESTRICT, VOLATILE, ATOMIC};

        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
        return create_type_quals();
    }

    while (is_type_qual(s->it->type)) {
        update_type_quals(s, &res);
    }

    return res;
}

bool is_valid_type_quals(const struct type_quals* q) {
    return q->is_const || q->is_volatile || q->is_restrict || q->is_atomic;
}
