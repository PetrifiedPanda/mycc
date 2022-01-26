#include "ast/func_def.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static struct func_def* create_func_def(struct declaration_specs* specs, struct declarator* decl, struct declaration_list decl_list, struct compound_statement* comp) {
    assert(decl);
    assert(comp);
    struct func_def* res = xmalloc(sizeof(struct func_def));
    res->specs = specs;
    res->decl = decl;
    res->decl_list = decl_list;
    res->comp = comp;
    
    return res;
}

bool parse_func_def_inplace(struct parser_state* s, struct func_def* res) {
    res->specs = parse_declaration_specs(s);
    if (!res->specs) {
        return false;
    }

    res->decl = parse_declarator(s);
    if (!res->decl) {
        free_declaration_specs(res->specs);
        return false;
    }

    if (s->it->type != LBRACE) {
        res->decl_list = parse_declaration_list(s);
        if (res->decl_list.len == 0) {
            free_declaration_specs(res->specs);
            free_declarator(res->decl);
            return false;
        }
    } else {
        res->decl_list = (struct declaration_list) {
            .len = 0,
            .decls = NULL
        };
    }

    res->comp = parse_compound_statement(s);
    if (!res->comp) {
        free_declaration_specs(res->specs);
        free_declarator(res->decl);
        free_declaration_list(&res->decl_list);
        return false;
    }

    return true;
}

void free_func_def_children(struct func_def* d) {
    if (d->specs) {
        free_declaration_specs(d->specs);
    }
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}

void free_func_def(struct func_def* d) {
    free_func_def_children(d);
    free(d);
}

