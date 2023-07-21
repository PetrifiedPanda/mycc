#ifndef MYCC_BACKEND_CFG_H
#define MYCC_BACKEND_CFG_H

#include "util/StrBuf.h"

#include "BasicBlock.h"

typedef struct {
    StrBuf name;
    uint32_t len;
    BasicBlock* blocks;
    uint32_t num_func_args; // first n regs are args
    uint32_t num_regs;
    IRReg* regs;
} CFG;

void CFG_free(CFG* cfg);

#endif

