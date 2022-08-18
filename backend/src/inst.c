#include "backend/inst.h"

#include <stdlib.h>
#include <assert.h>

void free_inst_type_info(struct inst_type_info* t) {
    switch (t->type) {
        case TAC_TYPE_BUILTIN:
        case TAC_TYPE_ARR:
            break;
        case TAC_TYPE_STRUCT:
            free(t->member_types);
            break;
        case TAC_TYPE_FUNC:
            free(t->arg_types);
            break;
    }
}

void free_inst_global_info(struct inst_global_info* g) {
    free(g->name);
}

void free_inst_reg_info(struct inst_reg_info* reg) {
    free(reg->name);
}

void free_inst_literal(struct inst_literal* lit) {
    switch (lit->type) {
        case TAC_LITERAL_STRUCT:
        case TAC_LITERAL_ARR:
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
        .op = TAC_CALL,
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
        .op = TAC_ASSIGN,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
        .arg2.type = TAC_ARG_NONE,
    };
}

struct inst create_cast_inst(const struct inst_type* type,
                             const struct inst_reg* dest,
                             const struct inst_arg* val) {
    return (struct inst){
        .op = TAC_CAST,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
    };
}

struct inst create_alloca_inst(const struct inst_reg* dest,
                               const struct inst_type* type) {
    return (struct inst){
        .op = TAC_ALLOCA,
        .type = *type,
        .dest = *dest,
        .arg1.type = TAC_ARG_NONE,
        .arg2.type = TAC_ARG_NONE,
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
        .op = TAC_STORE,
        .arg1 = *ptr,
        .arg2 = *to_store,
    };
}

struct inst create_load_inst(const struct inst_reg* dest,
                             const struct inst_arg* ptr) {
    return (struct inst){
        .op = TAC_LOAD,
        .dest = *dest,
        .arg1 = *ptr,
        .arg2.type = TAC_ARG_NONE,
    };
}

struct inst create_getelem_inst(const struct inst_reg* dest,
                                const struct inst_type* type,
                                const struct inst_arg* accessed,
                                size_t num_accesses,
                                struct inst_arg* elems) {
    return (struct inst){
        .op = TAC_GETELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = TAC_ARG_NONE,
    };
}

struct inst create_getelemptr_inst(const struct inst_reg* dest,
                                   const struct inst_type* type,
                                   const struct inst_arg* accessed,
                                   size_t num_accesses,
                                   struct inst_arg* elems) {
    return (struct inst){
        .op = TAC_GETELEMPTR,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = TAC_ARG_NONE,
    };
}

struct inst create_replace_elem_inst(const struct inst_reg* dest,
                                     const struct inst_type* type,
                                     const struct inst_arg* accessed,
                                     size_t num_accesses,
                                     struct inst_arg* elems,
                                     const struct inst_arg* replacement) {
    return (struct inst){
        .op = TAC_REPLACEELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement = *replacement,
    };
}

void free_inst_arg(struct inst_arg* arg) {
    if (arg->type == TAC_ARG_LITERAL) {
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
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_DIV:
        case TAC_UDIV:
        case TAC_AND:
        case TAC_OR:
        case TAC_XOR:
        case TAC_LSHIFT:
        case TAC_RSHIFT:
        case TAC_MOD:
        case TAC_EQ:
        case TAC_NEQ:
        case TAC_LT:
        case TAC_LE:
        case TAC_GT:
        case TAC_GE:
        case TAC_STORE:
        case TAC_LOAD:
            free_inst_arg(&tac->arg1);
            free_inst_arg(&tac->arg2);
            break;
        case TAC_CALL:
            free_inst_arg(&tac->func);
            free_args(tac->func_args, tac->num_func_args);
            break;
        case TAC_ASSIGN:
        case TAC_CAST:
            free_inst_arg(&tac->arg1);
            break;
        case TAC_ALLOCA:
            break;
        case TAC_GETELEMPTR:
        case TAC_GETELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            break;
        case TAC_REPLACEELEM:
            free_inst_arg(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            free_inst_arg(&tac->replacement);
            break;
    }
}

