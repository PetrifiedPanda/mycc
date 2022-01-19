#include "ast/spec_qual_list.h"

#include <assert.h>

#include "util.h"

static bool parse_spec_or_qual(struct parser_state* s, struct type_spec_or_qual* res) {
    if (is_type_qual(s->it->type)) {
        res->is_type_spec = false;
        res->type_qual = parse_type_qual(s);
        if (res->type_qual.type == INVALID) {
            return false;
        }
    } else {
        res->is_type_spec = true;
        res->type_spec = parse_type_spec(s);
        if (!res->type_spec) {
            return false;
        }
    }

    return true;
}

static bool is_type_spec(const struct parser_state* s) {
    switch (s->it->type) {
        case VOID:
        case CHAR:
        case SHORT:
        case INT:
        case LONG:
        case FLOAT:
        case DOUBLE:
        case SIGNED:
        case UNSIGNED:
        case BOOL:
        case COMPLEX:
        case IMAGINARY:
        case ATOMIC:
        case STRUCT:
        case UNION:
        case ENUM:
            return true;
        case IDENTIFIER:
            return is_typedef_name(s, s->it->spelling);
        default:
            return false;
    }
}

struct spec_qual_list parse_spec_qual_list(struct parser_state* s) {
    struct spec_qual_list res = {
            .specs_or_quals = xmalloc(sizeof(struct type_spec_or_qual)),
            .len = 1
    };
    if (!parse_spec_or_qual(s, res.specs_or_quals)) {
        free(res.specs_or_quals);
        return (struct spec_qual_list){
            .specs_or_quals = NULL,
            .len = 0
        };
    }

    size_t alloc_size = res.len;
    while (is_type_spec(s) || is_type_qual(s->it->type)) {
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.specs_or_quals, &alloc_size, sizeof(struct type_spec_or_qual));
        }

        if (!parse_spec_or_qual(s, &res.specs_or_quals[res.len])) {
            goto fail;
        }

        ++res.len;
    }

    return res;

fail:
    free_spec_qual_list(&res);
    return (struct spec_qual_list){
            .specs_or_quals = NULL,
            .len = 0
    };
}

void free_spec_qual_list(struct spec_qual_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct type_spec_or_qual* item = &l->specs_or_quals[i];
        if (item->is_type_spec) {
            free_type_spec(item->type_spec);
        }
    }
}

