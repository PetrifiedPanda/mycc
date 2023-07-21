#include "frontend/ast/GenericSel.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/TypeName.h"
#include "frontend/ast/AssignExpr.h"

static bool parse_generic_assoc_inplace(ParserState* s, GenericAssoc* res) {
    assert(res);

    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    if (ParserState_curr_kind(s) == TOKEN_DEFAULT) {
        ParserState_accept_it(s);
        res->type_name = NULL;
    } else {
        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            return false;
        }
    }

    if (!ParserState_accept(s, TOKEN_COLON)) {
        goto fail;
    }

    res->assign = parse_assign_expr(s);
    if (!res->assign) {
        goto fail;
    }

    return true;
fail:
    if (res->type_name) {
        TypeName_free(res->type_name);
    }
    return false;
}

void GenericAssoc_free_children(GenericAssoc* a) {
    if (a->type_name) {
        TypeName_free(a->type_name);
    }
    AssignExpr_free(a->assign);
}

static bool parse_generic_assoc_list(ParserState* s, GenericAssocList* res) {
    uint32_t alloc_len = 1;
    *res = (GenericAssocList){
        .info = AstNodeInfo_create(ParserState_curr_loc(s)),
        .len = 1,
        .assocs = mycc_alloc(sizeof *res->assocs * alloc_len),
    };

    if (!parse_generic_assoc_inplace(s, &res->assocs[0])) {
        mycc_free(res->assocs);
        return false;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assocs,
                            &alloc_len,
                            sizeof *res->assocs);
        }

        if (!parse_generic_assoc_inplace(s, &res->assocs[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->assocs = mycc_realloc(res->assocs, sizeof *res->assocs * res->len);
    return res;

fail:
    GenericAssocList_free(res);
    return false;
}

void GenericAssocList_free(GenericAssocList* l) {
    for (uint32_t i = 0; i < l->len; ++i) {
        GenericAssoc_free_children(&l->assocs[i]);
    }
    mycc_free(l->assocs);
}

bool parse_generic_sel_inplace(ParserState* s, GenericSel* res) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    ParserState_accept_it(s);

    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return false;
    }

    res->assign = parse_assign_expr(s);
    if (!res->assign) {
        return false;
    }

    if (!ParserState_accept(s, TOKEN_COMMA)) {
        goto fail;
    }

    if (!parse_generic_assoc_list(s, &res->assocs)) {
        goto fail;
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        GenericAssocList_free(&res->assocs);
        goto fail;
    }

    return true;
fail:
    AssignExpr_free(res->assign);
    return false;
}

void GenericSel_free_children(GenericSel* s) {
    AssignExpr_free(s->assign);
    GenericAssocList_free(&s->assocs);
}
