#include "frontend/ast/declaration/AlignSpec.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/AssignExpr.h"
#include "frontend/ast/TypeName.h"

bool parse_align_spec_inplace(ParserState* s, AlignSpec* res) {
    assert(res);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    if (!(ParserState_accept(s, TOKEN_ALIGNAS)
          && ParserState_accept(s, TOKEN_LBRACKET))) {
        return false;
    }

    // TODO: this condition may be wrong
    if (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))) {
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

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        AlignSpec_free_children(res);
        return false;
    }

    return true;
}

void AlignSpec_free_children(AlignSpec* s) {
    if (s->is_type_name) {
        TypeName_free(s->type_name);
    } else {
        ConstExpr_free(s->const_expr);
    }
}

