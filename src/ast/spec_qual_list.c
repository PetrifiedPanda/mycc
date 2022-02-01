#include "ast/spec_qual_list.h"

#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static bool parse_spec_or_qual(struct parser_state* s, struct spec_qual_list* res, size_t* alloc_len) {
    assert(res);
    assert(alloc_len);

    if (is_type_qual(s->it->type)) {
        update_type_quals(s, &res->quals);
    } else {
        if (res->len == *alloc_len) {
            grow_alloc((void**)&res->type_specs, alloc_len, sizeof(struct type_spec));
        }

        if (!parse_type_spec_inplace(s, &res->type_specs[res->len])) {
            return false;
        }

        ++res->len;
    }

    return true;
}

struct spec_qual_list parse_spec_qual_list(struct parser_state* s) {
    struct spec_qual_list res = {
            .quals = create_type_quals(),
            .len = 0,
            .type_specs = NULL
    };

    size_t alloc_len = res.len;
    if (!parse_spec_or_qual(s, &res, &alloc_len)) {
        free(res.type_specs);
        return (struct spec_qual_list) {
            .quals = create_type_quals(),
            .len = 0,
            .type_specs = NULL
        };
    }

    while (is_type_spec(s) || is_type_qual(s->it->type)) {
        if (!parse_spec_or_qual(s, &res, &alloc_len)) {
            free_spec_qual_list(&res);
            return (struct spec_qual_list) {
                .quals = create_type_quals(),
                .len = 0,
                .type_specs = NULL,
            };
        }
    }

    res.type_specs = xrealloc(res.type_specs, sizeof(struct type_spec) * res.len);

    return res;
}

void free_spec_qual_list(struct spec_qual_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_type_spec_children(&l->type_specs[i]);
    }
    free(l->type_specs);
}

bool is_valid_spec_qual_list(struct spec_qual_list* l) {
    if (l->len > 0) {
        return true;
    }
    struct type_quals* tq = &l->quals;
    return tq->is_atomic || tq->is_volatile || tq->is_restrict || tq->is_const;
}
