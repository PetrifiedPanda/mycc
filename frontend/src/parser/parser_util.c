#include "frontend/parser/parser_util.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_err.h"

void expected_token_error(struct parser_state* s, enum token_kind expected) {
    set_parser_err(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it->loc);
    s->err->expected_tokens_err = create_expected_token_err(s->it->kind, expected);
}

void expected_tokens_error(struct parser_state* s,
                           const enum token_kind* expected,
                           size_t num_expected) {
    assert(expected);
    set_parser_err(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it->loc);
    s->err->expected_tokens_err = create_expected_tokens_err(s->it->kind,
                                                             expected,
                                                             num_expected);
}

bool is_storage_class_spec(enum token_kind k) {
    switch (k) {
        case TOKEN_TYPEDEF:
        case TOKEN_EXTERN:
        case TOKEN_STATIC:
        case TOKEN_THREAD_LOCAL:
        case TOKEN_AUTO:
        case TOKEN_REGISTER:
            return true;
        default:
            return false;
    }
}

bool is_type_qual(enum token_kind k) {
    switch (k) {
        case TOKEN_CONST:
        case TOKEN_RESTRICT:
        case TOKEN_VOLATILE:
        case TOKEN_ATOMIC:
            return true;

        default:
            return false;
    }
}

bool is_func_spec(enum token_kind k) {
    return k == TOKEN_INLINE || k == TOKEN_NORETURN;
}

bool is_type_spec_token(const struct parser_state* s,
                        const struct token* token) {
    switch (token->kind) {
        case TOKEN_VOID:
        case TOKEN_CHAR:
        case TOKEN_SHORT:
        case TOKEN_INT:
        case TOKEN_LONG:
        case TOKEN_FLOAT:
        case TOKEN_DOUBLE:
        case TOKEN_SIGNED:
        case TOKEN_UNSIGNED:
        case TOKEN_BOOL:
        case TOKEN_COMPLEX:
        case TOKEN_IMAGINARY:
        case TOKEN_ATOMIC:
        case TOKEN_STRUCT:
        case TOKEN_UNION:
        case TOKEN_ENUM:
            return true;
        case TOKEN_IDENTIFIER:
            return is_typedef_name(s, &token->spelling);
        default:
            return false;
    }
}

bool next_is_type_name(const struct parser_state* s) {
    assert(s->it->kind != TOKEN_INVALID);
    const struct token* next = s->it + 1;
    return is_type_spec_token(s, next) || is_type_qual(next->kind)
           || (next->kind == TOKEN_IDENTIFIER && is_typedef_name(s, &next->spelling));
}

bool is_type_spec(const struct parser_state* s) {
    return is_type_spec_token(s, s->it);
}

static bool is_declaration_spec(const struct parser_state* s) {
    return is_storage_class_spec(s->it->kind) || is_type_spec(s)
           || is_type_qual(s->it->kind) || is_func_spec(s->it->kind)
           || s->it->kind == TOKEN_ALIGNAS;
}

bool is_declaration(const struct parser_state* s) {
    return is_declaration_spec(s) || s->it->kind == TOKEN_STATIC_ASSERT;
}

