#ifndef MYCC_BACKEND_IR_TYPE_H
#define MYCC_BACKEND_IR_TYPE_H


#include <stdint.h>

// TODO: pointer modifiers like restrict, volatile

typedef struct IRTypeRef {
    uint32_t id; // ID in LUT for types
} IRTypeRef;

typedef enum {
    INST_TYPE_BUILTIN,
    INST_TYPE_PTR,
    INST_TYPE_ARR,
    INST_TYPE_STRUCT,
    INST_TYPE_FUNC,
} IRTypeKind;

// TODO: builtins (could just be in enum)
typedef struct IRType {
    IRTypeKind type;
    union {
        struct {
            IRTypeRef base;
        } ptr_type;
        struct {
            IRTypeRef type;
            uint32_t len;
        } arr_type;
        struct {
            uint32_t num_members;
            IRTypeRef* member_types;
        } struct_type;
        struct {
            IRTypeRef ret_type;
            uint32_t num_args;
            IRTypeRef* arg_types;
        } func_type;
    };
} IRType;

void IRType_free(IRType* t);

#endif

