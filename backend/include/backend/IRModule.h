#ifndef MYCC_BACKEND_IR_MODULE_H
#define MYCC_BACKEND_IR_MODULE_H

#include "util/StrBuf.h"

#include "CFG.h"

// TODO: how to incorporate ssa_cfgs?

typedef struct {
    StrBuf name;
    size_t num_funcs;
    CFG* funcs;
    size_t num_globals;
    IRGlobal* globals;
    size_t num_types;
    IRType* types;
} IRModule;

void IRModule_free(IRModule* mod);

#endif

