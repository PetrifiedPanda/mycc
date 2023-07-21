#include "backend/BasicBlock.h"

#include "util/mem.h"

void BranchInst_free(BranchInst* b) {
    switch (b->type) {
        case BRANCH_OP_SWITCH:
            for (uint32_t i = 0; i < b->switch_len; ++i) {
                IRLiteral_free(&b->targets[i].val);
            }
            break;
        default:
            break;
    }
}

void BasicBlock_free(BasicBlock* bb) {
    StrBuf_free(&bb->name);
    for (uint32_t i = 0; i < bb->len; ++i) {
        IRInst_free(&bb->ops[i]);
    }
    mycc_free(bb->ops);
    
    BranchInst_free(&bb->branch);

    mycc_free(bb->preds);
}
