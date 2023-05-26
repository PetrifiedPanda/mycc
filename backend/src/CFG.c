#include "backend/CFG.h"

#include "util/mem.h"

void free_cfg(CFG* cfg) {
    Str_free(&cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        free_basic_block(&cfg->blocks[i]);
    }
    mycc_free(cfg->blocks);

    for (size_t i = 0; i < cfg->num_regs; ++i) {
        free_ir_reg(&cfg->regs[i]);
    }
    mycc_free(cfg->regs);
}
