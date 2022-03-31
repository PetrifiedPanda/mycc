#include "token.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct token create_token(enum token_type type,
                          char* spelling,
                          struct source_location loc,
                          const char* filename) {
    assert(filename);
    if (get_spelling(type) == NULL) {
        assert(spelling);
    } else {
        assert(spelling == NULL);
    }

    return (struct token) {
        .type = type,
        .spelling = spelling,
        .file = alloc_string_copy(filename),
        .source_loc = loc,
    };
}

struct token create_token_copy(enum token_type type,
                               const char* spelling,
                               struct source_location loc,
                               const char* filename) {
    assert(spelling);
    return create_token(type, alloc_string_copy(spelling), loc, filename); 
}

void free_token(struct token* t) {
    assert(t);
    free(t->spelling);
    free(t->file);
}

