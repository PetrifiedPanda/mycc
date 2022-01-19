#include "parser/parser_state.h"

#include <assert.h>

#include "parser/parser_util.h"

bool accept(struct parser_state* s, enum token_type expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

void accept_it(struct parser_state* s) {
    assert(s->it->type != INVALID);
    ++s->it;
}

void register_enum_constant(struct parser_state* s, const char* spell) {
    (void)s;
    (void)spell;
    // TODO:
}

bool is_enum_constant(const struct parser_state* s, const char* spell) {
    (void)s;
    (void)spell;
    // TODO:
    return false;
}

bool is_typedef_name(const struct parser_state* s, const char* spell) {
    (void)s;
    (void)spell;
    // TODO:
    return false;
}
