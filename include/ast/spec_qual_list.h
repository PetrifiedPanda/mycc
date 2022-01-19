#ifndef SPEC_QUAL_LIST_H
#define SPEC_QUAL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/type_qual.h"

struct type_spec;

struct type_spec_or_qual {
    bool is_type_spec;
    union {
        struct type_spec* type_spec;
        struct type_qual type_qual;
    };
};

struct spec_qual_list {
    size_t len;
    struct type_spec_or_qual* specs_or_quals;
};

struct spec_qual_list parse_spec_qual_list(struct parser_state* s);

void free_spec_qual_list(struct spec_qual_list* l);

#include "ast/type_spec.h"

#endif

