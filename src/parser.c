#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"

typedef struct {
    const Token* it;
} ParserState;

static void expected_token_error(TokenType expected, const Token* got) {
    set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got->type));
}

static bool accept(ParserState* s, TokenType expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(ParserState* s) {
    ++s->it;
}
