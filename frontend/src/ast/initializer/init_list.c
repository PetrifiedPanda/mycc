#include "frontend/ast/initializer/init_list.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_designation_init(struct parser_state* s,
                                   struct designation_init* res) {
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

bool parse_init_list(struct parser_state* s, struct init_list* res) {
    *res = (struct init_list){
        .len = 1,
        .inits = mycc_alloc(sizeof *res->inits),
    };
    if (!parse_designation_init(s, &res->inits[0])) {
        mycc_free(res->inits);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA && s->it[1].type != RBRACE) {
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->inits, &alloc_len, sizeof *res->inits);
        }

        if (!parse_designation_init(s, &res->inits[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->inits = mycc_realloc(res->inits, sizeof *res->inits * res->len);

    return res;
fail:
    free_init_list_children(res);
    return false;
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct designation_init* item = &l->inits[i];
        if (item->designation) {
            free_designation(item->designation);
        }
        free_initializer(item->init);
    }
    mycc_free(l->inits);
}
