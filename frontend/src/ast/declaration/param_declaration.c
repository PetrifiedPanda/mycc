#include "frontend/ast/declaration/param_declaration.h"

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
    if (s->it->type == ASTERISK) {
        ptr = parse_pointer(s);
        if (!ptr) {
            return false;
        }
    } else {
        ptr = NULL;
    }

    if (s->it->type == IDENTIFIER) {
        res->is_abs = false;
        res->decl = xmalloc(sizeof *res->decl);
        res->decl->ptr = ptr;
        res->decl->direct_decl = parse_direct_declarator(s);
        if (!res->decl->direct_decl) {
            free(res->decl);
            goto fail;
        }
    } else if (s->it->type == LINDEX) {
        res->is_abs = true;
        res->abs_decl = xmalloc(sizeof *res->abs_decl);
        res->abs_decl->ptr = ptr;
        res->abs_decl->direct_abs_decl = parse_direct_abs_declarator(s);
        if (!res->abs_decl->direct_abs_decl) {
            free(res->abs_decl);
            goto fail;
        }
    } else if (s->it->type == LBRACKET) {
        const struct source_loc loc = s->it->loc;
        accept_it(s);
        struct abs_decl_or_decl bracket_decl;
        if (!parse_abs_decl_or_decl(s, &bracket_decl)) {
            goto fail;
        }
        res->is_abs = bracket_decl.is_abs;
        if (bracket_decl.is_abs) {
            res->abs_decl = xmalloc(sizeof *res->abs_decl);
            res->abs_decl->ptr = ptr;
            res->abs_decl->direct_abs_decl = xmalloc(
                sizeof *res->abs_decl->direct_abs_decl);
            struct direct_abs_declarator* decl = res->abs_decl->direct_abs_decl;
            decl->info = create_ast_node_info(loc);
            decl->bracket_decl = bracket_decl.abs_decl;

            if (!accept(s, RBRACKET)) {
                decl->len = 0;
                decl->following_suffixes = NULL;
                free_direct_abs_declarator(decl);
                free(res->abs_decl);
                goto fail;
            }

            if (!parse_abs_arr_or_func_suffixes(s, decl)) {
                free(res->abs_decl);
                goto fail;
            }
        } else {
            res->decl = xmalloc(sizeof *res->decl);
            res->decl->ptr = ptr;
            res->decl->direct_decl = xmalloc(sizeof *res->decl->direct_decl);
            struct direct_declarator* decl = res->decl->direct_decl;
            decl->info = create_ast_node_info(loc);
            decl->is_id = false;
            decl->bracket_decl = bracket_decl.decl;

            if (!accept(s, RBRACKET)) {
                decl->len = 0;
                decl->suffixes = NULL;
                free_direct_declarator(decl);
                free(res->decl);
                goto fail;
            }

            if (!parse_arr_or_func_suffixes(s, decl)) {
                free(res->decl);
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
        res->abs_decl = xmalloc(sizeof *res->abs_decl);
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

bool parse_param_declaration_inplace(struct parser_state* s,
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

    if (s->it->type == COMMA || s->it->type == RBRACKET) {
        res->type = PARAM_DECL_NONE;
        res->decl = NULL;
    } else {
        struct abs_decl_or_decl abs_decl_or_decl;
        if (!parse_abs_decl_or_decl(s, &abs_decl_or_decl)) {
            free_declaration_specs(res->decl_specs);
            return false;
        }

        if (abs_decl_or_decl.is_abs) {
            res->type = PARAM_DECL_ABSTRACT_DECL;
            res->abstract_decl = abs_decl_or_decl.abs_decl;
        } else {
            res->type = PARAM_DECL_DECL;
            res->decl = abs_decl_or_decl.decl;
        }
    }

    return true;
}

void free_param_declaration_children(struct param_declaration* d) {
    free_declaration_specs(d->decl_specs);
    switch (d->type) {
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
