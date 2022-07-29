#ifndef BLOCK_ITEM_H
#define BLOCK_ITEM_H

#include <stdbool.h>

#include "statement.h"

#include "frontend/ast/declaration/declaration.h"

#include "frontend/parser/parser_state.h"

struct block_item {
    bool is_decl;
    union {
        struct declaration decl;
        struct statement stat;
    };
};

bool parse_block_item_inplace(struct parser_state* s, struct block_item* res);

void free_block_item_children(struct block_item* i);

#endif
