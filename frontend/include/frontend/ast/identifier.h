#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <stdbool.h>

#include "ast_node_info.h"

struct identifier {
    struct ast_node_info info;
    struct str spelling;
};

void init_identifier(struct identifier* res,
                     const struct str* spelling,
                     struct source_loc loc);
struct identifier* create_identifier(const struct str* spelling,
                                     struct source_loc loc);

void free_identifier_children(struct identifier* i);

void free_identifier(struct identifier* i);

#endif

