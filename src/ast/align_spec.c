#include "ast/align_spec.h"

#include "util.h"

#include "parser/parser_util.h"

struct align_spec* parse_align_spec(struct parser_state* s) {
    struct align_spec* res = xmalloc(sizeof(struct align_spec));
    if (is_type_spec(s) || is_type_qual(s->it->type)) {
        res->is_type_name = true;
        res->type_name = parse_type_name(s);

        if (!res->type_name) {
            free(res);
            return NULL;
        }
    } else {
        res->is_type_name = false;
        res->const_expr = parse_const_expr(s);
        if (!res->const_expr) {
            free(res);
            return NULL;
        }
    }
    return res;
}

void free_align_spec(struct align_spec* s) {
    if (s->is_type_name) {
        free_type_name(s->type_name);
    } else {
        free_const_expr(s->const_expr);
    }
}
