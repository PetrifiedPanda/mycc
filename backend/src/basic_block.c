#include "backend/basic_block.h"

#include <stdlib.h>

void tac_branch_free(struct tac_branch* b) {
    switch (b->type) {
        case BRANCH_OP_SWITCH:
            for (size_t i = 0; i < b->switch_len; ++i) {
                tac_literal_free(&b->targets[i].val);
            }
            break;
        default:
            break;
    }
}

void basic_block_free(struct basic_block* bb) {
    free(bb->name);
    for (size_t i = 0; i < bb->len; ++i) {
        tac_free(&bb->ops[i]);
    }
    free(bb->ops);
    
    tac_branch_free(&bb->branch);

    free(bb->preds);
}
