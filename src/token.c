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

static void set_alloc_error() {
    set_error(ERR_ALLOC_FAIL, "Failed to allocate token contents");
}

bool create_token(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename) {
    assert(t); 
    assert(filename);
    if (get_spelling(type) == NULL) {
        assert(spelling);
        t->spelling = alloc_string_copy(spelling);
        if (!t->spelling) {
            goto fail;
        }
    } else {
        t->spelling = NULL;
    }
    t->file = alloc_string_copy(filename);
    if (!t->file) {
        goto fail;      
    }
    t->type = type;
    t->source_loc = loc;

    return true;

fail:
    free(t->spelling);
    set_alloc_error();
    return false;
}

bool create_token_move(Token* t, TokenType type, char* spelling, SourceLocation loc, const char* filename) {
    assert(t);
    t->spelling = spelling;
    t->file = alloc_string_copy(filename);
    if (!t->file) {
       set_alloc_error();
       return false;
    }
    t->type = type;
    t->source_loc = loc;

    return true;
}

void free_token(Token* t) {
    assert(t);
    free(t->spelling);
    free(t->file);
}
