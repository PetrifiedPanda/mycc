#include "frontend/ast/spec_qual_list.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_spec_or_qual(struct parser_state* s,
                               struct spec_qual_list* res) {
    assert(res);

    if (is_type_qual(s->it->kind)) {
        update_type_quals(s, &res->quals);
    } else if (!update_type_specs(s, &res->specs)) {
        return false;
    }

    return true;
}

bool parse_spec_qual_list(struct parser_state* s, struct spec_qual_list* res) {
    *res = (struct spec_qual_list){
        .info = create_ast_node_info(s->it->loc),
        .quals = create_type_quals(),
        .specs = create_type_specs(),
    };

    if (!parse_spec_or_qual(s, res)) {
        return false;
    }

    while (is_type_spec(s) || is_type_qual(s->it->kind)) {
        if (!parse_spec_or_qual(s, res)) {
            free_spec_qual_list_children(res);
            return false;
        }
    }

    return res;
}

void free_spec_qual_list_children(struct spec_qual_list* l) {
    free_type_specs_children(&l->specs);
}

void free_spec_qual_list(struct spec_qual_list* l) {
    free_spec_qual_list_children(l);
    mycc_free(l);
}

