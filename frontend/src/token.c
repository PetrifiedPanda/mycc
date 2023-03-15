#include "frontend/token.h"

#include <assert.h>

#include "util/mem.h"

struct token create_token(enum token_kind kind,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx) {
    assert(spelling);
    assert(file_idx != (size_t)-1);
    if (get_spelling(kind) == NULL) {
        assert(str_is_valid(spelling));
    } else {
        assert(!str_is_valid(spelling));
    }

    return (struct token){
        .kind = kind,
        .spelling = *spelling,
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct token create_token_copy(enum token_kind kind,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx) {
    assert(spelling);
    assert(str_is_valid(spelling));

    return (struct token){
        .kind = kind,
        .spelling = str_copy(spelling),
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct str take_spelling(struct token* t) {
    assert(str_is_valid(&t->spelling));
    struct str spelling = str_take(&t->spelling);
    return spelling;
}

void free_token(struct token* t) {
    assert(t);
    if (t->kind != I_CONSTANT && t->kind != F_CONSTANT) {
        free_str(&t->spelling);
    }
}

