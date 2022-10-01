#ifndef CFG_H
#define CFG_H

#include "util/str.h"

#include "basic_block.h"

struct cfg {
    struct str name;
    size_t len;
    struct basic_block* blocks;
    size_t num_func_args; // first n regs are args
    size_t num_regs;
    struct inst_reg_info* regs;
};

void free_cfg(struct cfg* cfg);

#endif

