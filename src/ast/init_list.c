#include "ast/init_list.h"

#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct init_list create_init_list(struct designation_init* inits, size_t len) {
    assert(len > 0);
    assert(inits);
    return (struct init_list){.len = len, .inits = inits};
}

static bool parse_designation_init(struct parser_state* s, struct designation_init* res) {
    if (s->it->type == LINDEX || s->it->type == DOT) {
        res->designation = parse_designation(s);
        if (!res->designation) {
            return false;
        }
    } else {
        res->designation = NULL;
    }

    res->init = parse_initializer(s);
    if (!res->init) {
        if (res->designation) {
            free_designation(res->designation);
        }
        return false;
    }
    return true;
}

struct init_list parse_init_list(struct parser_state* s) {
    struct init_list res = {
            .len = 1,
            .inits = xmalloc(sizeof(struct designation_init))
    };
    if (!parse_designation_init(s, &res.inits[0])) {
        free(res.inits);
        return (struct init_list){.len = 0, .inits = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == COMMA && s->it[1].type != RBRACE) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.inits, &alloc_len, sizeof(struct designation_init));
        }

        if (!parse_designation_init(s, &res.inits[res.len])) {
            goto fail;
        }

        ++res.len;
    }

    res.inits = xrealloc(res.inits, res.len * sizeof(struct designation_init));

    return res;
fail:
    free_init_list_children(&res);
    return (struct init_list){.len = 0, .inits = NULL};
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct designation_init* item = &l->inits[i];
        if (item->designation) {
            free_designation(item->designation);
        }
        free_initializer(item->init);
    }
    free(l->inits);
}

