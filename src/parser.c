#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"

typedef struct {
    const Token* it;
} ParserState;

void expected_token_error(TokenType expected, const Token* got, const char* filename) {
    set_error(ERR_PARSER, filename, got->source_loc, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got->type));
}

bool accept(ParserState* s, TokenType expected, const char* filename) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it, filename);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(ParserState* s) {
    ++s->it;
}
