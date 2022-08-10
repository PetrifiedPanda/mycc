#include "backend/ssa_basic_block.h"

#include <stdlib.h>

void free_phi_inst(struct phi_inst* i) {
    free(i->options); 
}

void free_ssa_basic_block(struct ssa_basic_block* bb) {
    for (size_t i = 0; i < bb->num_phis; ++i) {
        free_phi_inst(&bb->phis[i]);
    }

    free_basic_block(&bb->bb);
}
