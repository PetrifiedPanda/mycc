#include "backend/CFG.h"

#include "util/mem.h"

void CFG_free(CFG* cfg) {
    StrBuf_free(&cfg->name);

    for (uint32_t i = 0; i < cfg->len; ++i) {
        BasicBlock_free(&cfg->blocks[i]);
    }
    mycc_free(cfg->blocks);

    for (uint32_t i = 0; i < cfg->num_regs; ++i) {
        IRReg_free(&cfg->regs[i]);
    }
    mycc_free(cfg->regs);
}
