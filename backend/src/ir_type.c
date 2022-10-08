#include "backend/ir_type.h"

#include <stdlib.h>

void free_ir_type(struct ir_type* t) {
    switch (t->type) {
        case INST_TYPE_BUILTIN:
        case INST_TYPE_ARR:
            break;
        case INST_TYPE_STRUCT:
            free(t->struct_type.member_types);
            break;
        case INST_TYPE_FUNC:
            free(t->func_type.arg_types);
            break;
    }
}
