#ifndef IR_MODULE
#define IR_MODULE

#include "cfg.h"

// TODO: how to incorporate ssa_cfgs?

struct ir_module {
    char* name;
    size_t num_funcs;
    struct cfg* funcs;
    size_t num_globals;
    struct inst_global_info* globals;
};

void free_ir_module(struct ir_module* mod);

#endif

