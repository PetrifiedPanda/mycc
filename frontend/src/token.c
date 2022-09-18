#include "frontend/token.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct token create_token(enum token_type type,
                          char* spelling,
                          struct file_loc file_loc,
                          size_t file_idx) {
    assert(file_idx != (size_t)-1);
    if (get_spelling(type) == NULL) {
        assert(spelling);
    } else {
        assert(spelling == NULL);
    }

    return (struct token){
        .type = type,
        .spelling = spelling,
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct token create_token_copy(enum token_type type,
                               const char* spelling,
                               struct file_loc file_loc,
                               size_t file_idx) {
    assert(spelling);
    return create_token(type, alloc_string_copy(spelling), file_loc, file_idx);
}

char* take_spelling(struct token* t) {
    assert(t->spelling);
    char* spelling = t->spelling;
    t->spelling = NULL;
    return spelling;
}

void free_token(struct token* t) {
    assert(t);
    if (t->type != I_CONSTANT && t->type != F_CONSTANT) {
        free(t->spelling);
    }
}

