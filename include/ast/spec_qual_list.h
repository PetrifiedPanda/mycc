#ifndef SPEC_QUAL_LIST_H
#define SPEC_QUAL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/type_quals.h"

struct type_spec;

struct spec_qual_list {
    struct type_quals quals;
    size_t len;
    struct type_spec* type_specs;
};

struct spec_qual_list parse_spec_qual_list(struct parser_state* s);

void free_spec_qual_list(struct spec_qual_list* l);

bool is_valid_spec_qual_list(struct spec_qual_list* l);

#include "ast/type_spec.h"

#endif

