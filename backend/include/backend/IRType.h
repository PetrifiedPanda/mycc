#ifndef MYCC_BACKEND_IR_TYPE_H
#define MYCC_BACKEND_IR_TYPE_H


#include <stddef.h>

// TODO: pointer modifiers like restrict, volatile

typedef struct {
    size_t id; // ID in LUT for types
    size_t num_indirs;
} IRTypeRef;

typedef enum {
    INST_TYPE_BUILTIN,
    INST_TYPE_ARR,
    INST_TYPE_STRUCT,
    INST_TYPE_FUNC,
} IRTypeKind;

// TODO: builtins (could just be in enum)
typedef struct {
    IRTypeKind type;
    union {
        struct {
            IRTypeRef type;
            size_t len;
        } arr_type;
        struct {
            size_t num_members;
            IRTypeRef* member_types;
        } struct_type;
        struct {
            IRTypeRef ret_type;
            size_t num_args;
            IRTypeRef* arg_types;
        } func_type;
    };
} IRType;

void IRType_free(IRType* t);

#endif

