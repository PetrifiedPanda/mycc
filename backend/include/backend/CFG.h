#ifndef CFG_H
#define CFG_H

#include "util/Str.h"

#include "BasicBlock.h"

typedef struct {
    Str name;
    size_t len;
    BasicBlock* blocks;
    size_t num_func_args; // first n regs are args
    size_t num_regs;
    IRReg* regs;
} CFG;

void CFG_free(CFG* cfg);

#endif

