#include "backend/inst.h"

#include <stdlib.h>
#include <assert.h>

void free_inst_type_info(struct inst_type_info* t) {
    switch (t->type) {
        case INST_TYPE_BUILTIN:
        case INST_TYPE_ARR:
            break;
        case INST_TYPE_STRUCT:
            free(t->member_types);
            break;
        case INST_TYPE_FUNC:
            free(t->arg_types);
            break;
    }
}

void free_inst_global_info(struct inst_global_info* g) {
    free_str(&g->name);
}

void free_inst_reg_info(struct inst_reg_info* reg) {
    free_str(&reg->name);
}

void free_inst_literal(struct inst_literal* lit) {
    switch (lit->type) {
        case INST_LITERAL_STRUCT:
        case INST_LITERAL_ARR:
            for (size_t i = 0; i < lit->num_members; ++i) {
                free_inst_literal(&lit->members[i]);
            }
            free(lit->members);
            break;
        default:
            break;
    }
}

struct inst create_call_inst(const struct inst_type* type,
                             const struct inst_reg* dest,
                             const struct inst_arg* func,
                             size_t num_func_args,
                             struct inst_arg* func_args) {
    return (struct inst){
        .op = INST_CALL,
        .type = *type,
        .dest = *dest,
        .func = *func,
        .num_func_args = num_func_args,
        .func_args = func_args,
    };
}

struct inst create_assign_inst(const struct inst_type* type,
                               const struct inst_reg* dest,
                               const struct inst_arg* val) {
    return (struct inst){
        .op = INST_ASSIGN,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
        .arg2.type = INST_ARG_NONE,
    };
}

struct inst create_cast_inst(const struct inst_type* type,
                             const struct inst_reg* dest,
                             const struct inst_arg* val) {
    return (struct inst){
        .op = INST_CAST,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
    };
}

struct inst create_alloca_inst(const struct inst_reg* dest,
                               const struct inst_type* type) {
    return (struct inst){
        .op = INST_ALLOCA,
        .type = *type,
        .dest = *dest,
        .arg1.type = INST_ARG_NONE,
        .arg2.type = INST_ARG_NONE,
    };
}

struct inst create_inst(enum inst_op op,
                        const struct inst_type* type,
                        const struct inst_reg* dest,
                        const struct inst_arg* arg1,
                        const struct inst_arg* arg2) {
    return (struct inst){
        .op = op,
        .type = *type,
        .dest = *dest,
        .arg1 = *arg1,
        .arg2 = *arg2,
    };
}

struct inst create_store_inst(const struct inst_arg* ptr,
                              const struct inst_arg* to_store) {
    return (struct inst){
        .op = INST_STORE,
        .arg1 = *ptr,
        .arg2 = *to_store,
    };
}

struct inst create_load_inst(const struct inst_reg* dest,
                             const struct inst_arg* ptr) {
    return (struct inst){
        .op = INST_LOAD,
        .dest = *dest,
        .arg1 = *ptr,
        .arg2.type = INST_ARG_NONE,
    };
}

struct inst create_getelem_inst(const struct inst_reg* dest,
                                const struct inst_type* type,
                                const struct inst_arg* accessed,
                                size_t num_accesses,
                                struct inst_arg* elems) {
    return (struct inst){
        .op = INST_GETELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = INST_ARG_NONE,
    };
}

struct inst create_getelemptr_inst(const struct inst_reg* dest,
                                   const struct inst_type* type,
                                   const struct inst_arg* accessed,
                                   size_t num_accesses,
                                   struct inst_arg* elems) {
    return (struct inst){
        .op = INST_GETELEMPTR,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = INST_ARG_NONE,
    };
}

struct inst create_replace_elem_inst(const struct inst_reg* dest,
                                     const struct inst_type* type,
                                     const struct inst_arg* accessed,
                                     size_t num_accesses,
                                     struct inst_arg* elems,
                                     const struct inst_arg* replacement) {
    return (struct inst){
        .op = INST_REPLACEELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement = *replacement,
    };
}

void free_inst_arg(struct inst_arg* arg) {
    if (arg->type == INST_ARG_LITERAL) {
        free_inst_literal(&arg->lit);
    }
}

static void free_args(struct inst_arg* args, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        free_inst_arg(&args[i]);
    }
    free(args);
}

void free_inst(struct inst* tac) {
    switch (tac->op) {
        case INST_ADD:
        case INST_SUB:
        case INST_MUL:
        case INST_DIV:
        case INST_UDIV:
        case INST_AND:
        case INST_OR:
        case INST_XOR:
        case INST_LSHIFT:
        case INST_RSHIFT:
        case INST_MOD:
        case INST_EQ:
        case INST_NEQ:
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_STORE:
        case INST_LOAD:
            free_inst_arg(&tac->arg1);
            free_inst_arg(&tac->arg2);
            break;
        case INST_CALL:
            free_inst_arg(&tac->func);
            free_args(tac->func_args, tac->num_func_args);
            break;
        case INST_ASSIGN:
        case INST_CAST:
            free_inst_arg(&tac->arg1);
            break;
        case INST_ALLOCA:
            break;
        case INST_GETELEMPTR:
        case INST_GETELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            break;
        case INST_REPLACEELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            free_inst_arg(&tac->replacement);
            break;
    }
}

