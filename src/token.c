#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"

static char* alloc_string_copy(const char* str) {
    assert(str);
    char* res = xmalloc(sizeof(char) * (strlen(str) + 1));
    strcpy(res, str);
    return res;
}

void init_token(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename) {
    assert(t); 
    assert(filename);
    if (get_spelling(type) == NULL) {
        assert(spelling);
        t->spelling = alloc_string_copy(spelling);
    } else {
        t->spelling = NULL;
    }
    t->file = alloc_string_copy(filename);
    t->type = type;
    t->source_loc = loc;
}

void free_token(Token* t) {
    assert(t);
    free(t->spelling);
    free(t->file);
}

