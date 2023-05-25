#include "backend/SSABasicBlock.h"

#include "util/mem.h"

void free_phi_inst(PhiInst* i) {
    mycc_free(i->options); 
}

void free_ssa_basic_block(SSABasicBlock* bb) {
    for (size_t i = 0; i < bb->num_phis; ++i) {
        free_phi_inst(&bb->phis[i]);
    }

    free_basic_block(&bb->bb);
}
