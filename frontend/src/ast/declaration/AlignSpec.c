#include "frontend/ast/declaration/AlignSpec.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static void free_align_spec(AlignSpec* s);

bool parse_align_spec_inplace(ParserState* s, AlignSpec* res) {
    assert(res);
    res->info = create_ast_node_info(s->it->loc);
    if (!(parser_accept(s, TOKEN_ALIGNAS) && parser_accept(s, TOKEN_LBRACKET))) {
        return false;
    }

    // TODO: this condition may be wrong
    if (is_type_spec(s) || is_type_qual(s->it->kind)) {
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

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        free_align_spec(res);
        return false;
    }

    return true;
}

void free_align_spec_children(AlignSpec* s) {
    if (s->is_type_name) {
        free_type_name(s->type_name);
    } else {
        free_const_expr(s->const_expr);
    }
}

static void free_align_spec(AlignSpec* s) {
    free_align_spec_children(s);
    mycc_free(s);
}

