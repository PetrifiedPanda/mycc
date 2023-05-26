#ifndef IR_MODULE
#define IR_MODULE

#include "util/Str.h"

#include "CFG.h"

// TODO: how to incorporate ssa_cfgs?

typedef struct {
    Str name;
    size_t num_funcs;
    CFG* funcs;
    size_t num_globals;
    IRGlobal* globals;
    size_t num_types;
    IRType* types;
} IRModule;

void IRModule_free(IRModule* mod);

#endif

