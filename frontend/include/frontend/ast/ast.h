#ifndef MYCC_FRONTEND_AST_AST_H
#define MYCC_FRONTEND_AST_AST_H

#include <stdint.h>
#include <stdbool.h>

#include "frontend/Token.h"
#include "frontend/parser/ParserErr.h"

/**
 * Special node types
 * subrange ast_node_kind[lhs...rhs] the following nodes will all be of type type and rhs will be the next node after the list
 * token_range token_kind[token_idx...rhs] rhs is the length of tokens that all have the same type
 */
typedef enum {
    // subrange (func_def | declaration | attribute_declaration | static_assert_declaration)[lhs...rhs]
    AST_TRANSLATION_UNIT = 0,
    // lhs ?attribute_spec_sequence rhs func_def_impl TODO: rename func_def_impl 
    AST_FUNC_DEF,
    // lhs declaration_specs rhs func_def_sub_impl TODO: rename func_def_sub_impl
    AST_FUNC_DEF_IMPL,
    // lhs func_declarator_and_decl_list rhs compound_statement 
    AST_FUNC_DEF_SUB_IMPL,
    // lhs declarator rhs ?declaration_list
    AST_FUNC_DECLARATOR_AND_DECL_LIST,
    // lhs ?attribute_spec_sequence rhs declaration_specs_and_init_declarator_list 
    AST_DECLARATION,
    // subrange (declaration | static_assert_declaration | attribute_declaration)[lhs...rhs]
    AST_DECLARATION_LIST,
    // lhs declaration_specs rhs ?init_declarator_list
    AST_DECLARATION_SPECS_AND_INIT_DECLARATOR_LIST,
    // TODO: might not be needed
    // lhs attribute_spec_sequence ';'
    AST_ATTRIBUTE_DECLARATION,
    // lhs expr rhs string_literal 
    AST_STATIC_ASSERT_DECLARATION,
    // subrange (declaration | static_assert_declaration | attribute_statement | expr_statement | unlabeled_statement | label)[lhs...rhs] 
    AST_COMPOUND_STATEMENT,
    // TODO: comment out of date
    // lhs ?attribute_spec_sequence rhs identifier ':'
    // lhs ?attribute_spec_sequence 'case' rhs ?const_expr ':'
    // lhs ?attribute_spec_sequence 'default' ':'
    AST_LABEL,
    // statement:
    // lhs ?attribute_spec_sequence rhs ((?expr ';') | compound_statement | selection_statement | iteration_statement | jump_statement)
    AST_UNLABELED_STATEMENT,
    // lhs ?attribute_spec_sequence rhs (labeled_statement_label | labeled_statement_case)
    AST_LABELED_STATEMENT,
    // lhs (identifier ':') rhs statement
    AST_LABELED_STATEMENT_LABEL,
    // 'case' lhs const_expr ':' rhs statement
    // 'default' lhs empty ':' rhs statement
    AST_LABELED_STATEMENT_CASE,
    // sel_statement:
    // 'switch' '(' lhs expr ')' rhs statement
    AST_SWITCH_STATEMENT,
    // 'if' '(' lhs expr ')' rhs statement
    AST_SIMPLE_IF,
    // lhs simple_if else rhs statement
    AST_IF_ELSE,
    // iteration_statement:
    // 'while' '(' lhs expr ')' rhs statement
    AST_WHILE,
    // 'do' lhs statement while '(' rhs expr ')' ';'
    AST_DO_WHILE,
    // for '('lhs for_clause')' rhs statement
    AST_FOR,
    // lhs (expr_statement | declaration) rhs for_loop_actions 
    AST_FOR_CLAUSE,
    // lhs expr ';' rhs expr
    AST_FOR_LOOP_ACTIONS,
    // 'goto' lhs identifier ';'
    AST_GOTO_STATEMENT,
    // 'continue' ';'
    AST_CONTINUE_STATEMENT,
    // 'break' ';'
    AST_BREAK_STATEMENT,
    // 'return' rhs ?expr ';'
    AST_RETURN_STATEMENT,
    // subrange (storage_class_spec | type_spec | func_spec | align_spec)[lhs...rhs] rhs ?attribute_spec_sequence
    AST_DECLARATION_SPECS,
    // main token is (typedef extern static thread_local auto register)
    AST_STORAGE_CLASS_SPEC_TYPEDEF,
    AST_STORAGE_CLASS_SPEC_EXTERN,
    AST_STORAGE_CLASS_SPEC_STATIC,
    AST_STORAGE_CLASS_SPEC_THREAD_LOCAL,
    AST_STORAGE_CLASS_SPEC_AUTO,
    AST_STORAGE_CLASS_SPEC_REGISTER,
    // main token is type spec 
    AST_TYPE_SPEC_VOID,
    AST_TYPE_SPEC_CHAR,
    AST_TYPE_SPEC_SHORT,
    AST_TYPE_SPEC_INT,
    AST_TYPE_SPEC_LONG,
    AST_TYPE_SPEC_FLOAT,
    AST_TYPE_SPEC_DOUBLE,
    AST_TYPE_SPEC_SIGNED,
    AST_TYPE_SPEC_UNSIGNED,
    AST_TYPE_SPEC_BOOL,
    AST_TYPE_SPEC_COMPLEX,
    AST_TYPE_SPEC_IMAGINARY,
    // main token is identifier
    AST_TYPE_SPEC_TYPEDEF_NAME,
    // 'enum' lhs ?attribute_id rhs ?enum_body
    AST_ENUM_SPEC,
    // lhs ?attribute_spec_sequence rhs ?identifier
    AST_ATTRIBUTE_ID,
    // lhs ?specifier_qualifier_list rhs ?enum_list
    AST_ENUM_BODY,
    // subrange enumerator[lhs...rhs]
    AST_ENUM_LIST,
    // lhs enum_constant_and_attribute '=' rhs ?const_expr
    AST_ENUMERATOR,
    // lhs enum_constant rhs ?attribute_spec_sequence
    AST_ENUM_CONSTANT_AND_ATTRIBUTE,
    // ('struct' | 'union') lhs ?attribute_spec_sequence rhs struct_union_body
    AST_STRUCT_SPEC,
    AST_UNION_SPEC,
    // needs either identifier or member_declaration_list
    // lhs ?identifier rhs ?member_declaration_list
    AST_STRUCT_UNION_BODY,
    // subrange (member_declaration | static_assert_declaration)[lhs...rhs]
    AST_MEMBER_DECLARATION_LIST,
    // lhs ?attribute_spec_sequence rhs member_declaration_body
    AST_MEMBER_DECLARATION,
    // lhs spec_qual_list rhs ?member_declarator_list ';'
    AST_MEMBER_DECLARATION_BODY,
    // subrange member_declarator[lhs...rhs]
    AST_MEMBER_DECLARATOR_LIST,
    // lhs ?declarator ':' rhs ?const_expr;
    AST_MEMBER_DECLARATOR,
    // '_Atomic' '(' lhs type_name ')'
    AST_ATOMIC_TYPE_SPEC,
    // main_token is inline / noreturn
    AST_FUNC_SPEC_INLINE,
    AST_FUNC_SPEC_NORETURN,
    // lhs type_name
    // lhs const_expr
    AST_ALIGN_SPEC,
    // subrange init_declarator[lhs...rhs]
    AST_INIT_DECLARATOR_LIST,
    // lhs declarator rhs initializer
    AST_INIT_DECLARATOR,
    // lhs ?pointer rhs direct_declarator
    AST_DECLARATOR,
    // '*' lhs ?pointer_attrs_and_quals rhs ?pointer
    AST_POINTER,
    // lhs ?attribute_spec_sequence rhs ?type_qual_list
    AST_POINTER_ATTRS_AND_QUALS,
    // subrange attribute_spec[lhs...rhs]
    AST_ATTRIBUTE_SPEC_SEQUENCE,
    // lhs attribute_list
    AST_ATTRIBUTE_SPEC,
    // subrange attribute[lhs...rhs]
    AST_ATTRIBUTE_LIST,
    // lhs (attribute_prefixed_token | identifier) rhs ?attribute_arg_clause
    AST_ATTRIBUTE,
    // lhs identifier rhs identifier
    AST_ATTRIBUTE_PREFIXED_TOKEN,
    // '(' rhs ?balanced_token_sequence ')'
    AST_ATTRIBUTE_ARGUMENT_CLAUSE,
    // subrange balanced_token[lhs...rhs]
    AST_BALANCED_TOKEN_SEQUENCE,
    // '(' rhs ?balanced_token_sequence ')'
    // '[' rhs ?balanced_token_sequence ']'
    // '{' rhs ?balanced_token_sequence '}'
    AST_BALANCED_TOKEN_BRACKET,
    // just a token
    AST_BALANCED_TOKEN,
    // token_range type_qual[lhs...rhs]
    AST_TYPE_QUAL_LIST,
    // lhs id_attribute rhs arr_or_func_suffix_list
    AST_DIRECT_DECLARATOR,
    // lhs identifier rhs ?attribute_spec_sequence
    AST_ID_ATTRIBUTE,
    // subrange (arr_suffix | func_suffix)[lhs...rhs]
    AST_ARR_OR_FUNC_SUFFIX_LIST,
    // lhs (arr_suffix | func_suffix) rhs ?attribute_spec_sequence
    AST_ARR_OR_FUNC_SUFFIX,
    // lhs ?type_qual_list rhs ?assign_expr
    AST_ARR_SUFFIX,
    // lhs ?type_qual_list rhs assign_expr
    AST_ARR_SUFFIX_STATIC,
    // rhs ?type_qual_list
    AST_ARR_SUFFIX_ASTERISK,
    // rhs ?param_type_list
    AST_FUNC_SUFFIX,
    // lhs identifier_list
    AST_FUNC_SUFFIX_OLD,
    // subrange identifier[lhs...rhs]
    AST_IDENTIFIER_LIST,
    // subrange param_declaration[lhs...rhs]
    AST_PARAM_TYPE_LIST,
    AST_PARAM_TYPE_LIST_VARIADIC,
    // lhs attrs_and_declaration_specs rhs (declarator | ?abs_declarator)
    AST_PARAM_DECLARATION,
    // lhs ?attribute_spec_sequence rhs declaration_specs
    AST_ATTRS_AND_DECLARATION_SPECS,
    // lhs ?pointer rhs ?direct_abs_declarator
    AST_ABS_DECLARATOR,
    // lhs (abs_declarator | ?direct_abs_declarator) rhs (abs_arr_suffix | abs_func_suffix)
    AST_DIRECT_ABS_DECLARATOR,
    // subrange abs_arr_or_func_suffix[lhs...rhs]
    AST_ABS_ARR_OR_FUNC_SUFFIX_LIST,
    // lhs (abs_arr_suffix | func_suffix) rhs ?attribute_spec_sequence
    AST_ABS_ARR_OR_FUNC_SUFFIX,
    // lhs ?type_qual_list rhs ?assign_expr 
    AST_ABS_ARR_SUFFIX,
    // lhs ?type_qual_list rhs assign_expr 
    AST_ABS_ARR_SUFFIX_STATIC,
    // none
    AST_ABS_ARR_SUFFIX_ASTERISK,
    // param_type_list (TODO: might be removeable)
    AST_ABS_FUNC_SUFFIX,
    // lhs assign_expr | braced_initializer (might be removeable)
    AST_INITIALIZER,    
    // subrange designation_init[lhs...rhs]
    AST_INIT_LIST,
    // lhs ?designator_list, rhs initializer
    AST_DESIGNATION_INIT,
    // subrange designator[lhs...rhs]
    AST_DESIGNATOR_LIST,
    // lhs (const_expr | identifier) (might be removeable)
    AST_DESIGNATOR,
    // cond_expr (might be removeable)
    AST_CONST_EXPR,
    // lhs assign_expr ',' rhs ?assign_expr
    AST_EXPR,
    // assign_expr:
    // lhs cond_expr '=' rhs (assign_expr | cond_expr)
    AST_ASSIGN,
    // lhs cond_expr '*=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_MUL,
    // lhs cond_expr '/=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_DIV,
    // lhs cond_expr '%=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_MOD,
    // lhs cond_expr '+=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_ADD,
    // lhs cond_expr '-=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_SUB,
    // lhs cond_expr '<<=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_SHL,
    // lhs cond_expr '>>=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_SHR,
    // lhs cond_expr '&=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_AND,
    // lhs cond_expr '^=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_XOR,
    // lhs cond_expr '|=' rhs (assign_expr | cond_expr)
    AST_ASSIGN_OR,
    // cond_expr:
    // lhs log_or_expr '?' rhs cond_items
    AST_COND_EXPR,
    // rhs (cond_expr | expr) ':' lhs (cond_expr | expr)
    AST_COND_ITEMS,
    // lhs log_and_expr '||' rhs (log_or_expr | log_and_expr)
    AST_LOG_OR_EXPR,
    // lhs or_expr '&&' rhs (log_and_expr | or_expr)
    AST_LOG_AND_EXPR,
    // lhs xor_expr '|' rhs (or_expr | xor_expr)
    AST_OR_EXPR,
    // lhs and_expr '^' rhs (xor_expr | and_expr)
    AST_XOR_EXPR,
    // lhs eq_expr '&' rhs (and_expr | eq_expr)
    AST_AND_EXPR,
    // eq_expr:
    // lhs rel_expr '==' rhs (eq_expr | rel_expr)
    AST_EQ_EXPR,
    // lhs rel_expr '!=' rhs (eq_expr | rel_expr)
    AST_NE_EXPR,
    // rel_expr:
    // lhs shift_expr '<' rhs (rel_expr | shift_expr)
    AST_REL_EXPR_LT,
    // lhs shift_expr '>' rhs (rel_expr | shift_expr)
    AST_REL_EXPR_GT,
    // lhs shift_expr '<=' rhs (rel_expr | shift_expr)
    AST_REL_EXPR_LE,
    // lhs shift_expr '>=' rhs (rel_expr | shift_expr)
    AST_REL_EXPR_GE,
    // shift_expr
    // lhs add_expr '<<' rhs (shift_expr | add_expr)
    AST_LSHIFT_EXPR,
    // lhs add_expr '>>' rhs (shift_expr | add_expr)
    AST_RSHIFT_EXPR,
    // add_expr:
    // lhs mul_expr '+' rhs (add_expr | mul_expr)
    AST_ADD_EXPR,
    // lhs mul_expr '-' rhs (add_expr | mul_expr)
    AST_SUB_EXPR,
    // mul_expr:
    // lhs cast_expr '*' rhs (mul_expr | cast_expr)
    AST_MUL_EXPR,
    // lhs cast_expr '/' rhs (mul_expr | cast_expr)
    AST_DIV_EXPR,
    // lhs cast_expr '%' rhs (mul_expr | cast_expr)
    AST_MOD_EXPR,
    // '('lhs ?type_name')' (rhs cast_expr | unary_expr) 
    AST_CAST_EXPR,
    // lhs spec_qual_list rhs ?abs_declartor
    AST_TYPE_NAME,
    // lhs spec_qual_list rhs ?attribute_spec_sequence
    AST_SPEC_QUAL_LIST_ATTR,
    // subrange type_spec_qual[lhs...rhs]
    AST_SPEC_QUAL_LIST,
    // main_token is type qualifier
    AST_TYPE_QUAL_CONST,
    AST_TYPE_QUAL_RESTRICT,
    AST_TYPE_QUAL_VOLATILE,
    AST_TYPE_QUAL_ATOMIC,
    // unary_expr:
    // '++' lhs (unary_expr | postfix_expr)
    AST_UNARY_EXPR_INC,
    // '--' lhs (unary_expr | postfix_expr)
    AST_UNARY_EXPR_DEC,
    // 'sizeof' lhs (unary_expr | postfix_expr | type_name)
    AST_UNARY_EXPR_SIZEOF,
    // '_Alignof' (lhs type_name)
    AST_UNARY_EXPR_ALIGNOF,
    // '&' lhs cast_expr
    AST_UNARY_EXPR_ADDRESSOF,
    // '*' lhs cast_expr
    AST_UNARY_EXPR_DEREF,
    // '+' lhs cast_expr
    AST_UNARY_EXPR_PLUS,
    // '-' lhs cast_expr
    AST_UNARY_EXPR_MINUS,
    // '~' lhs cast_expr
    AST_UNARY_EXPR_BNOT,
    // '!' lhs cast_expr
    AST_UNARY_EXPR_NOT,
    // lhs primary_expr | compound_literal rhs ?postfix_op
    AST_POSTFIX_EXPR,
    // postfix_op:
    // '['lhs expr']' rhs ?postfix_op
    AST_POSTFIX_OP_INDEX,
    // '('lhs arg_expr_list')' rhs ?postfix_op
    AST_POSTFIX_OP_CALL,
    // '.' lhs identifier rhs ?postfix_op
    AST_POSTFIX_OP_ACCESS,
    // '->' lhs identifier rhs ?postfix_op
    AST_POSTFIX_OP_PTR_ACCESS,
    // main_token is either inc or dec
    AST_POSTFIX_OP_INC,
    AST_POSTFIX_OP_DEC,
    // subrange assign_expr[lhs...rhs]
    AST_ARG_EXPR_LIST,
    // constant | string_constant | identifier | bracket_expr | generic_sel
    AST_PRIMARY_EXPR,
    // '_Generic' '(' lhs assign_expr ',' rhs generic_assoc_list ')'
    AST_GENERIC_SEL,
    // subrange generic_assoc[lhs...rhs] 
    AST_GENERIC_ASSOC_LIST,
    // lhs ?type_name ':' assign_expr
    AST_GENERIC_ASSOC,
    // main token is value / identifier
    AST_CONSTANT,
    AST_ENUM_CONSTANT,
    // main token is string literal / __func__
    AST_STRING_LITERAL,
    AST_FUNC,
    // '(' lhs compound_literal_type ')' rhs braced_initializer
    AST_COMPOUND_LITERAL,
    // '{' rhs ?init_list ?',' '}'
    AST_BRACED_INITIALIZER,
    // lhs ?storage_class_specs rhs type_name
    AST_COMPOUND_LITERAL_TYPE,
    // TODO: combine this with STORAGE_CLASS_SPEC?
    // token_range storage_class_spec[token_idx...rhs]
    AST_STORAGE_CLASS_SPECS,
    // main token is identifier
    AST_IDENTIFIER,
} ASTNodeKind;

_Static_assert(AST_IDENTIFIER < 256, "ASTNodeKind does not fit into a byte");

typedef struct ASTNodeData {
    uint32_t main_token;
    // lhs is implicit, as it is always the next node
    uint32_t rhs;
    uint32_t type_data_idx;
} ASTNodeData;

typedef struct ASTTypeData {
    // TODO:
    uint32_t dummy;
} ASTTypeData;

typedef struct AST {
    uint32_t len, cap;
    uint8_t* kinds;
    ASTNodeData* datas;
    uint32_t type_data_len, type_data_cap;
    ASTTypeData* type_data;
    TokenArr toks;
} AST;

AST parse_ast(TokenArr* tokens, ParserErr* err);

void AST_free(AST* ast);

#endif
