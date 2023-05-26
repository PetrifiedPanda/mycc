#include "backend/BasicBlock.h"

#include "util/mem.h"

void free_branch_inst(BranchInst* b) {
    switch (b->type) {
        case BRANCH_OP_SWITCH:
            for (size_t i = 0; i < b->switch_len; ++i) {
                free_ir_literal(&b->targets[i].val);
            }
            break;
        default:
            break;
    }
}

void free_basic_block(BasicBlock* bb) {
    Str_free(&bb->name);
    for (size_t i = 0; i < bb->len; ++i) {
        free_ir_inst(&bb->ops[i]);
    }
    mycc_free(bb->ops);
    
    free_branch_inst(&bb->branch);

    mycc_free(bb->preds);
}
