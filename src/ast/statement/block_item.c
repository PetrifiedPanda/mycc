#include "ast/statement/block_item.h"

#include "parser/parser_util.h"

bool parse_block_item_inplace(struct parser_state* s, struct block_item* res) {
    if (is_declaration(s)) {
        res->is_decl = true;
        if (!parse_declaration_inplace(s, &res->decl)) {
            return false;
        }
    } else {
        res->is_decl = false;
        if (!parse_statement_inplace(s, &res->stat)) {
            return false;
        }
    }

    return true;
}

void free_block_item_children(struct block_item* i) {
    if (i->is_decl) {
        free_declaration_children(&i->decl);
    } else {
        free_statement_children(&i->stat);
    }
}
