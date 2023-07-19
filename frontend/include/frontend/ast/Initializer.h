#ifndef MYCC_FRONTEND_AST_INITIALIZER_H
#define MYCC_FRONTEND_AST_INITIALIZER_H

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

void Initializer_free_children(Initializer* i);
void Initializer_free(Initializer* i);

void Designator_free_children(Designator* d);

void DesignatorList_free(DesignatorList* l);

bool Designation_is_valid(const Designation* d);

void Designation_free(Designation* d);
void Designation_free_children(Designation* d);

void InitList_free_children(InitList* l);

#endif
