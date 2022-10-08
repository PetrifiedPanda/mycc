#ifndef SSA_BASIC_BLOCK_H
#define SSA_BASIC_BLOCK_H

#include "basic_block.h"

// TODO: may be better to make this more like a struct of arrays
struct phi_inst {
    struct ir_reg_ref dest;
    
    size_t len;
    struct ir_reg_ref* options;
};

struct ssa_basic_block {
    size_t num_phis;
    struct phi_inst* phis;
    struct basic_block bb;
};

void free_phi_inst(struct phi_inst* i);

void free_ssa_basic_block(struct ssa_basic_block* bb);

#endif

