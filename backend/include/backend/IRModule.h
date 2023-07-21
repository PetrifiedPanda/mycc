#ifndef MYCC_BACKEND_IR_MODULE_H
#define MYCC_BACKEND_IR_MODULE_H

#include "util/StrBuf.h"

#include "CFG.h"

// TODO: how to incorporate ssa_cfgs?

typedef struct {
    StrBuf name;
    uint32_t num_funcs;
    CFG* funcs;
    uint32_t num_globals;
    IRGlobal* globals;
    uint32_t num_types;
    IRType* types;
} IRModule;

void IRModule_free(IRModule* mod);

#endif

