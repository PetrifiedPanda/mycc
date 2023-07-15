#ifndef MYCC_FRONTEND_DECLARATION_STATIC_ASSERT_DECLARATION_H
#define MYCC_FRONTEND_DECLARATION_STATIC_ASSERT_DECLARATION_H

#include "frontend/ast/StringLiteralNode.h"

#include "frontend/parser/ParserState.h"

typedef struct ConstExpr ConstExpr;

typedef struct StaticAssertDeclaration {
    ConstExpr* const_expr;
    StringLiteralNode err_msg;
} StaticAssertDeclaration;

StaticAssertDeclaration* parse_static_assert_declaration(ParserState* s);

void StaticAssertDeclaration_free(StaticAssertDeclaration* d);

#endif

