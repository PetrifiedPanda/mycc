#include "backend/tac.h"

#include <stdlib.h>
#include <assert.h>

void tac_type_info_free(struct tac_type_info* t) {
    switch (t->type) {
        case TAC_TYPE_BUILTIN:
        case TAC_TYPE_ARR:
            break;
        case TAC_TYPE_STRUCT:
            free(t->member_types);
            break;
    }
}

void tac_reg_info_free(struct tac_reg_info* reg) {
    free(reg->name);
}

void tac_literal_free(struct tac_literal* lit) {
    switch (lit->type) {
        case TAC_LITERAL_STRUCT:
        case TAC_LITERAL_ARR:
            for (size_t i = 0; i < lit->num_members; ++i) {
                tac_literal_free(&lit->members[i]);
            }
            free(lit->members);
            break;
        default:
            break;
    }
}

struct tac tac_create_call(const struct tac_type* type,
                           const struct tac_reg* dest,
                           const struct tac_arg* func,
                           size_t num_func_args,
                           struct tac_arg* func_args) {
    return (struct tac) {
        .op = TAC_CALL,
        .type = *type,
        .dest = *dest,
        .func = *func,
        .num_func_args = num_func_args,
        .func_args = func_args,
    };
}

struct tac tac_create_assign(const struct tac_type* type,
                             const struct tac_reg* dest,
                             const struct tac_arg* val) {
    return (struct tac) {
        .op = TAC_ASSIGN,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
        .arg2.type = TAC_ARG_NONE,
    };
}

struct tac tac_create_cast(const struct tac_type* type,
                           const struct tac_reg* dest,
                           const struct tac_arg* val) {
    return (struct tac) {
        .op = TAC_CAST,
        .type = *type,
        .dest = *dest,
        .arg1 = *val,
    };
}

struct tac tac_create_alloca(const struct tac_reg* dest, const struct tac_type* type) {
    return (struct tac){
        .op = TAC_ALLOCA,
        .type = *type,
        .dest = *dest,
        .arg1.type = TAC_ARG_NONE,
        .arg2.type = TAC_ARG_NONE,
    };
}

struct tac tac_create(enum tac_op op,
                      const struct tac_type* type,
                      const struct tac_reg* dest,
                      const struct tac_arg* arg1,
                      const struct tac_arg* arg2) {
    assert(op != TAC_CALL);
    assert(op != TAC_ASSIGN);
    assert(op != TAC_ALLOCA);
    return (struct tac) {
        .op = op,
        .type = *type,
        .dest = *dest,
        .arg1 = *arg1,
        .arg2 = *arg2,
    };
}

struct tac tac_create_getelem(const struct tac_reg* dest,
                              const struct tac_type* type,
                              const struct tac_arg* accessed,
                              size_t num_accesses,
                              struct tac_arg* elems) {
    return (struct tac) {
        .op = TAC_GETELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
    };
}

struct tac tac_create_getelemptr(const struct tac_reg* dest,
                                 const struct tac_type* type,
                                 const struct tac_arg* accessed,
                                 size_t num_accesses,
                                 struct tac_arg* elems) {
    return (struct tac) {
        .op = TAC_GETELEMPTR,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
    };
}

struct tac tac_create_replace_elem(const struct tac_reg* dest,
                                   const struct tac_type* type,
                                   const struct tac_arg* accessed,
                                   size_t num_accesses,
                                   struct tac_arg* elems,
                                   const struct tac_arg* replacement) {
    return (struct tac) {
        .op = TAC_REPLACEELEM,
        .type = *type,
        .dest = *dest,
        .accessed = *accessed,
        .num_accesses = num_accesses,
        .elems = elems,
        .replacement = *replacement,
    };
}

void tac_arg_free(struct tac_arg* arg) {
    if (arg->type == TAC_ARG_LITERAL) {
        tac_literal_free(&arg->lit);
    }
}

static void free_args(struct tac_arg* args, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        tac_arg_free(&args[i]);
    }
    free(args);
}

void tac_free(struct tac* tac) {
    switch (tac->op) {
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_DIV:
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
            tac_arg_free(&tac->arg1);
            tac_arg_free(&tac->arg2);
            break;
        case TAC_CALL:
            tac_arg_free(&tac->func);
            free_args(tac->func_args, tac->num_func_args);
            break;
        case TAC_ASSIGN:
        case TAC_CAST:
            tac_arg_free(&tac->arg1);
            break;
        case TAC_ALLOCA:
            break;
        case TAC_GETELEMPTR:
        case TAC_GETELEM:
            tac_arg_free(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            break;
        case TAC_REPLACEELEM:
            tac_arg_free(&tac->accessed);
            free_args(tac->elems, tac->num_accesses);
            tac_arg_free(&tac->replacement);
            break;
    }
}

