#include "frontend/parser/parser_util.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_err.h"

void expected_token_error(struct parser_state* s, enum token_type expected) {
    expected_tokens_error(s, &expected, 1);
}

void expected_tokens_error(struct parser_state* s,
                           const enum token_type* expected,
                           size_t num_expected) {
    assert(expected);
    enum { MAX_NUM_EXPECTED = sizeof s->err->expected / sizeof *s->err->expected};
    assert(num_expected <= MAX_NUM_EXPECTED);
    assert(num_expected >= 1);

    set_parser_err(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it->loc);

    const size_t bytes = sizeof *s->err->expected * num_expected;
    memcpy(s->err->expected, expected, bytes);
    s->err->num_expected = num_expected;
    s->err->got = s->it->type;
}

bool is_type_spec_token(const struct parser_state* s,
                               const struct token* token) {
    switch (token->type) {
        case VOID:
        case CHAR:
        case SHORT:
        case INT:
        case LONG:
        case FLOAT:
        case DOUBLE:
        case SIGNED:
        case UNSIGNED:
        case BOOL:
        case COMPLEX:
        case IMAGINARY:
        case ATOMIC:
        case STRUCT:
        case UNION:
        case ENUM:
            return true;
        case IDENTIFIER:
            return is_typedef_name(s, &token->spelling);
        default:
            return false;
    }
}

bool next_is_type_name(const struct parser_state* s) {
    assert(s->it->type != INVALID);
    const struct token* next = s->it + 1;
    return is_type_spec_token(s, next) || is_type_qual(next->type)
           || (next->type == IDENTIFIER && is_typedef_name(s, &next->spelling));
}

bool is_type_spec(const struct parser_state* s) {
    return is_type_spec_token(s, s->it);
}

static bool is_declaration_spec(const struct parser_state* s) {
    return is_storage_class_spec(s->it->type) || is_type_spec(s)
           || is_type_qual(s->it->type) || is_func_spec(s->it->type)
           || s->it->type == ALIGNAS;
}

bool is_declaration(const struct parser_state* s) {
    return is_declaration_spec(s) || s->it->type == STATIC_ASSERT;
}

