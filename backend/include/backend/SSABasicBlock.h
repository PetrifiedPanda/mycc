#ifndef SSA_BASIC_BLOCK_H
#define SSA_BASIC_BLOCK_H

#include "BasicBlock.h"

// TODO: may be better to make this more like a struct of arrays
typedef struct {
    IRRegRef dest;
    
    size_t len;
    IRRegRef* options;
} PhiInst;

typedef struct {
    size_t num_phis;
    PhiInst* phis;
    BasicBlock bb;
} SSABasicBlock;

void free_phi_inst(PhiInst* i);

void free_ssa_basic_block(SSABasicBlock* bb);

#endif

