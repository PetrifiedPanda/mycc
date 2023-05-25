#ifndef SSA_CFG_H
#define SSA_CFG_H

#include "SSABasicBlock.h"

typedef struct {
    size_t name_idx;
    size_t val_inst_num;
    IRTypeRef type;
} SSARegInfo;

typedef struct {
    Str name;
    size_t len;
    SSABasicBlock* blocks;

    size_t num_func_args;
    size_t num_regs;
    SSARegInfo* regs;
    
    size_t num_val_names;
    Str* val_names;
} SSACFG;

void free_ssa_cfg(SSACFG* cfg);

#endif

