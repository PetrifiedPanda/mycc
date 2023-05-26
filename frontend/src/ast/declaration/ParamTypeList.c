#include "frontend/ast/declaration/ParamTypeList.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

typedef struct {
    bool is_abs;
    union {
        Declarator* decl;
        AbsDeclarator* abs_decl;
    };
} AbsDeclOrDecl;

static bool parse_abs_decl_or_decl(ParserState* s, AbsDeclOrDecl* res) {
    assert(res);
    Pointer* ptr;
    if (s->it->kind == TOKEN_ASTERISK) {
        ptr = parse_pointer(s);
        if (!ptr) {
            return false;
        }
    } else {
        ptr = NULL;
    }

    if (s->it->kind == TOKEN_IDENTIFIER) {
        res->is_abs = false;
        res->decl = mycc_alloc(sizeof *res->decl);
        res->decl->ptr = ptr;
        res->decl->direct_decl = parse_direct_declarator(s);
        if (!res->decl->direct_decl) {
            mycc_free(res->decl);
            goto fail;
        }
    } else if (s->it->kind == TOKEN_LINDEX) {
        res->is_abs = true;
        res->abs_decl = mycc_alloc(sizeof *res->abs_decl);
        res->abs_decl->ptr = ptr;
        res->abs_decl->direct_abs_decl = parse_direct_abs_declarator(s);
        if (!res->abs_decl->direct_abs_decl) {
            mycc_free(res->abs_decl);
            goto fail;
        }
    } else if (s->it->kind == TOKEN_LBRACKET) {
        const SourceLoc loc = s->it->loc;
        parser_accept_it(s);
        AbsDeclOrDecl bracket_decl;
        if (!parse_abs_decl_or_decl(s, &bracket_decl)) {
            goto fail;
        }
        res->is_abs = bracket_decl.is_abs;
        if (bracket_decl.is_abs) {
            res->abs_decl = mycc_alloc(sizeof *res->abs_decl);
            res->abs_decl->ptr = ptr;
            res->abs_decl->direct_abs_decl = mycc_alloc(
                sizeof *res->abs_decl->direct_abs_decl);
            struct DirectAbsDeclarator* decl = res->abs_decl->direct_abs_decl;
            decl->info = AstNodeInfo_create(loc);
            decl->bracket_decl = bracket_decl.abs_decl;

            if (!parser_accept(s, TOKEN_RBRACKET)) {
                decl->len = 0;
                decl->following_suffixes = NULL;
                DirectAbsDeclarator_free(decl);
                mycc_free(res->abs_decl);
                goto fail;
            }

            if (!parse_abs_arr_or_func_suffixes(s, decl)) {
                mycc_free(res->abs_decl);
                goto fail;
            }
        } else {
            res->decl = mycc_alloc(sizeof *res->decl);
            res->decl->ptr = ptr;
            res->decl->direct_decl = mycc_alloc(sizeof *res->decl->direct_decl);
            DirectDeclarator* decl = res->decl->direct_decl;
            decl->info = AstNodeInfo_create(loc);
            decl->is_id = false;
            decl->bracket_decl = bracket_decl.decl;

            if (!parser_accept(s, TOKEN_RBRACKET)) {
                decl->len = 0;
                decl->suffixes = NULL;
                DirectDeclarator_free(decl);
                mycc_free(res->decl);
                goto fail;
            }

            if (!parse_arr_or_func_suffixes(s, decl)) {
                mycc_free(res->decl);
                goto fail;
            }
        }
    } else {
        res->is_abs = true;
        if (ptr == NULL) {
            ParserErr_set(s->err,
                           PARSER_ERR_EMPTY_DIRECT_ABS_DECL,
                           s->it->loc);
            return false;
        }
        res->abs_decl = mycc_alloc(sizeof *res->abs_decl);
        res->abs_decl->ptr = ptr;
        res->abs_decl->direct_abs_decl = NULL;
    }

    return true;
fail:
    if (ptr) {
        Pointer_free(ptr);
    }
    return false;
}

static bool parse_param_declaration_inplace(ParserState* s, ParamDeclaration* res) {
    assert(res);

    bool found_typedef = false;
    res->decl_specs = parse_declaration_specs(s, &found_typedef);
    if (!res->decl_specs) {
        return false;
    }

    if (found_typedef) {
        ParserErr_set(s->err, PARSER_ERR_TYPEDEF_PARAM_DECL, s->it->loc);
        DeclarationSpecs_free(res->decl_specs);
        return false;
    }

    if (s->it->kind == TOKEN_COMMA || s->it->kind == TOKEN_RBRACKET) {
        res->kind = PARAM_DECL_NONE;
        res->decl = NULL;
    } else {
        AbsDeclOrDecl abs_decl_or_decl;
        if (!parse_abs_decl_or_decl(s, &abs_decl_or_decl)) {
            DeclarationSpecs_free(res->decl_specs);
            return false;
        }

        if (abs_decl_or_decl.is_abs) {
            res->kind = PARAM_DECL_ABSTRACT_DECL;
            res->abstract_decl = abs_decl_or_decl.abs_decl;
        } else {
            res->kind = PARAM_DECL_DECL;
            res->decl = abs_decl_or_decl.decl;
        }
    }

    return true;
}

void ParamDeclaration_free_children(ParamDeclaration* d) {
    DeclarationSpecs_free(d->decl_specs);
    switch (d->kind) {
        case PARAM_DECL_DECL:
            Declarator_free(d->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            AbsDeclarator_free(d->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            break;
    }
}

static bool parse_param_list_inplace(ParserState* s, ParamList* res) {
    res->decls = mycc_alloc(sizeof *res->decls);
    res->len = 1;

    if (!parse_param_declaration_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_COMMA && s->it[1].kind != TOKEN_ELLIPSIS) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls,
                            &alloc_len,
                            sizeof *res->decls);
        }

        if (!parse_param_declaration_inplace(s, &res->decls[res->len])) {
            ParamList_free(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return true;
}

void ParamList_free(ParamList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        ParamDeclaration_free_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

bool parse_param_type_list(ParserState* s, ParamTypeList* res) {
    if (!parse_param_list_inplace(s, &res->param_list)) {
        return false;
    }

    res->is_variadic = false;
    if (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);
        if (!parser_accept(s, TOKEN_ELLIPSIS)) {
            ParamList_free(&res->param_list);
            return false;
        }
        res->is_variadic = true;
    }

    return true;
}

void ParamTypeList_free(ParamTypeList* l) {
    ParamList_free(&l->param_list);
}

