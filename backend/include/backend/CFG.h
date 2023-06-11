#ifndef CFG_H
#define CFG_H

#include "util/StrBuf.h"

#include "BasicBlock.h"

typedef struct {
    StrBuf name;
    size_t len;
    BasicBlock* blocks;
    size_t num_func_args; // first n regs are args
    size_t num_regs;
    IRReg* regs;
} CFG;

void CFG_free(CFG* cfg);

#endif

