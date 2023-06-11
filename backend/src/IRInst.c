#include "backend/IRInst.h"

#include <assert.h>

#include "util/mem.h"

void IRGlobal_free(IRGlobal* g) {
    StrBuf_free(&g->name);
}

void IRReg_free(IRReg* reg) {
    StrBuf_free(&reg->name);
}

void IRLiteral_free(IRLiteral* lit) {
    switch (lit->type) {
        case IR_LITERAL_STRUCT:
        case IR_LITERAL_ARR:
            for (size_t i = 0; i < lit->num_members; ++i) {
                IRLiteral_free(&lit->members[i]);
            }
            mycc_free(lit->members);
            break;
        default:
            break;
    }
}

IRInst IRInst_create_call(const IRTypeRef* type,
                             const IRRegRef* dest,
                             const IRInstArg* func,
                             size_t num_func_args,
                             IRInstArg* func_args) {
    return (IRInst){
        .op = IR_INST_CALL,
        .type = *type,
        .dest = *dest,
        .func = *func,
        .num_func_args = num_func_args,
        .func_args = func_args,
    };
}

IRInst IRInst_create_assign(const IRTypeRef* type,
                               const IRRegRef* dest,
                               const IRInstArg* val) {
    return (IRInst){
        .op = IR_INST_ASSIGN,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

IRInst IRInst_create_cast(const IRTypeRef* type,
                             const IRRegRef* dest,
                             const IRInstArg* val) {
    return (IRInst){
        .op = IR_INST_CAST,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
    };
}

IRInst IRInst_create_alloca(const IRRegRef* dest,
                               const IRTypeRef* type) {
    return (IRInst){
        .op = IR_INST_ALLOCA,
        .type = *type,
        .dest = *dest,
        .arg1.type = IR_INST_ARG_NONE,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

IRInst IRInst_create(IRInstOp op,
                        const IRTypeRef* type,
                        const IRRegRef* dest,
                        const IRInstArg* arg1,
                        const IRInstArg* arg2) {
    return (IRInst){
        .op = op,
        .type = *type,
        .dest = *dest,
        .arg1 = *arg1,
        .arg2 = *arg2,
    };
}

IRInst IRInst_create_store(const IRInstArg* ptr,
                              const IRInstArg* to_store) {
    return (IRInst){
        .op = IR_INST_STORE,
        .arg1 = *ptr,
        .arg2 = *to_store,
    };
}

IRInst IRInst_create_load(const IRRegRef* dest,
                             const IRInstArg* ptr) {
    return (IRInst){
        .op = IR_INST_LOAD,
        .dest = *dest,
        .arg1 = *ptr,
        .arg2.type = IR_INST_ARG_NONE,
    };
}

IRInst IRInst_create_getelem(const IRRegRef* dest,
                                const IRTypeRef* type,
                                const IRInstArg* accessed,
                                size_t num_accesses,
                                IRInstArg* elems) {
    return (IRInst){
        .op = IR_INST_GETELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = IR_INST_ARG_NONE,
    };
}

IRInst IRInst_create_getelemptr(const IRRegRef* dest,
                                   const IRTypeRef* type,
                                   const IRInstArg* accessed,
                                   size_t num_accesses,
                                   IRInstArg* elems) {
    return (IRInst){
        .op = IR_INST_GETELEMPTR,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement.type = IR_INST_ARG_NONE,
    };
}

IRInst IrInst_create_replace_elem(const IRRegRef* dest,
                                     const IRTypeRef* type,
                                     const IRInstArg* accessed,
                                     size_t num_accesses,
                                     IRInstArg* elems,
                                     const IRInstArg* replacement) {
    return (IRInst){
        .op = IR_INST_REPLACEELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement = *replacement,
    };
}

void free_inst_arg(IRInstArg* arg) {
    if (arg->type == IR_INST_ARG_LITERAL) {
        IRLiteral_free(&arg->lit);
    }
}

static void free_args(IRInstArg* args, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        free_inst_arg(&args[i]);
    }
    mycc_free(args);
}

void IRInst_free(IRInst* tac) {
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

