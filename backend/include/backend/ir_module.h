#ifndef IR_MODULE
#define IR_MODULE

#include "util/str.h"

#include "cfg.h"

// TODO: how to incorporate ssa_cfgs?

struct ir_module {
    struct str name;
    size_t num_funcs;
    struct cfg* funcs;
    size_t num_globals;
    struct ir_global* globals;
    size_t num_types;
    struct ir_type_info* types;
};

void free_ir_module(struct ir_module* mod);

#endif

