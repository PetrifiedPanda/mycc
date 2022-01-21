#include "ast/type_name.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res) {
    assert(res);

    if (is_type_spec(s) || is_type_qual(s->it->type)) {
        res->spec_qual_list = parse_spec_qual_list(s);
        if (res->spec_qual_list.len == 0) {
            return false;
        }
    } else {
        res->spec_qual_list = (struct spec_qual_list) {
            .len = 0,
            .specs_or_quals = NULL
        };
    }

    res->abstract_decl = parse_abs_declarator(s);
    if (!res->abstract_decl) {
        free_spec_qual_list(&res->spec_qual_list);
        return false;
    }

    return true;
}

struct type_name* parse_type_name(struct parser_state* s) {
    struct type_name* res = xmalloc(sizeof(struct type_name));
    if (!parse_type_name_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_type_name_children(struct type_name* n) {
    free_spec_qual_list(&n->spec_qual_list);
    if (n->abstract_decl) {
        free_abs_declarator(n->abstract_decl);
    }
}

void free_type_name(struct type_name* n) {
    free_type_name_children(n);
    free(n);
}

