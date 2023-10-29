#ifndef MYCC_BACKEND_SSA_BASIC_BLOCK_H
#define MYCC_BACKEND_SSA_BASIC_BLOCK_H

#include "BasicBlock.h"

// TODO: may be better to make this more like a struct of arrays
typedef struct PhiInst {
    IRRegRef dest;
    
    uint32_t len;
    IRRegRef* options;
} PhiInst;

typedef struct SSABasicBlock {
    uint32_t num_phis;
    PhiInst* phis;
    BasicBlock bb;
} SSABasicBlock;

void PhiInst_free(PhiInst* i);

void SSABasicBlock_free(SSABasicBlock* bb);

#endif

