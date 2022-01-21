#include "parser/parser_util.h"

#include <assert.h>

#include "error.h"

void expected_token_error(enum token_type expected, const struct token* got) {
    assert(got);
    expected_tokens_error(&expected, 1, got);
}

void expected_tokens_error(const enum token_type* expected, size_t num_expected, const struct token* got) {
    assert(expected);
    assert(got);

    bool not_eof = got->type != INVALID;

    if (not_eof) {
        set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s", get_type_str(expected[0]));
    } else {
        set_error(ERR_PARSER, "Expected token of type %s", get_type_str(expected[0]));
    }

    for (size_t i = 1; i < num_expected; ++i) {
        append_error_msg(", %s", get_type_str(expected[i]));
    }

    if (not_eof) {
        append_error_msg(" but got token of type %s", get_type_str(got->type));
    } else {
        append_error_msg(" but got to end of file");
    }
}

bool next_is_type_name(const struct parser_state* s) {
    assert(s->it->type != INVALID);
    struct token* next = s->it + 1;
    return is_keyword_type_spec(next->type) || is_type_qual(next->type) || (next->type == IDENTIFIER && is_typedef_name(s, next->spelling));
}

bool is_type_spec(const struct parser_state* s) {
    switch (s->it->type) {
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
            return is_typedef_name(s, s->it->spelling);
        default:
            return false;
    }
}

bool is_declaration(const struct parser_state* s) {
    return  is_storage_class_spec(s->it->type) ||
            is_type_spec(s) ||
            is_type_qual(s->it->type) ||
            is_func_spec(s->it->type) ||
            s->it->type == ALIGNAS ||
            s->it->type == STATIC_ASSERT;
}

char* take_spelling(struct token* t) {
    char* spelling = t->spelling;
    t->spelling = NULL;
    return spelling;
}
