#ifndef MYCC_FRONTEND_AST_AST_H
#define MYCC_FRONTEND_AST_AST_H

typedef enum {
    // subrange (func_def | declaration)[lhs...rhs]
    AST_TRANSLATION_UNIT,
    // lhs func_signature rhs compound_statement
    AST_FUNC_DEF,
    // lhs declaration_specs, rhs is init_declarator_list 
    AST_DECLARATION,
    // lhs is expr, rhs is String literal
    AST_STATIC_ASSERT_DECLARATION,
    // lhs is Declaration(?), rhs is DeclarationList
    AST_FUNC_SIGNATURE,
    // subrange (declaration | statement)[lhs...rhs] 
    AST_COMPOUND_STATEMENT,
    // labeled_statement | compound_statement | expr_statement | selection_statement | iteration_statement | jump_statement (might be removeable)
    AST_STATEMENT,
    // lhs identifier: rhs statement
    AST_LABELED_STATEMENT,
    // case lhs const_expr: rhs statement
    AST_CASE_STATEMENT,
    // default: rhs (merge with case statement?)
    AST_DEFAULT_STATEMENT,
    // expr (might be removeable)
    AST_EXPR_STATEMENT,
    // switch (lhs expr) rhs statement
    AST_SWITCH_STATEMENT,
    // if (lhs expr) rhs statement
    AST_SIMPLE_IF,
    // lhs simple_if else rhs statement
    AST_IF_ELSE,
    // while (lhs expr) rhs statement
    AST_WHILE,
    // lhs expr while (rhs expr);
    AST_DO_WHILE,
    // for (lhs for_clause) rhs statement
    AST_FOR,
    // lhs (expr_statement | declaration) rhs for_loop_actions 
    AST_FOR_CLAUSE,
    // lhs expr_statement rhs expr
    AST_FOR_LOOP_ACTIONS,
    // goto lhs identifier;
    AST_GOTO_STATEMENT,
    // continue;
    AST_CONTINUE_STATEMENT,
    // break;
    AST_BREAK_STATEMENT,
    // return lhs expr;
    AST_RETURN_STATEMENT,
    // lhs (storage_class_spec | type_spec | func_spec | align_spec) rhs declaration_specifiers
    AST_DECLARATION_SPECS,
    // typedef extern static thread_local auto register
    AST_STORAGE_CLASS_SPEC,
    // lhs ?(identifier | enum_spec | struct_union_spec | atomic_type_spec) (otherwise void char int float double bool atomic struct enum)
    AST_TYPE_SPEC,
    // enum lhs ?identifier rhs enum_list 
    AST_ENUM_SPEC,
    // subrange enumerator[lhs...rhs]
    AST_ENUM_LIST,
    // lhs identifier = rhs ?const_expr
    AST_ENUMERATOR,
    // (struct | union) lhs ?identifier rhs ?struct_declaration_list
    // needs either identifier or struct_declaration_list
    AST_STRUCT_SPEC,
    AST_UNION_SPEC,
    // subrange (struct_declaration | static_assert_declaration)[lhs...rhs]
    AST_STRUCT_DECLARATION_LIST,
    // lhs declaration_specs rhs struct_declarator_list
    AST_STRUCT_DECLARATION,
    // subrange struct_declarator[lhs...rhs]
    AST_STRUCT_DECLARATOR_LIST,
    // lhs declarator : rhs const_expr;
    AST_STRUCT_DECLARATOR,
    // _Atomic(lhs type_name)
    AST_ATOMIC_TYPE_SPEC,
    // inline noreturn
    AST_FUNC_SPEC,
    // lhs type_name
    AST_ALIGN_SPEC_TYPE,
    // lhs const_expr
    AST_ALIGN_SPEC_BYTES,
    // subrange init_declarator[lhs...rhs]
    AST_INIT_DECLARATOR_LIST,
    // lhs declarator, rhs initializer
    AST_INIT_DECLARATOR,
    // lhs pointer, rhs direct_declarator
    AST_DECLARATOR,
    // * lhs type_qualifier_list
    AST_POINTER,
    // lhs ?type_qual_list (const restrict volatile atomic)
    AST_TYPE_QUAL_LIST,
    // lhs (identifier | declarator) rhs arr_or_func_suffix_list
    AST_DIRECT_DECLARATOR,
    // TODO: remove
    // subrange (arr_suffix | func_suffix)[lhs...rhs]
    AST_ARR_OR_FUNC_SUFFIX_LIST,
    // lhs ?type_qual_list rhs ?assign_expr
    AST_ARR_SUFFIX,
    // lhs ?type_qual_list rhs assign_expr
    AST_ARR_SUFFIX_STATIC,
    // lhs ?type_qual_list
    AST_ARR_SUFFIX_ASTERISK,
    // lhs param_type_list
    AST_FUNC_SUFFIX,
    // lhs identifier_list
    AST_FUNC_SUFFIX_OLD,
    // subrange identifier[lhs...rhs]
    AST_IDENTIFIER_LIST,
    // subrange param_declaration[lhs...rhs]
    AST_PARAM_TYPE_LIST,
    AST_PARAM_TYPE_LIST_VARIADIC,
    // lhs declaration_specs rhs (declarator | ?abs_declarator)
    AST_PARAM_DECLARATION,
    // lhs pointer rhs direct_abs_declarator
    AST_ABS_DECLARATOR,
    // lhs (abs_declarator | ?direct_abs_declarator) rhs (abs_arr_suffix | abs_func_suffix)
    AST_DIRECT_ABS_DECLARATOR,
    // lhs ?type_qual_list rhs ?assign_expr 
    AST_ABS_ARR_SUFFIX,
    // lhs ?type_qual_list rhs assign_expr 
    AST_ABS_ARR_SUFFIX_STATIC,
    // none
    AST_ABS_ARR_SUFFIX_ASTERISK,
    // param_type_list (might be removeable)
    AST_ABS_FUNC_SUFFIX,
    // Eiteher assign expr or init_list (might be removeable)
    AST_INITIALIZER,    
    // subrange designation_init[lhs...rhs]
    AST_INIT_LIST,
    // lhs designator_list, rhs initializer
    AST_DESIGNATION_INIT,
    // subrange designator[lhs...rhs]
    AST_DESIGNATOR_LIST,
    // constexpr or identifier (might be removeable)
    AST_DESIGNATOR,
    // lhs assign_expr, rhs ?assign_expr
    AST_EXPR,
    // lhs (assign_expr | cond_expr) assign_op rhs cond_expr
    AST_ASSIGN,
    AST_ASSIGN_MUL,
    AST_ASSIGN_DIV,
    AST_ASSIGN_MOD,
    AST_ASSIGN_ADD,
    AST_ASSIGN_SUB,
    AST_ASSIGN_SHL,
    AST_ASSIGN_SHR,
    AST_ASSIGN_AND,
    AST_ASSIGN_XOR,
    AST_ASSIGN_OR,
    // cond_expr (might be removeable)
    AST_CONST_EXPR,
    // lhs log_or_expr ? rhs cond_items
    AST_COND_EXPR,
    // rhs (expr | cond_expr) : lhs (cond_expr | expr)
    AST_COND_ITEMS,
    // lhs (log_or_expr | log_and_expr) || rhs log_and_expr
    AST_LOG_OR_EXPR,
    // lhs (log_and_expr | or_expr) && rhs or_expr
    AST_LOG_AND_EXPR,
    // lhs (or_expr | xor_expr) | rhs xor_expr
    AST_OR_EXPR,
    // lhs (xor_expr | and_expr) ^ rhs and_expr
    AST_XOR_EXPR,
    // lhs (and_expr | eq_expr) & rhs eq_expr
    AST_AND_EXPR,
    // lhs (eq_expr | rel_expr) eq_op rhs (rel_expr)
    AST_EQ_EXPR,
    AST_NE_EXPR,
    // lhs (rel_expr | shift_expr) rel_op rhs shift_expr
    AST_REL_EXPR_LT,
    AST_REL_EXPR_GT,
    AST_REL_EXPR_LE,
    AST_REL_EXPR_GE,
    // lhs (shift_expr | add_expr) shift_op rhs add_expr
    AST_SHIFT_EXPR_LEFT,
    AST_SHIFT_EXPR_RIGHT,
    // lhs (add_expr | mul_expr) add_expr rhs mul_expr
    AST_ADD_EXPR,
    AST_SUB_EXPR,
    // lhs (mul_expr | cast_expr) mul_epxr rhs cast_expr
    AST_MUL_EXPR,
    AST_DIV_EXPR,
    AST_MOD_EXPR,
    // (lhs type_name)rhs unary_expr
    AST_CAST_EXPR,
    // lhs spec_qual_list, rhs abs_declartor
    AST_TYPE_NAME,
    // unary_op, rhs (postfix_expr | cast_expr | type_name) (TODO: lhs)
    AST_UNARY_EXPR_INC,
    AST_UNARY_EXPR_DEC,
    AST_UNARY_EXPR_SIZEOF,
    // lhs postfix_expr[rhs expr]
    AST_POSTFIX_EXPR_INDEX,
    // lhs postfix_expr(rhs arg_expr_list)
    AST_POSTFIX_EXPR_BRACKET,
    // lhs postfix_expr . rhs identifier
    AST_POSTFIX_ACCESS,
    // lhs postfix_expr -> rhs identifier
    AST_POSTFIX_PTR_ACCESS,
    // lhs postfix_expr ++
    AST_POSTFIX_INC,
    // lhs postfix_expr --
    AST_POSTFIX_DEC,
    // subrange assign_expr[lhs...rhs]
    AST_ARG_EXPR_LIST,
    // main token is identifier
    AST_IDENTIFIER,
} ASTNodeKind;

typedef struct {
    ASTNodeKind kind;
    int main_token;
    int rhs;
} ASTNode;

#include "TranslationUnit.h"
#include "declaration/ExternalDeclaration.h"
#include "declaration/DeclarationSpecs.h"
#include "declaration/AtomicTypeSpec.h"
#include "declaration/StructUnionSpec.h"
#include "declaration/EnumSpec.h"
#include "declaration/AlignSpec.h"
#include "TypeName.h"
#include "SpecQualList.h"
#include "declaration/AbsDeclarator.h"
#include "declaration/Pointer.h"
#include "declaration/TypeQuals.h"
#include "declaration/DirectAbsDeclarator.h"
#include "AssignExpr.h"
#include "declaration/Declarator.h"
#include "declaration/DirectDeclarator.h"
#include "Identifier.h"
#include "Expr.h"
#include "declaration/StaticAssertDeclaration.h"
#include "declaration/InitDeclarator.h"
#include "Initializer.h"

#endif
