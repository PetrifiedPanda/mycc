#include "ast/param_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "error.h"

// There might be a better way to do this
static bool is_declarator(const struct token* current) {
    const struct token* it = current;
    while (it->type == ASTERISK) {
        ++it;

        while (is_type_qual(it->type)) {
            ++it;
        }
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

bool parse_param_declaration_inplace(struct parser_state* s,
                                     struct param_declaration* res) {
    assert(res);

    bool found_typedef = false;
    res->decl_specs = parse_declaration_specs(s, &found_typedef);
    if (!res->decl_specs) {
        return false;
    }

    if (found_typedef) {
        set_error_file(ERR_PARSER,
                       s->it->file,
                       s->it->source_loc,
                       "typedef is not allowed in function declarator");
        free_declaration_specs(res->decl_specs);
        return false;
    }

    if (s->it->type == COMMA || s->it->type == RBRACKET) {
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
