#include "frontend/parser/parser_util.h"

#include <assert.h>

#include "frontend/parser/ParserErr.h"

void expected_token_error(ParserState* s, TokenKind expected) {
    ParserErr_set(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it);
    s->err->expected_tokens_err = ExpectedTokensErr_create_single_token(
        ParserState_curr_kind(s),
        expected);
}

void expected_tokens_error(ParserState* s,
                           const TokenKind* expected,
                           uint32_t num_expected) {
    assert(expected);
    ParserErr_set(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it);
    s->err->expected_tokens_err = ExpectedTokensErr_create(
        ParserState_curr_kind(s),
        expected,
        num_expected);
}

bool is_storage_class_spec(TokenKind k) {
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

bool is_type_qual(TokenKind k) {
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

bool is_func_spec(TokenKind k) {
    return k == TOKEN_INLINE || k == TOKEN_NORETURN;
}

bool is_type_spec_token(const ParserState* s, TokenKind kind, Str spell) {
    switch (kind) {
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
            return ParserState_is_typedef(s, spell);
        default:
            return false;
    }
}

static bool is_type_spec_helper(const ParserState* s,
                                TokenKind (*get_kind)(const ParserState*),
                                Str (*get_spell)(const ParserState*)) {
    TokenKind kind = get_kind(s);
    switch (kind) {
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
            return ParserState_is_typedef(s, get_spell(s));
        default:
            return false;
    }
}

bool next_is_type_spec(const ParserState* s) {
    const SourceLoc loc = s->_arr.locs[s->it + 1];
    const uint32_t val_idx = s->_arr.val_indices[s->it + 1];
    mycc_printf("Got loc {u32}:{u32}, idx: {u32}, token idx: {u32}\n", loc.file_loc.line, loc.file_loc.index, val_idx, s->it + 1);
    return is_type_spec_helper(s,
                               ParserState_next_token_kind,
                               ParserState_next_token_spell);
}

bool next_is_type_name(const ParserState* s) {
    assert(ParserState_curr_kind(s) != TOKEN_INVALID);
    TokenKind kind = ParserState_next_token_kind(s);
    return next_is_type_spec(s) || is_type_qual(kind);
}

bool is_type_spec(const ParserState* s) {
    return is_type_spec_helper(s,
                               ParserState_curr_kind,
                               ParserState_curr_spell);
}

bool is_declaration_spec(const ParserState* s) {
    const TokenKind kind = ParserState_curr_kind(s);
    return is_storage_class_spec(kind) || is_type_spec(s) || is_type_qual(kind)
           || is_func_spec(kind) || kind == TOKEN_ALIGNAS;
}

bool is_declaration(const ParserState* s) {
    return is_declaration_spec(s)
           || ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT;
}

