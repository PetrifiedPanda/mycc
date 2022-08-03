#ifndef CFG_H
#define CFG_H

#include "basic_block.h"

struct cfg {
    char* name;
    size_t len;
    struct basic_block* blocks;
    size_t num_regs;
    struct tac_reg_info* regs;
};

void cfg_free(struct cfg* cfg);

#endif

