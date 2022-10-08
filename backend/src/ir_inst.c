#include "backend/ir_inst.h"

#include <stdlib.h>
#include <assert.h>

void free_ir_global(struct ir_global* g) {
    free_str(&g->name);
}

void free_ir_reg(struct ir_reg* reg) {
    free_str(&reg->name);
}

void free_ir_literal(struct ir_literal* lit) {
    switch (lit->type) {
        case IR_LITERAL_STRUCT:
        case IR_LITERAL_ARR:
            for (size_t i = 0; i < lit->num_members; ++i) {
                free_ir_literal(&lit->members[i]);
            }
            free(lit->members);
            break;
        default:
            break;
    }
}

struct ir_inst create_call_inst(const struct ir_type* type,
                             const struct ir_reg_ref* dest,
                             const struct ir_inst_arg* func,
                             size_t num_func_args,
                             struct ir_inst_arg* func_args) {
    return (struct ir_inst){
        .op = IR_INST_CALL,
        .type = *type,
        .dest = *dest,
        .func = *func,
        .num_func_args = num_func_args,
        .func_args = func_args,
    };
}

struct ir_inst create_assign_inst(const struct ir_type* type,
                               const struct ir_reg_ref* dest,
                               const struct ir_inst_arg* val) {
    return (struct ir_inst){
        .op = IR_INST_ASSIGN,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

struct ir_inst create_cast_inst(const struct ir_type* type,
                             const struct ir_reg_ref* dest,
                             const struct ir_inst_arg* val) {
    return (struct ir_inst){
        .op = IR_INST_CAST,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
    };
}

struct ir_inst create_alloca_inst(const struct ir_reg_ref* dest,
                               const struct ir_type* type) {
    return (struct ir_inst){
        .op = IR_INST_ALLOCA,
        .type = *type,
        .dest = *dest,
        .arg1.type = IR_INST_ARG_NONE,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

struct ir_inst create_inst(enum ir_inst_op op,
                        const struct ir_type* type,
                        const struct ir_reg_ref* dest,
                        const struct ir_inst_arg* arg1,
                        const struct ir_inst_arg* arg2) {
    return (struct ir_inst){
        .op = op,
        .type = *type,
        .dest = *dest,
        .arg1 = *arg1,
        .arg2 = *arg2,
    };
}

struct ir_inst create_store_inst(const struct ir_inst_arg* ptr,
                              const struct ir_inst_arg* to_store) {
    return (struct ir_inst){
        .op = IR_INST_STORE,
        .arg1 = *ptr,
        .arg2 = *to_store,
    };
}

struct ir_inst create_load_inst(const struct ir_reg_ref* dest,
                             const struct ir_inst_arg* ptr) {
    return (struct ir_inst){
        .op = IR_INST_LOAD,
        .dest = *dest,
        .arg1 = *ptr,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

struct ir_inst create_getelem_inst(const struct ir_reg_ref* dest,
                                const struct ir_type* type,
                                const struct ir_inst_arg* accessed,
                                size_t num_accesses,
                                struct ir_inst_arg* elems) {
    return (struct ir_inst){
        .op = IR_INST_GETELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = IR_INST_ARG_NONE,
    };
}

struct ir_inst create_getelemptr_inst(const struct ir_reg_ref* dest,
                                   const struct ir_type* type,
                                   const struct ir_inst_arg* accessed,
                                   size_t num_accesses,
                                   struct ir_inst_arg* elems) {
    return (struct ir_inst){
        .op = IR_INST_GETELEMPTR,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = IR_INST_ARG_NONE,
    };
}

struct ir_inst create_replace_elem_inst(const struct ir_reg_ref* dest,
                                     const struct ir_type* type,
                                     const struct ir_inst_arg* accessed,
                                     size_t num_accesses,
                                     struct ir_inst_arg* elems,
                                     const struct ir_inst_arg* replacement) {
    return (struct ir_inst){
        .op = IR_INST_REPLACEELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement = *replacement,
    };
}

void free_inst_arg(struct ir_inst_arg* arg) {
    if (arg->type == IR_INST_ARG_LITERAL) {
        free_ir_literal(&arg->lit);
    }
}

static void free_args(struct ir_inst_arg* args, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        free_inst_arg(&args[i]);
    }
    free(args);
}

void free_ir_inst(struct ir_inst* tac) {
    switch (tac->op) {
        case IR_INST_ADD:
        case IR_INST_SUB:
        case IR_INST_MUL:
        case IR_INST_DIV:
        case IR_INST_UDIV:
        case IR_INST_AND:
        case IR_INST_OR:
        case IR_INST_XOR:
        case IR_INST_LSHIFT:
        case IR_INST_RSHIFT:
        case IR_INST_MOD:
        case IR_INST_EQ:
        case IR_INST_NEQ:
        case IR_INST_LT:
        case IR_INST_LE:
        case IR_INST_GT:
        case IR_INST_GE:
        case IR_INST_STORE:
        case IR_INST_LOAD:
            free_inst_arg(&tac->arg1);
            free_inst_arg(&tac->arg2);
            break;
        case IR_INST_CALL:
            free_inst_arg(&tac->func);
            free_args(tac->func_args, tac->num_func_args);
            break;
        case IR_INST_ASSIGN:
        case IR_INST_CAST:
            free_inst_arg(&tac->arg1);
            break;
        case IR_INST_ALLOCA:
            break;
        case IR_INST_GETELEMPTR:
        case IR_INST_GETELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            break;
        case IR_INST_REPLACEELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            free_inst_arg(&tac->replacement);
            break;
    }
}

