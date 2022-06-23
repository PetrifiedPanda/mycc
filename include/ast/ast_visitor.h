#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include <stdbool.h>

struct identifier;
struct spec_qual_list;
struct string_literal;
struct translation_unit;
struct type_name;

struct abs_declarator;
struct align_spec;
struct atomic_type_spec;
struct declaration;
struct declaration_list;
struct declaration_specs;
struct declarator;
struct direct_abs_declarator;
struct direct_declarator;
struct enum_list;
struct enum_spec;
struct enumerator;
struct external_declaration;
struct func_def;
struct identifier_list;
struct init_declarator;
struct init_declarator_list;
struct param_declaration;
struct param_list;
struct param_type_list;
struct pointer;
struct static_assert_declaration;
struct struct_declaration;
struct struct_declaration_list;
struct struct_declarator;
struct struct_declarator_list;
struct struct_union_spec;
struct type_quals;
struct type_specs;

struct add_expr;
struct and_expr;
struct arg_expr_list;
struct assign_expr;
struct cast_expr;
struct cond_expr;
struct const_expr;
struct constant;
struct eq_expr;
struct expr;
struct generic_assoc;
struct generic_assoc_list;
struct generic_sel;
struct log_and_expr;
struct log_or_expr;
struct mul_expr;
struct or_expr;
struct postfix_expr;
struct primary_expr;
struct rel_expr;
struct shift_expr;
struct string_constant;
struct unary_expr;
struct xor_expr;

struct designation;
struct designator;
struct designator_list;
struct init_list;
struct initializer;

struct block_item;
struct compound_statement;
struct expr_statement;
struct iteration_statement;
struct jump_statement;
struct labelled_statement;
struct selection_statement;
struct statement;

#define AST_VISITOR_FUNC(type_name)                                            \
    bool (*visit_##type_name)(struct ast_visitor*, struct type_name*)

enum ast_visitor_order {
    AST_VISITOR_PREORDER,
    AST_VISITOR_POSTORDER,
};

struct ast_visitor {
    enum ast_visitor_order order;

    AST_VISITOR_FUNC(identifier);
    AST_VISITOR_FUNC(spec_qual_list);
    AST_VISITOR_FUNC(string_literal);
    AST_VISITOR_FUNC(translation_unit);
    AST_VISITOR_FUNC(type_name);

    AST_VISITOR_FUNC(abs_declarator);
    AST_VISITOR_FUNC(align_spec);
    AST_VISITOR_FUNC(atomic_type_spec);
    AST_VISITOR_FUNC(declaration);
    AST_VISITOR_FUNC(declaration_list);
    AST_VISITOR_FUNC(declaration_specs);
    AST_VISITOR_FUNC(declarator);
    AST_VISITOR_FUNC(direct_abs_declarator);
    AST_VISITOR_FUNC(direct_declarator);
    AST_VISITOR_FUNC(enum_list);
    AST_VISITOR_FUNC(enum_spec);
    AST_VISITOR_FUNC(enumerator);
    AST_VISITOR_FUNC(external_declaration);
    AST_VISITOR_FUNC(func_def);
    AST_VISITOR_FUNC(identifier_list);
    AST_VISITOR_FUNC(init_declarator);
    AST_VISITOR_FUNC(init_declarator_list);
    AST_VISITOR_FUNC(param_declaration);
    AST_VISITOR_FUNC(param_list);
    AST_VISITOR_FUNC(param_type_list);
    AST_VISITOR_FUNC(pointer);
    AST_VISITOR_FUNC(static_assert_declaration);
    AST_VISITOR_FUNC(struct_declaration);
    AST_VISITOR_FUNC(struct_declaration_list);
    AST_VISITOR_FUNC(struct_declarator);
    AST_VISITOR_FUNC(struct_declarator_list);
    AST_VISITOR_FUNC(struct_union_spec);
    AST_VISITOR_FUNC(type_quals);
    AST_VISITOR_FUNC(type_specs);

    AST_VISITOR_FUNC(add_expr);
    AST_VISITOR_FUNC(and_expr);
    AST_VISITOR_FUNC(arg_expr_list);
    AST_VISITOR_FUNC(assign_expr);
    AST_VISITOR_FUNC(cast_expr);
    AST_VISITOR_FUNC(cond_expr);
    AST_VISITOR_FUNC(const_expr);
    AST_VISITOR_FUNC(constant);
    AST_VISITOR_FUNC(eq_expr);
    AST_VISITOR_FUNC(expr);
    AST_VISITOR_FUNC(generic_assoc);
    AST_VISITOR_FUNC(generic_assoc_list);
    AST_VISITOR_FUNC(generic_sel);
    AST_VISITOR_FUNC(log_and_expr);
    AST_VISITOR_FUNC(log_or_expr);
    AST_VISITOR_FUNC(mul_expr);
    AST_VISITOR_FUNC(or_expr);
    AST_VISITOR_FUNC(postfix_expr);
    AST_VISITOR_FUNC(primary_expr);
    AST_VISITOR_FUNC(rel_expr);
    AST_VISITOR_FUNC(shift_expr);
    AST_VISITOR_FUNC(string_constant);
    AST_VISITOR_FUNC(unary_expr);
    AST_VISITOR_FUNC(xor_expr);

    AST_VISITOR_FUNC(designation);
    AST_VISITOR_FUNC(designator);
    AST_VISITOR_FUNC(designator_list);
    AST_VISITOR_FUNC(init_list);
    AST_VISITOR_FUNC(initializer);

    AST_VISITOR_FUNC(block_item);
    AST_VISITOR_FUNC(compound_statement);
    AST_VISITOR_FUNC(expr_statement);
    AST_VISITOR_FUNC(iteration_statement);
    AST_VISITOR_FUNC(jump_statement);
    AST_VISITOR_FUNC(labelled_statement);
    AST_VISITOR_FUNC(selection_statement);
    AST_VISITOR_FUNC(statement);
};

#define AST_VISITOR_VISIT_TEMPLATE(visitor,                                    \
                                   node,                                       \
                                   visit_children,                             \
                                   visitor_func)                               \
    switch (visitor->order) {                                                  \
        case AST_VISITOR_PREORDER: {                                           \
            if (!visitor_func(visitor, node)) {                                \
                return false;                                                  \
            }                                                                  \
            return visit_children(visitor, node);                              \
        }                                                                      \
        case AST_VISITOR_POSTORDER: {                                          \
            if (!visit_children(visitor, node)) {                              \
                return false;                                                  \
            }                                                                  \
            return visitor_func(visitor, node);                                \
        }                                                                      \
    }                                                                          \
    return false // unreachable

#include "ast/translation_unit.h"

#undef AST_VISITOR_FUNC

#endif

