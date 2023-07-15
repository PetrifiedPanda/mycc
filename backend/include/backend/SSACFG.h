#ifndef MYCC_BACKEND_SSA_CFG_H
#define MYCC_BACKEND_SSA_CFG_H

#include "SSABasicBlock.h"

typedef struct {
    size_t name_idx;
    size_t val_inst_num;
    IRTypeRef type;
} SSARegInfo;

typedef struct {
    StrBuf name;
    size_t len;
    SSABasicBlock* blocks;

    size_t num_func_args;
    size_t num_regs;
    SSARegInfo* regs;
    
    size_t num_val_names;
    StrBuf* val_names;
} SSACFG;

void SSACFG_free(SSACFG* cfg);

#endif

