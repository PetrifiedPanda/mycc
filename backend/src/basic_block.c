#include "backend/basic_block.h"

#include <stdlib.h>

void free_branch_inst(struct branch_inst* b) {
    switch (b->type) {
        case BRANCH_OP_SWITCH:
            for (size_t i = 0; i < b->switch_len; ++i) {
                free_inst_literal(&b->targets[i].val);
            }
            break;
        default:
            break;
    }
}

void free_basic_block(struct basic_block* bb) {
    free(bb->name);
    for (size_t i = 0; i < bb->len; ++i) {
        free_inst(&bb->ops[i]);
    }
    free(bb->ops);
    
    free_branch_inst(&bb->branch);

    free(bb->preds);
}
