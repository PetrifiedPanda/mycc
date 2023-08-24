#include "frontend/ast/ast.h"

#include "util/mem.h"

static uint32_t add_node(AST* ast, ASTNodeKind kind, uint32_t main_token, uint32_t rhs, bool alloc_type_data) {
    if (ast->len == ast->cap) {
        mycc_grow_alloc((void**)&ast->kinds, &ast->cap, sizeof *ast->kinds);
        ast->datas = mycc_realloc(ast->datas, sizeof *ast->datas * ast->cap);
    }
    
    uint32_t type_data_idx = (uint32_t)-1;
    if (alloc_type_data) {
        if (ast->type_data_len == ast->type_data_cap) {
            mycc_grow_alloc((void**)&ast->type_data, &ast->type_data_cap, sizeof *ast->type_data);
        }
        type_data_idx = ast->type_data_len;
        ++ast->type_data_len;
    }
    const uint32_t idx = ast->len;
    ast->kinds[ast->len] = kind;
    ast->datas[ast->len] = (ASTNodeData){
        .main_token = main_token,
        .rhs = rhs,
        .type_data_idx = type_data_idx,
    };
    

    ++ast->len;
    return idx;
}

static uint32_t add_node_without_children(AST* ast, ASTNodeKind kind, uint32_t main_token) {    
    return add_node(ast, kind, main_token, 0, true);
}

uint32_t parse_expr_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

uint32_t parse_assign_expr_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

uint32_t parse_generic_assoc_list_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

uint32_t parse_generic_sel_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    const uint32_t res = add_node(ast, AST_GENERIC_SEL, s->_it, 0, true);
    ParserState_accept_it(s);
    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return 0;
    }
    // lhs
    if (parse_assign_expr_2(s, ast) == 0) {
        return 0;
    }

    if (!ParserState_accept(s, TOKEN_COMMA)) {
        return 0;
    }

    const uint32_t rhs = parse_generic_assoc_list_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

uint32_t parse_primary_expr_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_PRIMARY_EXPR, s->_it, 0, false);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_enum_constant(s, ParserState_curr_spell(s))) {
                add_node_without_children(ast, AST_ENUM_CONSTANT, s->_it);
            } else {
                add_node_without_children(ast, AST_IDENTIFIER, s->_it);
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_I_CONSTANT:
        case TOKEN_F_CONSTANT:
            add_node_without_children(ast, AST_CONSTANT, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_STRING_LITERAL:
            add_node_without_children(ast, AST_STRING_LITERAL, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_FUNC_NAME:
            add_node_without_children(ast, AST_FUNC, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_LBRACKET:
            ParserState_accept_it(s);
            if (parse_expr_2(s, ast) == 0) {
                return 0;
            }
            if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                return 0;
            }
            break;
        case TOKEN_GENERIC:
            parse_generic_sel_2(s, ast);
            break;
        default:
            // TODO: error or unreachable
            return 0;
    }

    return res;
}
