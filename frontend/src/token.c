#include "frontend/token.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct token create_token(enum token_type type,
                          char* spelling,
                          struct file_loc file_loc,
                          const char* filename) {
    assert(filename);
    if (get_spelling(type) == NULL) {
        assert(spelling);
    } else {
        assert(spelling == NULL);
    }

    return (struct token){
        .type = type,
        .spelling = spelling,
        .loc = {
            .file = alloc_string_copy(filename),
            .file_loc = file_loc,
        },
    };
}

struct token create_token_copy(enum token_type type,
                               const char* spelling,
                               struct file_loc file_loc,
                               const char* filename) {
    assert(spelling);
    return create_token(type, alloc_string_copy(spelling), file_loc, filename);
}

void free_source_loc(struct source_loc* loc) {
    assert(loc);
    free(loc->file);
}

void free_token(struct token* t) {
    assert(t);
    free(t->spelling);
    free_source_loc(&t->loc);
}

