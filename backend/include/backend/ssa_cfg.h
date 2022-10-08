#ifndef SSA_CFG_H
#define SSA_CFG_H

#include "ssa_basic_block.h"

struct ssa_reg_info {
    size_t name_idx;
    size_t val_inst_num;
    struct ir_type type;
};

struct ssa_cfg {
    struct str name;
    size_t len;
    struct ssa_basic_block* blocks;

    size_t num_func_args;
    size_t num_regs;
    struct ssa_reg_info* regs;
    
    size_t num_val_names;
    struct str* val_names;
};

void free_ssa_cfg(struct ssa_cfg* cfg);

#endif

