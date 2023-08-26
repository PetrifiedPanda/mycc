#include "frontend/ast/declaration/AbsDeclarator.h"

#include "util/mem.h"

#include "frontend/ast/declaration/Pointer.h"
#include "frontend/ast/declaration/DirectAbsDeclarator.h"

AbsDeclarator* parse_abs_declarator(ParserState* s) {
    AbsDeclarator* res = mycc_alloc(sizeof *res);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            mycc_free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        res->direct_abs_decl = parse_direct_abs_declarator(s);
        if (!res->direct_abs_decl) {
            if (res->ptr) {
                Pointer_free(res->ptr);
            }
            mycc_free(res);
            return NULL;
        }
    } else {
        res->direct_abs_decl = NULL;
    }

    if (res->direct_abs_decl == NULL && res->ptr == NULL) {
        ParserErr_set(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it);
        mycc_free(res);
        return NULL;
    }

    return res;
}

static void AbsDeclarator_free_children(AbsDeclarator* d) {
    if (d->ptr) {
        Pointer_free(d->ptr);
    }
    if (d->direct_abs_decl) {
        DirectAbsDeclarator_free(d->direct_abs_decl);
    }
}

void AbsDeclarator_free(AbsDeclarator* d) {
    AbsDeclarator_free_children(d);
    mycc_free(d);
}
