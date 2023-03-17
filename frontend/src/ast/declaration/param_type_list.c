#include "frontend/ast/declaration/param_type_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct abs_decl_or_decl {
    bool is_abs;
    union {
        struct declarator* decl;
        struct abs_declarator* abs_decl;
    };
};

static bool parse_abs_decl_or_decl(struct parser_state* s,
                                   struct abs_decl_or_decl* res) {
    assert(res);
    struct pointer* ptr;
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
        const struct source_loc loc = s->it->loc;
        parser_accept_it(s);
        struct abs_decl_or_decl bracket_decl;
        if (!parse_abs_decl_or_decl(s, &bracket_decl)) {
            goto fail;
        }
        res->is_abs = bracket_decl.is_abs;
        if (bracket_decl.is_abs) {
            res->abs_decl = mycc_alloc(sizeof *res->abs_decl);
            res->abs_decl->ptr = ptr;
            res->abs_decl->direct_abs_decl = mycc_alloc(
                sizeof *res->abs_decl->direct_abs_decl);
            struct direct_abs_declarator* decl = res->abs_decl->direct_abs_decl;
            decl->info = create_ast_node_info(loc);
            decl->bracket_decl = bracket_decl.abs_decl;

            if (!parser_accept(s, TOKEN_RBRACKET)) {
                decl->len = 0;
                decl->following_suffixes = NULL;
                free_direct_abs_declarator(decl);
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
            struct direct_declarator* decl = res->decl->direct_decl;
            decl->info = create_ast_node_info(loc);
            decl->is_id = false;
            decl->bracket_decl = bracket_decl.decl;

            if (!parser_accept(s, TOKEN_RBRACKET)) {
                decl->len = 0;
                decl->suffixes = NULL;
                free_direct_declarator(decl);
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
            set_parser_err(s->err,
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
        free_pointer(ptr);
    }
    return false;
}

static bool parse_param_declaration_inplace(struct parser_state* s,
                                            struct param_declaration* res) {
    assert(res);

    bool found_typedef = false;
    res->decl_specs = parse_declaration_specs(s, &found_typedef);
    if (!res->decl_specs) {
        return false;
    }

    if (found_typedef) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_PARAM_DECL, s->it->loc);
        free_declaration_specs(res->decl_specs);
        return false;
    }

    if (s->it->kind == TOKEN_COMMA || s->it->kind == TOKEN_RBRACKET) {
        res->kind = PARAM_DECL_NONE;
        res->decl = NULL;
    } else {
        struct abs_decl_or_decl abs_decl_or_decl;
        if (!parse_abs_decl_or_decl(s, &abs_decl_or_decl)) {
            free_declaration_specs(res->decl_specs);
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

void free_param_declaration_children(struct param_declaration* d) {
    free_declaration_specs(d->decl_specs);
    switch (d->kind) {
        case PARAM_DECL_DECL:
            free_declarator(d->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            free_abs_declarator(d->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            break;
    }
}

static bool parse_param_list_inplace(struct parser_state* s,
                                     struct param_list* res) {
    res->decls = mycc_alloc(sizeof *res->decls);
    res->len = 1;

    if (!parse_param_declaration_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        mycc_free(res);
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
            free_param_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return true;
}

void free_param_list(struct param_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_param_declaration_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

bool parse_param_type_list(struct parser_state* s,
                           struct param_type_list* res) {
    if (!parse_param_list_inplace(s, &res->param_list)) {
        return false;
    }

    res->is_variadic = false;
    if (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);
        if (!parser_accept(s, TOKEN_ELLIPSIS)) {
            free_param_list(&res->param_list);
            return false;
        }
        res->is_variadic = true;
    }

    return true;
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(&l->param_list);
}

