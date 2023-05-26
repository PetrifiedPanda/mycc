#ifndef IR_INST_H
#define IR_INST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "util/Str.h"

#include "IRType.h"

typedef enum {
    IR_LITERAL_BOOL,
    IR_LITERAL_UINT,
    IR_LITERAL_INT,
    IR_LITERAL_FLOAT,
    IR_LITERAL_STRUCT,
    IR_LITERAL_ARR,
} IRLiteralType;

typedef struct IRLiteral {
    IRLiteralType type;
    union {
        bool bool_val;
        uintmax_t uint_val;
        intmax_t int_val;
        long double float_val;
        struct { // struct, array
            size_t num_members;
            struct IRLiteral* members;
        };
    };
} IRLiteral;

typedef struct {
    size_t id;
} IRRegRef;

typedef struct {
    size_t id;
} IRGlobalRef;

typedef struct {
    Str name;
    IRTypeRef type;
} IRReg;

typedef struct {
    Str name;
    IRTypeRef type;
    // TODO: value (if known at compile time)
} IRGlobal;

typedef enum IRInstArgType {
    IR_INST_ARG_NONE,
    IR_INST_ARG_LITERAL,
    IR_INST_ARG_REG,
    IR_INST_ARG_GLOBAL,
} IRInstArgType;

typedef struct {
    enum IRInstArgType type;
    union {
        IRLiteral lit;
        IRRegRef reg;
        IRGlobalRef global;
    };
} IRInstArg;

typedef enum {
    // dest = arg1 op arg2
    IR_INST_ADD,
    IR_INST_SUB,
    IR_INST_MUL,
    IR_INST_DIV,
    IR_INST_UDIV,
    IR_INST_AND,
    IR_INST_OR,
    IR_INST_XOR,
    IR_INST_LSHIFT,
    IR_INST_RSHIFT,
    IR_INST_MOD,
    IR_INST_EQ,
    IR_INST_NEQ,
    IR_INST_LT,
    IR_INST_LE,
    IR_INST_GT,
    IR_INST_GE,

    // dest = arg1
    IR_INST_ASSIGN,
    IR_INST_CAST,

    IR_INST_STORE, // *arg1 = arg2
    IR_INST_LOAD,  // dest = *arg1

    IR_INST_CALL, // dest = func(func_args)

    IR_INST_ALLOCA, // dest = alloca(sizeof(type))

    IR_INST_GETELEMPTR,
    IR_INST_GETELEM,
    IR_INST_REPLACEELEM,
} IRInstOp;

typedef struct {
    IRInstOp op;
    IRTypeRef type;
    IRRegRef dest;
    union {
        struct {
            IRInstArg arg1;
            IRInstArg arg2;
        };
        struct {
            IRInstArg func;
            size_t num_func_args;
            IRInstArg* func_args;
        };
        struct {
            IRInstArg accessed;
            size_t num_accesses;
            IRInstArg* elems;
            IRInstArg replacement;
        };
    };
} IRInst;

void IRGlobal_free(IRGlobal* g);
void IRReg_free(IRReg* reg);

void IRLiteral_free(IRLiteral* lit);

IRInst IRInst_create_call(const IRTypeRef* type,
                          const IRRegRef* dest,
                          const IRInstArg* func,
                          size_t num_args,
                          IRInstArg* func_args);

IRInst IRInst_create_assign(const IRTypeRef* type,
                            const IRRegRef* dest,
                            const IRInstArg* val);

IRInst IRInst_create_cast(const IRTypeRef* type,
                          const IRRegRef* dest,
                          const IRInstArg* val);

IRInst IRInst_create_alloca(const IRRegRef* dest, const IRTypeRef* type);

IRInst IRInst_create(IRInstOp op,
                     const IRTypeRef* type,
                     const IRRegRef* dest,
                     const IRInstArg* arg1,
                     const IRInstArg* arg2);

IRInst IRInst_create_load(const IRRegRef* dest, const IRInstArg* ptr);

IRInst IRInst_create_store(const IRInstArg* ptr, const IRInstArg* to_store);

IRInst IRInst_create_getelem(const IRRegRef* dest,
                             const IRTypeRef* type,
                             const IRInstArg* accessed,
                             size_t num_accesses,
                             IRInstArg* elems);

IRInst IRInst_create_getelemptr(const IRRegRef* dest,
                                const IRTypeRef* type,
                                const IRInstArg* accessed,
                                size_t num_accesses,
                                IRInstArg* elems);

IRInst IrInst_create_replace_elem(const IRRegRef* dest,
                                  const IRTypeRef* type,
                                  const IRInstArg* accessed,
                                  size_t num_accesses,
                                  IRInstArg* elems,
                                  const IRInstArg* replacement);
void IRInst_free(IRInst* tac);

#endif

