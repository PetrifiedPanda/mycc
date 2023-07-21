#include "backend/SSABasicBlock.h"

#include "util/mem.h"

void PhiInst_free(PhiInst* i) {
    mycc_free(i->options); 
}

void SSABasicBlock_free(SSABasicBlock* bb) {
    for (uint32_t i = 0; i < bb->num_phis; ++i) {
        PhiInst_free(&bb->phis[i]);
    }

    BasicBlock_free(&bb->bb);
}
