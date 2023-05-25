#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

#include "AstNodeInfo.h"

typedef struct Designator Designator;

typedef struct {
    size_t len;
    Designator* designators;
} DesignatorList;

typedef struct DesignationInit DesignationInit;

typedef struct {
    size_t len;
    DesignationInit* inits;
} InitList;

typedef struct AssignExpr AssignExpr;

typedef struct Initializer {
    AstNodeInfo info;
    bool is_assign;
    union {
        AssignExpr* assign;
        InitList init_list;
    };
} Initializer;

typedef struct ConstExpr ConstExpr;
typedef struct Identifier Identifier;

typedef struct Designator {
    AstNodeInfo info;
    bool is_index;
    union {
        ConstExpr* arr_index;
        Identifier* identifier;
    };
} Designator;

typedef struct {
    DesignatorList designators;
} Designation;

typedef struct DesignationInit {
    Designation designation;
    Initializer init;
} DesignationInit;

Initializer* parse_initializer(ParserState* s);
bool parse_init_list(ParserState* s, InitList* res);

Designation create_invalid_designation(void);

void free_initializer_children(Initializer* i);
void free_initializer(Initializer* i);

void free_designator_children(Designator* d);

void free_designator_list(DesignatorList* l);

bool is_valid_designation(const Designation* d);

void free_designation(Designation* d);
void free_designation_children(Designation* d);

void free_init_list_children(InitList* l);

#include "frontend/ast/expr/AssignExpr.h"
#include "frontend/ast/expr/ConstExpr.h"

#include "frontend/ast/Identifier.h"

#endif
