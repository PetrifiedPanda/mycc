#include "backend/ir_type.h"

#include "util/mem.h"

void free_ir_type(struct ir_type* t) {
    switch (t->type) {
        case INST_TYPE_BUILTIN:
        case INST_TYPE_ARR:
            break;
        case INST_TYPE_STRUCT:
            mycc_free(t->struct_type.member_types);
            break;
        case INST_TYPE_FUNC:
            mycc_free(t->func_type.arg_types);
            break;
    }
}
