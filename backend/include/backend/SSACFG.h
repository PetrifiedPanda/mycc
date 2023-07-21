#ifndef MYCC_BACKEND_SSA_CFG_H
#define MYCC_BACKEND_SSA_CFG_H

#include "SSABasicBlock.h"

typedef struct {
    uint32_t name_idx;
    uint32_t val_inst_num;
    IRTypeRef type;
} SSARegInfo;

typedef struct {
    StrBuf name;
    uint32_t len;
    SSABasicBlock* blocks;

    uint32_t num_func_args;
    uint32_t num_regs;
    SSARegInfo* regs;
    
    uint32_t num_val_names;
    StrBuf* val_names;
} SSACFG;

void SSACFG_free(SSACFG* cfg);

#endif

