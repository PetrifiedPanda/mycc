#include "ast/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

/**
 *
 * @param s The current parser_state
 * @param res The adress where the result is to be written in
 * @param found_typedef Logs whether a typedef was among the declaration specifiers
 * @return 0 for an error, 1 for success and 2 if the next token is not a declaration_spec
 */
static int parse_declaration_spec_cont(struct parser_state* s, struct declaration_specs_cont* res, bool* found_typedef) {
    assert(res);

    if (is_storage_class_spec(s->it->type)) {
        if (s->it->type == TYPEDEF) {
            *found_typedef = true;
        }
        res->type = DECLSPEC_STORAGE_CLASS_SPEC;
        res->storage_class_spec = s->it->type;
        accept_it(s);
    } else if (is_type_qual(s->it->type)) {
        res->type = DECLSPEC_TYPE_QUAL;
        res->type_qual = parse_type_qual(s);

        assert(res->type_qual.type != INVALID);
    } else if (is_type_spec(s)) {
        res->type = DECLSPEC_TYPE_SPEC;
        res->type_spec = parse_type_spec(s);
        if (!res->type_spec) {
            return 0;
        }
    } else if (is_func_spec(s->it->type)) {
        res->type = DECLSPEC_FUNC_SPEC;
        res->func_spec.is_inline = s->it->type == INLINE;
        accept_it(s);
    } else if (s->it->type == ALIGNAS) {
        res->type = DECLSPEC_ALIGN_SPEC;

        res->align_spec = parse_align_spec(s);
        if (!res->align_spec) {
            return 0;
        }
    } else {
        return 2;
    }

    return 1;
}

struct declaration_specs* parse_declaration_specs(struct parser_state* s, bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    struct declaration_specs* res = xmalloc(sizeof(struct declaration_specs));
    res->contents = xmalloc(sizeof(struct declaration_specs_cont));
    if (parse_declaration_spec_cont(s, res->contents, found_typedef) != 1) {
        free(res->contents);
        free(res);
        return NULL;
    }

    size_t alloc_len = res->len = 1;
    while (true) {
        if (alloc_len == res->len) {
            grow_alloc((void**)&res->contents, &alloc_len, sizeof(struct declaration_specs_cont));
        }

        int success = parse_declaration_spec_cont(s, &res->contents[res->len], found_typedef);

        if (success == 0) {
            goto fail;
        } else if (success == 2) {
            break;
        } else {
            ++res->len;
        }
    }

    res->contents = xrealloc(res->contents, res->len * sizeof(struct declaration_specs_cont));

    return res;
fail:
    free_declaration_specs(res);
    return NULL;
}

static void free_children(struct declaration_specs* s) {
    for (size_t i = 0; i < s->len; ++i) {
        struct declaration_specs_cont* item = &s->contents[i];
        switch (item->type) {
            case DECLSPEC_TYPE_SPEC:
                free_type_spec(item->type_spec);
                break;
            case DECLSPEC_ALIGN_SPEC:
                free_align_spec(item->align_spec);
                break;
            default:
                break;
        }
    }
    free(s->contents);
}

void free_declaration_specs(struct declaration_specs* s) {
    free_children(s);
    free(s);
}

