#include "ast/param_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

// There might be a better way to do this
static bool is_declarator(const struct token* current) {
    const struct token* it = current;
    while (it->type == ASTERISK) {
        while (is_type_qual(it->type)) {
            ++it;
        }

        // if got to end of file
        if (it->type == INVALID) {
            return false;
        }

        ++it;
    }

    if (it->type == IDENTIFIER) {
        return true;
    } else if (it->type == LBRACKET) {
        ++it;
        return is_declarator(it);
    } else {
        return false;
    }
}

bool parse_param_declaration_inplace(struct parser_state* s, struct param_declaration* res) {
    assert(res);

    res->decl_specs = parse_declaration_specs(s);
    if (!res->decl_specs) {
        return false;
    }

    if (s->it->type == COMMA) {
        res->type = PARAM_DECL_NONE;
        res->decl = NULL;
    } else if (is_declarator(s->it)) {
        res->type = PARAM_DECL_DECL;
        res->decl = parse_declarator(s);
        if (!res->decl) {
            free_declaration_specs(res->decl_specs);
            return false;
        }
    } else {
        res->type = PARAM_DECL_ABSTRACT_DECL;
        res->abstract_decl = parse_abs_declarator(s);
        if (!res->abstract_decl) {
            free_declaration_specs(res->decl_specs);
            return false;
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

void free_param_declaration(struct param_declaration* d) {
    free_param_declaration_children(d);
    free(d);
}

