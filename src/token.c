#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"

void init_token(Token* t, TokenType type, char* spelling, SourceLocation loc, const char* filename) {
    assert(t);
    assert(filename);

    if (get_spelling(type) == NULL) {
        assert(spelling);
        t->spelling = spelling;
    } else {
        assert(spelling == NULL);
        t->spelling = NULL;
    }

    t->file = alloc_string_copy(filename);
    t->type = type;
    t->source_loc = loc;
}

void init_token_copy(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename) {
    init_token(t, type, alloc_string_copy(spelling), loc, filename);
}

void free_token(Token* t) {
    assert(t);
    free(t->spelling);
    free(t->file);
}

