#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "error.h"

static char* alloc_string_copy(const char* str) {
    assert(str);
    char* res = malloc(sizeof(char) * (strlen(str) + 1));
    if (res) {
        strcpy(res, str);
    }
    return res;
}

bool create_token(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename) {
    assert(t);
    assert(spelling);
    assert(filename);
    t->spelling = alloc_string_copy(spelling);
    t->file = alloc_string_copy(filename);
    if (!t->file || !t->spelling) {
        set_error(ERR_ALLOC_FAIL, filename, loc, "Failed to allocate spelling or filename for token");
        return false;       
    }
    t->type = type;
    t->source_loc = loc;

    return true;
}

void create_token_move(Token* t, TokenType type, char* spelling, SourceLocation loc) {
    assert(t);
    t->spelling = spelling;
    t->type = type;
    t->source_loc = loc;
}

void free_token(Token* t) {
    assert(t);
    free(t->spelling);
}
