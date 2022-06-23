#include "ast/declaration/align_spec.h"

#include <assert.h>

#include "util/mem.h"

#include "parser/parser_util.h"

static void free_align_spec(struct align_spec* s);

bool parse_align_spec_inplace(struct parser_state* s, struct align_spec* res) {
    assert(res);
    if (!(accept(s, ALIGNAS) && accept(s, LBRACKET))) {
        return false;
    }

    // TODO: this condition may be wrong
    if (is_type_spec(s) || is_type_qual(s->it->type)) {
        res->is_type_name = true;
        res->type_name = parse_type_name(s);

        if (!res->type_name) {
            return false;
        }
    } else {
        res->is_type_name = false;
        res->const_expr = parse_const_expr(s);
        if (!res->const_expr) {
            return false;
        }
    }

    if (!accept(s, RBRACKET)) {
        free_align_spec(res);
        return false;
    }

    return true;
}

void free_align_spec_children(struct align_spec* s) {
    if (s->is_type_name) {
        free_type_name(s->type_name);
    } else {
        free_const_expr(s->const_expr);
    }
}

static void free_align_spec(struct align_spec* s) {
    free_align_spec_children(s);
    free(s);
}

