#ifndef SPEC_QUAL_LIST_H
#define SPEC_QUAL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/type_quals.h"
#include "ast/type_specs.h"

struct spec_qual_list {
    struct type_quals quals;
    struct type_specs specs;
};

struct spec_qual_list parse_spec_qual_list(struct parser_state* s);

void free_spec_qual_list_children(struct spec_qual_list* l);
void free_spec_qual_list(struct spec_qual_list* l);

bool is_valid_spec_qual_list(struct spec_qual_list* l);

#include "ast/type_specs.h"

#endif

