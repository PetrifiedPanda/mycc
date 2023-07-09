#include "frontend/ast/declaration/InitDeclaratorList.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/ast/declaration/InitDeclarator.h"

static bool parse_init_declarator_list_first_base(
    ParserState* s,
    InitDeclaratorList* res,
    bool (*inplace_parse_func)(ParserState*, InitDeclarator*),
    InitDeclarator* first_decl) {
    assert(first_decl);
    res->len = 1;
    res->decls = first_decl;

    size_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!inplace_parse_func(s, &res->decls[res->len])) {
            InitDeclaratorList_free(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return true;
}

static bool parse_init_declarator_list_base(
    ParserState* s,
    InitDeclaratorList* res,
    bool (*inplace_parse_func)(ParserState*, InitDeclarator*)) {
    InitDeclarator* first_decl = mycc_alloc(sizeof *first_decl);

    if (!inplace_parse_func(s, first_decl)) {
        mycc_free(first_decl);
        return false;
    }

    return parse_init_declarator_list_first_base(s,
                                                 res,
                                                 inplace_parse_func,
                                                 first_decl);
}

bool parse_init_declarator_list_first(ParserState* s,
                                      InitDeclaratorList* res,
                                      InitDeclarator* first_decl) {
    return parse_init_declarator_list_first_base(s,
                                                 res,
                                                 parse_init_declarator_inplace,
                                                 first_decl);
}

bool parse_init_declarator_list(ParserState* s,
                                InitDeclaratorList* res) {
    return parse_init_declarator_list_base(s,
                                           res,
                                           parse_init_declarator_inplace);
}

bool parse_init_declarator_list_typedef_first(
    ParserState* s,
    InitDeclaratorList* res,
    InitDeclarator* first_decl) {
    return parse_init_declarator_list_first_base(
        s,
        res,
        parse_init_declarator_typedef_inplace,
        first_decl);
}

bool parse_init_declarator_list_typedef(ParserState* s,
                                        InitDeclaratorList* res) {
    return parse_init_declarator_list_base(
        s,
        res,
        parse_init_declarator_typedef_inplace);
}

void InitDeclaratorList_free(InitDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        InitDeclarator_free_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}
