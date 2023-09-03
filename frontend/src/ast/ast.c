#include "frontend/ast/ast.h"

#include <string.h>

#include "util/mem.h"
#include "util/timing.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void AST_ensure_capacity(AST* ast) {
    if (ast->len == ast->cap) {
        mycc_grow_alloc((void**)&ast->kinds, &ast->cap, sizeof *ast->kinds);
        ast->datas = mycc_realloc(ast->datas, sizeof *ast->datas * ast->cap);
    }
}

static uint32_t add_node(AST* ast, ASTNodeKind kind, uint32_t main_token) {
    AST_ensure_capacity(ast);

    const uint32_t idx = ast->len;
    ast->kinds[idx] = kind;
    ast->datas[idx] = (ASTNodeData){
        .main_token = main_token,
        .rhs = 0, // has to be initialized after (because we don't have rhs yet)
        .type_data_idx = (uint32_t)-1,
    };

    ++ast->len;
    return idx;
}

static uint32_t add_node_with_type(AST* ast,
                                   ASTNodeKind kind,
                                   uint32_t main_token) {
    AST_ensure_capacity(ast);

    const uint32_t type_data_idx = ast->type_data_len;
    ++ast->type_data_len;
    const uint32_t idx = ast->len;
    ast->kinds[idx] = kind;
    ast->datas[idx] = (ASTNodeData){
        .main_token = main_token,
        .rhs = 0,
        .type_data_idx = type_data_idx,
    };

    ++ast->len;
    return idx;
}

#define CHECK_ERR(expr)                                                        \
    do {                                                                       \
        if (!(expr)) {                                                         \
            return 0;                                                          \
        }                                                                      \
    } while (0)

static bool parse_translation_unit_2(ParserState* s, AST* ast);

static void AST_free_error(AST* ast) {
    mycc_free(ast->kinds);
    mycc_free(ast->datas);
    mycc_free(ast->type_data);

    ast->len = 0;
    ast->cap = 0;
    ast->kinds = NULL;
    ast->datas = NULL;
    ast->type_data = NULL;
}

AST parse_ast(TokenArr* tokens, ParserErr* err) {
    assert(tokens);
    assert(err);

    MYCC_TIMER_BEGIN();

    ParserState s = ParserState_create(tokens, err);

    // TODO: allocate appropriate size for AST
    AST res = {
        .len = 0,
        .cap = 0,
        .kinds = NULL,
        .datas = NULL,
        .type_data_len = 0,
        .type_data_cap = 0,
        .type_data = NULL,
    };

    if (!parse_translation_unit_2(&s, &res)) {
        ParserState_free(&s);
        AST_free_error(&res);
        assert(err->kind != PARSER_ERR_NONE);
        res.toks = s._arr;
        res.len = 0;
        res.cap = 0;
        return res;
    }
    res.toks = s._arr;

    ParserState_free(&s);
    MYCC_TIMER_END("parser");
    return res;
}

void AST_free(AST* ast) {
    mycc_free(ast->kinds);
    mycc_free(ast->datas);
    mycc_free(ast->type_data);
    TokenArr_free(&ast->toks);
}

static uint32_t parse_external_declaration_2(ParserState* s, AST* ast);

static bool parse_translation_unit_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    assert(res == 0);

    while (ParserState_curr_kind(s) != TOKEN_INVALID) {
        if (!parse_external_declaration_2(s, ast)) {
            return false;
        }
    }
    ast->datas[res].rhs = ast->len;
    return true;
}

static uint32_t parse_statement_2(ParserState* s, AST* ast);

static uint32_t parse_labeled_statement_label(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    assert(ParserState_next_token_kind(s) == TOKEN_COLON);
    const uint32_t res = add_node(ast, AST_LABELED_STATEMENT_LABEL, s->it);
    add_node_with_type(ast, AST_IDENTIFIER, s->it);
    ParserState_accept_it(s);
    ParserState_accept_it(s);
    const uint32_t rhs = parse_statement_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast);

static uint32_t parse_labeled_statement_case(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_CASE
           || ParserState_curr_kind(s) == TOKEN_DEFAULT);
    const uint32_t res = add_node(ast, AST_LABELED_STATEMENT_CASE, s->it);

    const TokenKind begin_kind = ParserState_curr_kind(s);
    ParserState_accept_it(s);
    switch (begin_kind) {
        case TOKEN_CASE:
            CHECK_ERR(parse_const_expr_2(s, ast));
            break;
        case TOKEN_DEFAULT:
            break;
        default:
            UNREACHABLE();
    }
    CHECK_ERR(ParserState_accept(s, TOKEN_COLON));
    const uint32_t rhs = parse_statement_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_expr_statement_after_attr_2(ParserState* s,
                                                  AST* ast,
                                                  uint32_t res);
static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast);
static uint32_t parse_compound_statement_2(ParserState* s, AST* ast);
static uint32_t parse_jump_statement_2(ParserState* s, AST* ast);
static uint32_t parse_sel_statement_2(ParserState* s, AST* ast);
static uint32_t parse_iteration_statement_2(ParserState* s, AST* ast);

// TODO: If there is no attribute here, we don't need a AST_UNLABELED_STATEMENT
// node
static uint32_t parse_statement_2(ParserState* s, AST* ast) {
    // labeled_statement or unlabeled_statement
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);

    // TODO: might have to check next token as well
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_next_token_kind(s) == TOKEN_COLON) {
                ast->kinds[res] = AST_LABELED_STATEMENT;
                const uint32_t rhs = parse_labeled_statement_label(s, ast);
                CHECK_ERR(rhs);
                ast->datas[res].rhs = rhs;
            } else {
                ast->kinds[res] = AST_UNLABELED_STATEMENT;
                CHECK_ERR(parse_expr_statement_after_attr_2(s, ast, res));
            }
            break;
        case TOKEN_CASE:
        case TOKEN_DEFAULT: {
            ast->kinds[res] = AST_LABELED_STATEMENT;
            const uint32_t rhs = parse_labeled_statement_case(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_LBRACE: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_compound_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_SEMICOLON:
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            ParserState_accept_it(s);
            break;
        case TOKEN_FOR:
        case TOKEN_WHILE:
        case TOKEN_DO: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_iteration_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_GOTO:
        case TOKEN_CONTINUE:
        case TOKEN_BREAK:
        case TOKEN_RETURN: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_jump_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_IF:
        case TOKEN_SWITCH: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_sel_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        default:
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            CHECK_ERR(parse_expr_statement_after_attr_2(s, ast, res));
            break;
    }

    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    return res;
}

static uint32_t parse_expr_2(ParserState* s, AST* ast);

static uint32_t parse_for_loop_actions_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_FOR_LOOP_ACTIONS, s->it);
    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
    } else {
        CHECK_ERR(parse_expr_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    }

    if (ParserState_curr_kind(s) != TOKEN_RBRACKET) {
        const uint32_t rhs = parse_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_declaration_2(ParserState* s, AST* ast);

static uint32_t parse_for_clause_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_FOR_CLAUSE, s->it);
    if (is_declaration(s)) {
        CHECK_ERR(parse_declaration_2(s, ast));
    } else if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
    } else {
        CHECK_ERR(parse_expr_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    }

    const uint32_t rhs = parse_for_loop_actions_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

// TODO: parsing for simple_if, switch and while is identical
static uint32_t parse_iteration_statement_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_WHILE
           || ParserState_curr_kind(s) == TOKEN_DO
           || ParserState_curr_kind(s) == TOKEN_FOR);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_WHILE: {
            const uint32_t res = add_node(ast, AST_WHILE, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            CHECK_ERR(parse_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            const uint32_t rhs = parse_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            return res;
        }
        case TOKEN_DO: {
            const uint32_t res = add_node(ast, AST_DO_WHILE, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(parse_statement_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_WHILE));
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            const uint32_t rhs = parse_expr_2(s, ast);
            CHECK_ERR(rhs);
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
            return res;
        }
        case TOKEN_FOR: {
            const uint32_t res = add_node(ast, AST_FOR, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            CHECK_ERR(parse_for_clause_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            const uint32_t rhs = parse_statement_2(s, ast);
            CHECK_ERR(rhs);
            return res;
        }
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_simple_if_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IF);
    const uint32_t res = add_node(ast, AST_SIMPLE_IF, s->it);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    CHECK_ERR(parse_expr_2(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    const uint32_t rhs = parse_statement_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_sel_statement_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IF
           || ParserState_curr_kind(s) == TOKEN_SWITCH);

    switch (ParserState_curr_kind(s)) {
        case TOKEN_IF: {
            const uint32_t res = add_node(ast, AST_IF_ELSE, s->it);
            CHECK_ERR(parse_simple_if_2(s, ast));
            if (ParserState_curr_kind(s) == TOKEN_ELSE) {
                ParserState_accept_it(s);
                const uint32_t rhs = parse_statement_2(s, ast);
                CHECK_ERR(rhs);
                ast->datas[res].rhs = rhs;
            }
            return res;
        }
        case TOKEN_SWITCH: {
            const uint32_t res = add_node(ast, AST_SWITCH_STATEMENT, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            CHECK_ERR(parse_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            const uint32_t rhs = parse_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            return res;
        }
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_jump_statement_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GOTO
           || ParserState_curr_kind(s) == TOKEN_CONTINUE
           || ParserState_curr_kind(s) == TOKEN_BREAK
           || ParserState_curr_kind(s) == TOKEN_RETURN);

    const uint32_t start_idx = s->it;
    switch (ParserState_curr_kind(s)) {
        case TOKEN_GOTO: {
            ParserState_accept_it(s);
            const uint32_t id_idx = s->it;
            CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
            CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
            const uint32_t res = add_node(ast, AST_GOTO_STATEMENT, start_idx);
            add_node_with_type(ast, AST_IDENTIFIER, id_idx);
            return res;
        }
        case TOKEN_CONTINUE: {
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
            return add_node(ast, AST_CONTINUE_STATEMENT, start_idx);
        }
        case TOKEN_BREAK: {
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
            return add_node(ast, AST_BREAK_STATEMENT, start_idx);
        }
        case TOKEN_RETURN: {
            ParserState_accept_it(s);
            const uint32_t res = add_node(ast, AST_RETURN_STATEMENT, start_idx);
            if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
                ParserState_accept_it(s);
                return res;
            } else {
                CHECK_ERR(parse_expr_2(s, ast));
                CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
            }
            return res;
        }
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_label_after_attr_2(ParserState* s,
                                         AST* ast,
                                         uint32_t res) {
    assert(ast->kinds[res] == AST_LABEL);
    assert(ParserState_curr_kind(s) == TOKEN_CASE
           || ParserState_curr_kind(s) == TOKEN_DEFAULT
           || ParserState_curr_kind(s) == TOKEN_IDENTIFIER);

    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            assert(ParserState_next_token_kind(s) == TOKEN_COLON);
            ast->datas[res].rhs = add_node_with_type(ast,
                                                     AST_IDENTIFIER,
                                                     s->it);
            ParserState_accept_it(s);
            ParserState_accept_it(s);
            break;
        case TOKEN_CASE: {
            ParserState_accept_it(s);
            const uint32_t rhs = parse_const_expr_2(s, ast);
            CHECK_ERR(rhs);
            CHECK_ERR(ParserState_accept(s, TOKEN_COLON));
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_DEFAULT:
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_COLON));
            break;
        default:
            UNREACHABLE();
    }
    return res;
}

static uint32_t parse_expr_statement_after_attr_2(ParserState* s,
                                                  AST* ast,
                                                  uint32_t res) {
    assert(ast->kinds[res] == AST_UNLABELED_STATEMENT);
    const uint32_t rhs = parse_expr_2(s, ast);
    CHECK_ERR(rhs);
    CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_static_assert_declaration_2(ParserState* s, AST* ast);
static uint32_t parse_declaration_after_attr_2(ParserState* s,
                                               AST* ast,
                                               uint32_t res);

// TODO: If there is no attribute here, we don't need a AST_UNLABELED_STATEMENT
// node
static uint32_t parse_block_item(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }
    // declaration, attribute_declaration, unlabeled_statement or label
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    // TODO: might have to check next token as well
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_typedef(s, ParserState_curr_spell(s))) {
                ast->kinds[res] = AST_DECLARATION;
                CHECK_ERR(parse_declaration_after_attr_2(s, ast, res));
            } else if (ParserState_next_token_kind(s) == TOKEN_COLON) {
                ast->kinds[res] = AST_LABEL;
                CHECK_ERR(parse_label_after_attr_2(s, ast, res));
            } else {
                ast->kinds[res] = AST_UNLABELED_STATEMENT;
                CHECK_ERR(parse_expr_statement_after_attr_2(s, ast, res));
            }
            break;
        case TOKEN_CASE:
        case TOKEN_DEFAULT:
            ast->kinds[res] = AST_LABEL;
            CHECK_ERR(parse_label_after_attr_2(s, ast, res));
            break;
        case TOKEN_LBRACE: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_compound_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_SEMICOLON:
            // already parsed attribute
            if (ast->len != res + 1) {
                ast->kinds[res] = AST_ATTRIBUTE_DECLARATION;
            } else {
                ast->kinds[res] = AST_UNLABELED_STATEMENT;
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_FOR:
        case TOKEN_WHILE:
        case TOKEN_DO: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_iteration_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_GOTO:
        case TOKEN_CONTINUE:
        case TOKEN_BREAK:
        case TOKEN_RETURN: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_jump_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        case TOKEN_IF:
        case TOKEN_SWITCH: {
            ast->kinds[res] = AST_UNLABELED_STATEMENT;
            const uint32_t rhs = parse_sel_statement_2(s, ast);
            CHECK_ERR(rhs);
            ast->datas[res].rhs = rhs;
            break;
        }
        default:
            if (is_declaration_spec(s)) {
                ast->kinds[res] = AST_DECLARATION;
                CHECK_ERR(parse_declaration_after_attr_2(s, ast, res));
            } else {
                ast->kinds[res] = AST_UNLABELED_STATEMENT;
                CHECK_ERR(parse_expr_statement_after_attr_2(s, ast, res));
            }
            break;
    }

    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    return res;
}

// TODO: test with this ending before closing brace
static uint32_t parse_compound_statement_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACE);
    ParserState_push_scope(s);
    const uint32_t res = add_node(ast, AST_COMPOUND_STATEMENT, s->it);
    ParserState_accept_it(s);
    while (ParserState_curr_kind(s) != TOKEN_RBRACE
           && ParserState_curr_kind(s) != TOKEN_INVALID) {
        CHECK_ERR(parse_block_item(s, ast));
    }
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
    ParserState_pop_scope(s);
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_declarator_2(ParserState* s, AST* ast, bool is_typedef);
static uint32_t parse_initializer_2(ParserState* s, AST* ast);

static uint32_t parse_init_declarator_2(ParserState* s,
                                        AST* ast,
                                        bool is_typedef) {
    // TODO: not sure if this needs a type
    const uint32_t res = add_node_with_type(ast, AST_INIT_DECLARATOR, s->it);
    CHECK_ERR(parse_declarator_2(s, ast, is_typedef));
    if (ParserState_curr_kind(s) == TOKEN_ASSIGN) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_initializer_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_init_declarator_list_first_2(ParserState* s,
                                                   AST* ast,
                                                   uint32_t res,
                                                   bool is_typedef) {
    assert(ast->kinds[res] == AST_INIT_DECLARATOR_LIST);

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_init_declarator_2(s, ast, is_typedef));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_init_declarator_list_2(ParserState* s,
                                             AST* ast,
                                             bool is_typedef) {
    const uint32_t res = add_node(ast, AST_INIT_DECLARATOR_LIST, s->it);
    CHECK_ERR(parse_init_declarator_2(s, ast, is_typedef));
    return parse_init_declarator_list_first_2(s, ast, res, is_typedef);
}

static uint32_t parse_declaration_specs_2(ParserState* s,
                                          AST* ast,
                                          bool* is_typedef);

static uint32_t parse_declaration_specs_and_init_declarator_list(ParserState* s,
                                                                 AST* ast) {
    const uint32_t res = add_node(
        ast,
        AST_DECLARATION_SPECS_AND_INIT_DECLARATOR_LIST,
        s->it);
    bool is_typedef = false;
    CHECK_ERR(parse_declaration_specs_2(s, ast, &is_typedef));
    if (ParserState_curr_kind(s) != TOKEN_SEMICOLON) {
        const uint32_t rhs = parse_init_declarator_list_2(s, ast, is_typedef);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
        ast->datas[res].rhs = rhs;
    } else {
        ParserState_accept_it(s);
    }
    return res;
}

static uint32_t parse_declaration_after_attr_2(ParserState* s,
                                               AST* ast,
                                               uint32_t res) {
    assert(ast->kinds[res] == AST_DECLARATION);
    const uint32_t rhs = parse_declaration_specs_and_init_declarator_list(s,
                                                                          ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_declaration_2(ParserState* s, AST* ast) {
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }

    // attribute_declaration or declaration
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);

    // TODO: might need to check next token as well
    if (curr_kind == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
        if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
            ParserState_accept_it(s);
            ast->kinds[res] = AST_ATTRIBUTE_DECLARATION;
            return res;
        }
    }
    ast->kinds[res] = AST_DECLARATION;
    CHECK_ERR(parse_declaration_after_attr_2(s, ast, res));
    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    return res;
}

// TODO: test this with EOF instead of LBRACE
// for K&R function declarations
static uint32_t parse_declaration_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DECLARATION_LIST, s->it);
    CHECK_ERR(parse_declaration_2(s, ast));
    // TODO: not sure if this condition is correct
    while (ParserState_curr_kind(s) != TOKEN_LBRACE
           && ParserState_curr_kind(s) != TOKEN_INVALID) {
        CHECK_ERR(parse_declaration_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_external_declaration_2(ParserState* s, AST* ast) {
    const TokenKind start_kind = ParserState_curr_kind(s);
    if (start_kind == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }
    // func_def, declaration, or attribute_declaration
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    // TODO: might need to check next token as well
    if (start_kind == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
        if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
            ParserState_accept_it(s);
            ast->kinds[res] = AST_ATTRIBUTE_DECLARATION;
            return res;
        }
    }

    // TODO: change comments when I have a better name for func_def_impl and
    // func_def_sub_impl func_def_impl or
    // declaration_specs_and_init_declarator_list
    const uint32_t rhs = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    ast->datas[res].rhs = rhs;
    bool is_typedef = false;
    // lhs of rhs
    CHECK_ERR(parse_declaration_specs_2(s, ast, &is_typedef));
    // declaration without declarators
    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
        ast->kinds[res] = AST_DECLARATION;
        ast->kinds[rhs] = AST_DECLARATION_SPECS_AND_INIT_DECLARATOR_LIST;
        // TODO: error if this has attributes
        // TODO: error typedef without declarator
        return res;
    }
    // func_def_sub_impl or init_declarator_list
    const uint32_t rhs_rhs = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    ast->datas[rhs].rhs = rhs_rhs;
    // declarator_and_decl_list or init_declarator
    const uint32_t rhs_rhs_lhs = add_node(ast, AST_TRANSLATION_UNIT, s->it);
    CHECK_ERR(parse_declarator_2(s, ast, is_typedef));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_ASSIGN || curr_kind == TOKEN_COMMA
        || curr_kind == TOKEN_SEMICOLON) {
        ast->kinds[res] = AST_DECLARATION;
        ast->kinds[rhs] = AST_DECLARATION_SPECS_AND_INIT_DECLARATOR_LIST;
        ast->kinds[rhs_rhs] = AST_INIT_DECLARATOR_LIST;
        ast->kinds[rhs_rhs_lhs] = AST_INIT_DECLARATOR;
        if (ParserState_curr_kind(s) == TOKEN_ASSIGN) {
            ParserState_accept_it(s);
            const uint32_t rhs_rhs_lhs_rhs = parse_initializer_2(s, ast);
            CHECK_ERR(rhs_rhs_lhs_rhs);
            ast->datas[rhs_rhs_lhs].rhs = rhs_rhs_lhs_rhs;
        }
        CHECK_ERR(
            parse_init_declarator_list_first_2(s, ast, rhs_rhs, is_typedef));
        CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    } else {
        ast->kinds[res] = AST_FUNC_DEF;
        ast->kinds[rhs] = AST_FUNC_DEF_IMPL;
        ast->kinds[rhs_rhs] = AST_FUNC_DEF_SUB_IMPL;
        ast->kinds[rhs_rhs_lhs] = AST_FUNC_DECLARATOR_AND_DECL_LIST;
        // TODO: error typedef in function
        if (ParserState_curr_kind(s) != TOKEN_LBRACE) {
            const uint32_t rhs_rhs_lhs_rhs = parse_declaration_list_2(s, ast);
            CHECK_ERR(rhs_rhs_lhs_rhs);
            ast->datas[rhs_rhs_lhs].rhs = rhs_rhs_lhs_rhs;
        }
        const uint32_t rhs_rhs_rhs = parse_compound_statement_2(s, ast);
        CHECK_ERR(rhs_rhs_rhs);
        ast->datas[rhs_rhs].rhs = rhs_rhs_rhs;
    }

    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    assert(ast->kinds[rhs] != AST_TRANSLATION_UNIT);
    assert(ast->kinds[rhs_rhs] != AST_TRANSLATION_UNIT);
    assert(ast->kinds[rhs_rhs_lhs] != AST_TRANSLATION_UNIT);
    return res;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast);

static uint32_t parse_atomic_type_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ATOMIC);
    const uint32_t res = add_node(ast, AST_ATOMIC_TYPE_SPEC, s->it);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    CHECK_ERR(parse_type_name_2(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    return res;
}

static uint32_t parse_static_assert_declaration_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT);
    const uint32_t res = add_node(ast, AST_STATIC_ASSERT_DECLARATION, s->it);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));

    CHECK_ERR(parse_const_expr_2(s, ast));

    // TODO: message not optional in earlier c versions
    if (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        const uint32_t lit_idx = s->it;
        CHECK_ERR(ParserState_accept(s, TOKEN_STRING_LITERAL));
        // TODO: type might not be necessary in this node
        ast->datas[res].rhs = add_node_with_type(ast,
                                                 AST_STRING_LITERAL,
                                                 lit_idx);
    }

    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET)
              && ParserState_accept(s, TOKEN_SEMICOLON));
    return res;
}

static uint32_t parse_id_attribute_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    // TODO: not sure if this needs type data
    const uint32_t res = add_node_with_type(ast, AST_ID_ATTRIBUTE, s->it);
    add_node_with_type(ast, AST_IDENTIFIER, s->it);
    ParserState_accept_it(s);

    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_type_qual_list_2(ParserState* s, AST* ast);
static uint32_t parse_assign_expr_2(ParserState* s, AST* ast);

static uint32_t parse_arr_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_STATIC) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX_STATIC, idx);
        if (is_type_qual(ParserState_curr_kind(s))) {
            CHECK_ERR(parse_type_qual_list_2(s, ast));
        }

        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);

        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    } else if (curr_kind == TOKEN_ASTERISK) {
        ParserState_accept_it(s);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        return add_node(ast, AST_ARR_SUFFIX_ASTERISK, idx);
    } else if (is_type_qual(curr_kind)) {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx);
        CHECK_ERR(parse_type_qual_list_2(s, ast));

        switch (ParserState_curr_kind(s)) {
            case TOKEN_ASTERISK: {
                ast->kinds[res] = AST_ARR_SUFFIX_ASTERISK;
                ParserState_accept_it(s);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                return res;
            }
            case TOKEN_RINDEX: {
                ParserState_accept_it(s);
                return res;
            }
            case TOKEN_STATIC:
                ast->kinds[res] = AST_ARR_SUFFIX_STATIC;
                ParserState_accept_it(s);
                FALLTHROUGH();
            default: {
                const uint32_t rhs = parse_assign_expr_2(s, ast);
                CHECK_ERR(rhs);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                ast->datas[res].rhs = rhs;
                return res;
            }
        }

    } else if (curr_kind == TOKEN_RINDEX) {
        ParserState_accept_it(s);
        return add_node(ast, AST_ARR_SUFFIX, idx);
    } else {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx);
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    }
}

static uint32_t parse_identifier_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    const uint32_t res = add_node(ast, AST_IDENTIFIER_LIST, s->it);

    // TODO: does this need a type?
    add_node_with_type(ast, AST_IDENTIFIER, s->it);
    ParserState_accept_it(s);

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        const uint32_t curr = s->it;
        CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
        // TODO: does this need a type?
        add_node_with_type(ast, AST_IDENTIFIER, curr);
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_storage_class_spec_2(ParserState* s,
                                           AST* ast,
                                           bool* is_typedef) {
    // TODO: special handling for typedef
    assert(is_storage_class_spec(ParserState_curr_kind(s)));
    if (ParserState_curr_kind(s) == TOKEN_TYPEDEF) {
        *is_typedef = true;
    }
    const uint32_t res = add_node(ast, AST_STORAGE_CLASS_SPEC, s->it);
    ParserState_accept_it(s);
    return res;
}

static uint32_t parse_func_spec_2(ParserState* s, AST* ast) {
    assert(is_func_spec(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_FUNC_SPEC, s->it);
    ParserState_accept_it(s);
    return res;
}

static uint32_t parse_type_qual_2(ParserState* s, AST* ast) {
    assert(is_type_qual(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_TYPE_QUAL, s->it);
    ParserState_accept_it(s);
    return res;
}

static bool curr_is_type_qual(const ParserState* s) {
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_ATOMIC) {
        return ParserState_next_token_kind(s) != TOKEN_LBRACKET;
    } else {
        return is_type_qual(curr_kind);
    }
}

static uint32_t parse_type_spec_2(ParserState* s, AST* ast);
static uint32_t parse_align_spec_2(ParserState* s, AST* ast);

// TODO: might have problems with atomic
static uint32_t parse_declaration_spec_2(ParserState* s,
                                         AST* ast,
                                         bool* is_typedef) {
    // TODO: maybe use parse_type_spec_qual_2() here
    assert(is_declaration_spec(s));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (is_storage_class_spec(curr_kind)) {
        return parse_storage_class_spec_2(s, ast, is_typedef);
    } else if (curr_is_type_qual(s)) {
        return parse_type_qual_2(s, ast);
    } else if (is_type_spec(s)) {
        return parse_type_spec_2(s, ast);
    } else if (curr_kind == TOKEN_ALIGNAS) {
        return parse_align_spec_2(s, ast);
    } else if (is_func_spec(curr_kind)) {
        return parse_func_spec_2(s, ast);
    } else {
        UNREACHABLE();
    }
}

static uint32_t parse_declaration_specs_2(ParserState* s,
                                          AST* ast,
                                          bool* is_typedef) {
    if (!is_declaration_spec(s)) {
        // TODO: error
        return 0;
    }
    const uint32_t res = add_node(ast, AST_DECLARATION_SPECS, s->it);
    CHECK_ERR(parse_declaration_spec_2(s, ast, is_typedef));

    while (is_declaration_spec(s)) {
        CHECK_ERR(parse_declaration_spec_2(s, ast, is_typedef));
    }
    ast->datas[res].rhs = ast->len;
    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        assert(rhs == ast->datas[res].rhs);
    }
    return res;
}

static uint32_t parse_attrs_and_declaration_specs_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t res = add_node(ast,
                                      AST_ATTRS_AND_DECLARATION_SPECS,
                                      s->it);
        // TODO: cond may not be sufficient
        if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
            CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
        }

        bool is_typedef = false;
        const uint32_t rhs = parse_declaration_specs_2(s, ast, &is_typedef);
        CHECK_ERR(rhs);
        if (is_typedef) {
            // TODO: err
            return 0;
        }
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        bool is_typedef = false;
        const uint32_t res = parse_declaration_specs_2(s, ast, &is_typedef);
        if (is_typedef) {
            // TODO: err
            return 0;
        }
        return res;
    }
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast);
static uint32_t parse_direct_declarator_2(ParserState* s,
                                          AST* ast,
                                          bool is_typedef);
static uint32_t parse_direct_abs_declarator_2(ParserState* s, AST* ast);
static uint32_t parse_abs_arr_or_func_suffix_list_2(ParserState* s, AST* ast);
static uint32_t parse_arr_or_func_suffix_list_2(ParserState* s, AST* ast);

static uint32_t parse_abs_decl_or_decl_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it);

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    uint32_t rhs;
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            ast->kinds[res] = AST_DECLARATOR;
            rhs = parse_direct_declarator_2(s, ast, false);
            CHECK_ERR(rhs);
            break;
        case TOKEN_LINDEX:
            ast->kinds[res] = AST_ABS_DECLARATOR;
            rhs = parse_direct_abs_declarator_2(s, ast);
            CHECK_ERR(rhs);
            break;
        case TOKEN_LBRACKET: {
            // direct_abs_declarator or direct_declarator
            rhs = add_node(ast, AST_TRANSLATION_UNIT, s->it);
            ParserState_accept_it(s);
            // lhs of rhs
            const uint32_t bracket_decl = parse_abs_decl_or_decl_2(s, ast);
            CHECK_ERR(bracket_decl);
            uint32_t internal_rhs;
            if (ast->kinds[bracket_decl] == AST_ABS_DECLARATOR) {
                ast->kinds[res] = AST_ABS_DECLARATOR;
                ast->kinds[rhs] = AST_DIRECT_ABS_DECLARATOR;
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                internal_rhs = parse_abs_arr_or_func_suffix_list_2(s, ast);
            } else {
                ast->kinds[res] = AST_DECLARATOR;
                ast->kinds[rhs] = AST_DIRECT_DECLARATOR;
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                internal_rhs = parse_arr_or_func_suffix_list_2(s, ast);
            }
            CHECK_ERR(internal_rhs);
            ast->datas[rhs].rhs = internal_rhs;
            assert(ast->kinds[rhs] != AST_TRANSLATION_UNIT);
            break;
        }
        default:
            ast->kinds[res] = AST_ABS_DECLARATOR;
            // no pointer
            if (ast->len == res + 1) {
                ParserErr_set(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it);
                return 0;
            }
            rhs = 0;
            break;
    }

    ast->datas[res].rhs = rhs;
    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    return res;
}

static uint32_t parse_param_declaration_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_PARAM_DECLARATION, s->it);
    CHECK_ERR(parse_attrs_and_declaration_specs_2(s, ast));
    const TokenKind curr = ParserState_curr_kind(s);
    // no declarator
    if (curr == TOKEN_COMMA || curr == TOKEN_RBRACKET) {
        return res;
    }
    const uint32_t rhs = parse_abs_decl_or_decl_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_param_type_list_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_ELLIPSIS) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast, AST_PARAM_TYPE_LIST_VARIADIC, s->it);
        ast->datas[res].rhs = ast->len;
        return res;
    }

    const uint32_t res = add_node(ast, AST_PARAM_TYPE_LIST, s->it);
    CHECK_ERR(parse_param_declaration_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        if (ParserState_curr_kind(s) == TOKEN_ELLIPSIS) {
            ParserState_accept_it(s);
            ast->kinds[res] = AST_PARAM_TYPE_LIST_VARIADIC;
            break;
        }

        CHECK_ERR(parse_param_declaration_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_RBRACKET) {
        ParserState_accept_it(s);
        return add_node(ast, AST_FUNC_SUFFIX, idx);
    } else if (curr_kind == TOKEN_IDENTIFIER
               && !ParserState_is_typedef(s, ParserState_curr_spell(s))) {
        const uint32_t res = add_node(ast, AST_FUNC_SUFFIX_OLD, idx);
        CHECK_ERR(parse_identifier_list_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
        return res;
    } else {
        const uint32_t res = add_node(ast, AST_FUNC_SUFFIX, idx);
        CHECK_ERR(parse_param_type_list_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
        return res;
    }
}

static uint32_t parse_arr_or_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ARR_OR_FUNC_SUFFIX, s->it);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            CHECK_ERR(parse_arr_suffix_2(s, ast));
            break;
        case TOKEN_LBRACKET:
            CHECK_ERR(parse_func_suffix_2(s, ast));
            break;
        default:
            UNREACHABLE();
    }

    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_arr_or_func_suffix_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ARR_OR_FUNC_SUFFIX_LIST, s->it);

    CHECK_ERR(parse_arr_or_func_suffix_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_arr_or_func_suffix_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_direct_declarator_2(ParserState* s,
                                          AST* ast,
                                          bool is_typedef) {
    const uint32_t res = add_node(ast, AST_DIRECT_DECLARATOR, s->it);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_IDENTIFIER) {
        if (is_typedef) {
            CHECK_ERR(
                ParserState_register_typedef(s,
                                             ParserState_curr_spell_buf(s),
                                             s->it));
        }
        CHECK_ERR(parse_id_attribute_2(s, ast));
    } else if (curr_kind == TOKEN_LBRACKET) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_declarator_2(s, ast, is_typedef));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    } else {
        static const TokenKind ex[] = {
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
        };
        expected_tokens_error(s, ex, ARR_LEN(ex));
        return 0;
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_arr_or_func_suffix_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_declarator_2(ParserState* s, AST* ast, bool is_typedef) {
    // TODO: unsure about whether this needs a type
    const uint32_t res = add_node_with_type(ast, AST_DECLARATOR, s->it);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    const uint32_t rhs = parse_direct_declarator_2(s, ast, is_typedef);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATOR, s->it);

    if (ParserState_curr_kind(s) != TOKEN_COLON) {
        CHECK_ERR(parse_declarator_2(s, ast, false));
    }

    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_const_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_member_declarator_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATOR_LIST, s->it);
    CHECK_ERR(parse_member_declarator_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_member_declarator_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast);

static uint32_t parse_member_declaration_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATION_BODY, s->it);
    CHECK_ERR(parse_spec_qual_list_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
        return res;
    }

    const uint32_t rhs = parse_member_declarator_list_2(s, ast);
    CHECK_ERR(rhs);
    CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declaration_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }

    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATION, s->it);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    const uint32_t rhs = parse_member_declaration_body_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declaration_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATION_LIST, s->it);
    CHECK_ERR(parse_member_declaration_2(s, ast));

    // TODO: condition may be wrong
    while (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        CHECK_ERR(parse_member_declaration_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_struct_union_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_STRUCT_UNION_BODY, s->it);
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        add_node_with_type(ast, AST_IDENTIFIER, s->it);
        ParserState_accept_it(s);
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_member_declaration_list_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
    }

    // neither lhs or rhs exists
    if (ast->len == res + 1) {
        // TODO: error
        return 0;
    }
    return res;
}

static uint32_t parse_struct_union_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_STRUCT
           || ParserState_curr_kind(s) == TOKEN_UNION);
    const ASTNodeKind kind = ParserState_curr_kind(s) == TOKEN_STRUCT
                                 ? AST_STRUCT_SPEC
                                 : AST_UNION_SPEC;
    const uint32_t res = add_node(ast, kind, s->it);
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    const uint32_t rhs = parse_struct_union_body_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_attribute_id_2(ParserState* s, AST* ast, bool* has_id) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX
           || ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_ID, s->it);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        // TODO: not sure if this needs a type
        const uint32_t rhs = add_node_with_type(ast, AST_IDENTIFIER, s->it);
        ParserState_accept_it(s);
        *has_id = true;
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enum_constant_and_attribute_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ENUM_CONSTANT_AND_ATTRIBUTE, s->it);
    const uint32_t const_idx = s->it;
    const StrBuf* spell = ParserState_curr_spell_buf(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
    add_node_with_type(ast, AST_ENUM_CONSTANT, const_idx);
    CHECK_ERR(ParserState_register_enum_constant(s, spell, const_idx));
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enumerator_2(ParserState* s, AST* ast) {
    // TODO: not sure if this needs a type
    const uint32_t res = add_node_with_type(ast, AST_ENUMERATOR, s->it);
    CHECK_ERR(parse_enum_constant_and_attribute_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_ASSIGN) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_const_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enum_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ENUM_LIST, s->it);
    CHECK_ERR(parse_enumerator_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_enumerator_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_enum_body_2(ParserState* s, AST* ast, bool has_id) {
    assert(ParserState_curr_kind(s) == TOKEN_COLON
           || ParserState_curr_kind(s) == TOKEN_LBRACE);
    const uint32_t res = add_node(ast, AST_ENUM_BODY, s->it);
    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_spec_qual_list_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_enum_list_2(s, ast);
        CHECK_ERR(rhs);
        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
        ast->datas[res].rhs = rhs;
    } else if (!has_id) {
        expected_token_error(s, TOKEN_LBRACE);
        return 0;
    }

    return res;
}

static uint32_t parse_enum_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ENUM);
    const uint32_t res = add_node(ast, AST_ENUM_SPEC, s->it);
    ParserState_accept_it(s);
    bool has_id = false;
    const TokenKind first_kind = ParserState_curr_kind(s);
    if (first_kind == TOKEN_LINDEX || first_kind == TOKEN_IDENTIFIER) {
        CHECK_ERR(parse_attribute_id_2(s, ast, &has_id));
    }

    const TokenKind next_kind = ParserState_curr_kind(s);
    if (next_kind == TOKEN_COLON || next_kind == TOKEN_LBRACE) {
        const uint32_t rhs = parse_enum_body_2(s, ast, has_id);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_type_spec_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    const uint32_t start_idx = s->it;
    switch (curr_kind) {
        case TOKEN_VOID:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_VOID, start_idx);
        case TOKEN_CHAR:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_CHAR, start_idx);
        case TOKEN_SHORT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SHORT, start_idx);
        case TOKEN_INT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_INT, start_idx);
        case TOKEN_LONG:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_LONG, start_idx);
        case TOKEN_FLOAT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_FLOAT, start_idx);
        case TOKEN_DOUBLE:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_DOUBLE, start_idx);
        case TOKEN_SIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SIGNED, start_idx);
        case TOKEN_UNSIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_UNSIGNED, start_idx);
        case TOKEN_BOOL:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_BOOL, start_idx);
        case TOKEN_COMPLEX:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_COMPLEX, start_idx);
        case TOKEN_IMAGINARY:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_IMAGINARY, start_idx);
        // TODO: _BitInt, _Decimal(32, 64, 128)
        case TOKEN_ATOMIC:
            return parse_atomic_type_spec_2(s, ast);
        case TOKEN_STRUCT:
        case TOKEN_UNION:
            return parse_struct_union_spec_2(s, ast);
        case TOKEN_ENUM:
            return parse_enum_spec_2(s, ast);
        case TOKEN_IDENTIFIER:
            // TODO: this error might not be necessary
            if (!ParserState_is_typedef(s, ParserState_curr_spell(s))) {
                ParserErr_set(s->err,
                              PARSER_ERR_EXPECTED_TYPEDEF_NAME,
                              start_idx);
                return 0;
            }
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_TYPEDEF_NAME, start_idx);
        // TODO: typeof, typeof_unqual
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_align_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    const uint32_t res = add_node(ast, AST_ALIGN_SPEC, s->it);
    ParserState_accept_it(s);
    const bool is_type_name = next_is_type_name(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    uint32_t lhs;
    if (is_type_name) {
        lhs = parse_type_name_2(s, ast);
    } else {
        lhs = parse_const_expr_2(s, ast);
    }
    CHECK_ERR(lhs);

    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    return res;
}

static uint32_t parse_type_spec_qual_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    if (curr_is_type_qual(s)) {
        return parse_type_qual_2(s, ast);
    } else if (is_type_spec(s)) {
        return parse_type_spec_2(s, ast);
    } else if (ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
        return parse_align_spec_2(s, ast);
    } else {
        static const TokenKind ex[] = {
            // type_spec
            TOKEN_VOID,
            TOKEN_CHAR,
            TOKEN_SHORT,
            TOKEN_INT,
            TOKEN_LONG,
            TOKEN_FLOAT,
            TOKEN_DOUBLE,
            TOKEN_SIGNED,
            TOKEN_UNSIGNED,
            // TODO: _BitInt
            TOKEN_BOOL,
            TOKEN_COMPLEX,
            // TODO: Decimal(32, 64, 128)
            TOKEN_ATOMIC,
            TOKEN_STRUCT,
            TOKEN_UNION,
            TOKEN_ENUM,
            TOKEN_IDENTIFIER, // typedef name
            // TODO: typeof and typeof_unqual
            // type_qual
            TOKEN_CONST,
            TOKEN_RESTRICT,
            TOKEN_VOLATILE,
            TOKEN_ATOMIC,
            // func_spec
            TOKEN_INLINE,
            TOKEN_NORETURN,
        };
        expected_tokens_error(s, ex, ARR_LEN(ex));
        return 0;
    }
}

static uint32_t parse_spec_qual_list_without_attrs_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST, s->it);
    CHECK_ERR(parse_type_spec_qual_2(s, ast));

    while (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
        CHECK_ERR(parse_type_spec_qual_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast) {
    // TODO: this may not be needed
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST_ATTR, s->it);
    CHECK_ERR(parse_spec_qual_list_without_attrs_2(s, ast));

    // TODO: might need more than that to check
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static bool is_balanced_token(TokenKind k) {
    switch (k) {
        case TOKEN_RBRACKET:
        case TOKEN_RINDEX:
        case TOKEN_RBRACE:
            return false;
        default:
            return true;
    }
}

static TokenKind get_rbracket(TokenKind bracket) {
    assert(bracket == TOKEN_LBRACKET || bracket == TOKEN_LINDEX
           || bracket == TOKEN_LBRACE);
    switch (bracket) {
        case TOKEN_LBRACKET:
            return TOKEN_RBRACKET;
        case TOKEN_LINDEX:
            return TOKEN_RINDEX;
        case TOKEN_LBRACE:
            return TOKEN_RBRACE;
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_balanced_token_sequence(ParserState* s, AST* ast);

static uint32_t balanced_token_bracket(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX
           || ParserState_curr_kind(s) == TOKEN_LBRACE);

    const uint32_t res = add_node(ast, AST_BALANCED_TOKEN_BRACKET, s->it);
    const TokenKind rbracket = get_rbracket(ParserState_curr_kind(s));
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == rbracket) {
        ParserState_accept_it(s);
        return res;
    }
    CHECK_ERR(parse_balanced_token_sequence(s, ast));
    CHECK_ERR(ParserState_accept(s, rbracket));
    return res;
}

static uint32_t parse_balanced_token(ParserState* s, AST* ast) {
    assert(is_balanced_token(ParserState_curr_kind(s)));
    const uint32_t start_idx = s->it;
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LBRACKET:
        case TOKEN_LINDEX:
        case TOKEN_LBRACE:
            return balanced_token_bracket(s, ast);
        default:
            ParserState_accept_it(s);
            return add_node(ast, AST_BALANCED_TOKEN, start_idx);
    }
}

// TODO: I don't think this can fail
static uint32_t parse_balanced_token_sequence(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_BALANCED_TOKEN_SEQUENCE, s->it);

    // TODO: buffer overflow
    while (is_balanced_token(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_balanced_token(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_attribute_argument_clause_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_ARGUMENT_CLAUSE, s->it);
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ParserState_accept_it(s);
        return res;
    }
    CHECK_ERR(parse_balanced_token_sequence(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    return res;
}

static uint32_t parse_attribute_2(ParserState* s, AST* ast) {
    // TODO: if next token is '::' lhs is attribute_prefixed_token
    const uint32_t start_idx = s->it;
    CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
    const uint32_t res = add_node(ast, AST_ATTRIBUTE, start_idx);
    add_node(ast, AST_IDENTIFIER, start_idx);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET) {
        const uint32_t rhs = parse_attribute_argument_clause_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[rhs].rhs = rhs;
    }
    return res;
}

static uint32_t parse_attribute_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_LIST, s->it);
    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    CHECK_ERR(parse_attribute_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RINDEX) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_attribute_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_attribute_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    ParserState_accept_it(s);
    do {
        if (!(ParserState_accept(s, TOKEN_LINDEX))) {
            mycc_printf("Goot\n", 0);
            return 0;
        }
    } while (0);

    const uint32_t res = parse_attribute_list_2(s, ast);
    CHECK_ERR(res);

    CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX)
              && ParserState_accept(s, TOKEN_RINDEX));

    return res;
}

static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_SPEC_SEQUENCE, s->it);
    CHECK_ERR(parse_attribute_spec_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_token_range(ParserState* s,
                                  AST* ast,
                                  ASTNodeKind kind,
                                  bool (*pred)(TokenKind)) {
    assert(pred(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, kind, s->it);
    ParserState_accept_it(s);
    uint32_t len = 1;
    while (pred(ParserState_curr_kind(s))) {
        ParserState_accept_it(s);
        ++len;
    }
    // here rhs is not a node index, but how many tokens from token_idx are part
    // of the range
    ast->datas[res].rhs = len;
    return res;
}

static uint32_t parse_type_qual_list_2(ParserState* s, AST* ast) {
    return parse_token_range(s, ast, AST_TYPE_QUAL_LIST, is_type_qual);
}

static uint32_t parse_pointer_attrs_and_quals_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX
           || is_type_qual(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_POINTER_ATTRS_AND_QUALS, s->it);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    if (is_type_qual(ParserState_curr_kind(s))) {
        const uint32_t rhs = parse_type_qual_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ASTERISK);
    const uint32_t res = add_node(ast, AST_POINTER, s->it);
    ParserState_accept_it(s);

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LINDEX || is_type_qual(curr_kind)) {
        CHECK_ERR(parse_pointer_attrs_and_quals_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        const uint32_t rhs = parse_pointer_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_abs_arr_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_STATIC) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast, AST_ABS_ARR_SUFFIX_STATIC, idx);
        if (is_type_qual(ParserState_curr_kind(s))) {
            CHECK_ERR(parse_type_qual_list_2(s, ast));
        }
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);

        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    } else if (curr_kind == TOKEN_ASTERISK) {
        ParserState_accept_it(s);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        return add_node(ast, AST_ABS_ARR_SUFFIX_ASTERISK, idx);
    } else if (is_type_qual(curr_kind)) {
        const uint32_t res = add_node(ast, AST_ABS_ARR_SUFFIX, idx);
        CHECK_ERR(parse_type_qual_list_2(s, ast));

        switch (ParserState_curr_kind(s)) {
            case TOKEN_RINDEX: {
                ParserState_accept_it(s);
                return res;
            }
            case TOKEN_STATIC:
                ast->kinds[res] = AST_ABS_ARR_SUFFIX_STATIC;
                ParserState_accept_it(s);
                FALLTHROUGH();
            default: {
                const uint32_t rhs = parse_assign_expr_2(s, ast);
                CHECK_ERR(rhs);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                ast->datas[res].rhs = rhs;
                return res;
            }
        }
    } else if (curr_kind == TOKEN_RINDEX) {
        ParserState_accept_it(s);
        return add_node(ast, AST_ABS_ARR_SUFFIX, idx);
    } else {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx);
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    }
}

static uint32_t parse_abs_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node(ast, AST_ABS_FUNC_SUFFIX, s->it);
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_RBRACKET) {
        ParserState_accept_it(s);
    } else {
        CHECK_ERR(parse_param_type_list_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    }
    return res;
}

static uint32_t parse_abs_arr_or_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ABS_ARR_OR_FUNC_SUFFIX, s->it);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            CHECK_ERR(parse_abs_arr_suffix_2(s, ast));
            break;
        case TOKEN_LBRACKET:
            CHECK_ERR(parse_abs_func_suffix_2(s, ast));
            break;
        default:
            UNREACHABLE();
    }

    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_abs_arr_or_func_suffix_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ABS_ARR_OR_FUNC_SUFFIX_LIST, s->it);
    CHECK_ERR(parse_abs_arr_or_func_suffix_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_abs_arr_or_func_suffix_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_abs_declarator_2(ParserState* s, AST* ast);

static uint32_t parse_direct_abs_declarator_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_DIRECT_ABS_DECLARATOR, s->it);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    const TokenKind next_kind = ParserState_next_token_kind(s);
    if (curr_kind == TOKEN_LBRACKET
        && (next_kind == TOKEN_ASTERISK || next_kind == TOKEN_LBRACKET
            || next_kind == TOKEN_LINDEX)) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_abs_declarator_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_abs_arr_or_func_suffix_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_abs_declarator_2(ParserState* s, AST* ast) {
    // TODO: this might need a type
    const uint32_t res = add_node(ast, AST_ABS_DECLARATOR, s->it);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LBRACKET || curr_kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_direct_abs_declarator_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    // If the ast len has not changed, lhs and rhs are empty
    if (ast->len == res + 1) {
        ParserErr_set(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it);
        return 0;
    }

    return res;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TYPE_NAME, s->it);
    CHECK_ERR(parse_spec_qual_list_2(s, ast));

    const TokenKind kind = ParserState_curr_kind(s);
    if (kind == TOKEN_ASTERISK || kind == TOKEN_LBRACKET
        || kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_abs_declarator_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_generic_assoc_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node_with_type(ast, AST_GENERIC_ASSOC, s->it);
    if (ParserState_curr_kind(s) == TOKEN_DEFAULT) {
        ParserState_accept_it(s);
    } else {
        CHECK_ERR(parse_type_name_2(s, ast));
    }

    CHECK_ERR(ParserState_accept(s, TOKEN_COLON));

    const uint32_t rhs = parse_assign_expr_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_generic_assoc_list_2(ParserState* s, AST* ast) {
    // TODO: this probably does not need a node
    const uint32_t res = add_node_with_type(ast, AST_GENERIC_ASSOC_LIST, s->it);

    CHECK_ERR(parse_generic_assoc_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_generic_assoc_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_generic_sel_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    const uint32_t res = add_node_with_type(ast, AST_GENERIC_SEL, s->it);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    // lhs
    CHECK_ERR(parse_assign_expr_2(s, ast));

    CHECK_ERR(ParserState_accept(s, TOKEN_COMMA));

    const uint32_t rhs = parse_generic_assoc_list_2(s, ast);
    CHECK_ERR(rhs);
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_primary_expr_2(ParserState* s, AST* ast) {
    // TODO: this might not need a node
    const uint32_t res = add_node_with_type(ast, AST_PRIMARY_EXPR, s->it);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_enum_constant(s, ParserState_curr_spell(s))) {
                add_node_with_type(ast, AST_ENUM_CONSTANT, s->it);
            } else {
                add_node_with_type(ast, AST_IDENTIFIER, s->it);
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_I_CONSTANT:
        case TOKEN_F_CONSTANT:
            add_node_with_type(ast, AST_CONSTANT, s->it);
            ParserState_accept_it(s);
            break;
        case TOKEN_STRING_LITERAL:
            add_node_with_type(ast, AST_STRING_LITERAL, s->it);
            ParserState_accept_it(s);
            break;
        case TOKEN_FUNC_NAME:
            add_node_with_type(ast, AST_FUNC, s->it);
            ParserState_accept_it(s);
            break;
        case TOKEN_LBRACKET:
            ParserState_accept_it(s);
            CHECK_ERR(parse_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            break;
        case TOKEN_GENERIC:
            CHECK_ERR(parse_generic_sel_2(s, ast));
            break;
        default: {
            static const TokenKind ex[] = {
                TOKEN_IDENTIFIER,
                TOKEN_I_CONSTANT,
                TOKEN_F_CONSTANT,
                TOKEN_STRING_LITERAL,
                TOKEN_FUNC_NAME,
                TOKEN_LBRACKET,
            };
            expected_tokens_error(s, ex, ARR_LEN(ex));
            return 0;
        }
    }

    return res;
}

static uint32_t parse_storage_class_specs_2(ParserState* s, AST* ast) {
    return parse_token_range(s,
                             ast,
                             AST_STORAGE_CLASS_SPECS,
                             is_storage_class_spec);
}

static uint32_t parse_compound_literal_type_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_COMPOUND_LITERAL_TYPE, s->it);
    if (is_storage_class_spec(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_storage_class_specs_2(s, ast));
    }

    const uint32_t rhs = parse_type_name_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static bool is_designator(TokenKind k) {
    switch (k) {
        case TOKEN_LINDEX:
        case TOKEN_DOT:
            return true;
        default:
            return false;
    }
}

static uint32_t parse_designator_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    // TODO: might not need node
    const uint32_t res = add_node_with_type(ast, AST_DESIGNATOR, s->it);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            ParserState_accept_it(s);
            CHECK_ERR(parse_const_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
            break;
        case TOKEN_DOT:
            ParserState_accept_it(s);
            const uint32_t id_idx = s->it;
            CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
            add_node_with_type(ast, AST_IDENTIFIER, id_idx);
            break;
        default:
            UNREACHABLE();
    }
    return res;
}

static uint32_t parse_designator_list_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_DESIGNATOR_LIST, s->it);
    CHECK_ERR(parse_designator_2(s, ast));

    while (is_designator(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_designator_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast);

static uint32_t parse_initializer_2(ParserState* s, AST* ast) {
    // TODO: Might not need a node
    const uint32_t res = add_node(ast, AST_INITIALIZER, s->it);
    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        CHECK_ERR(parse_braced_initializer_2(s, ast));
    } else {
        CHECK_ERR(parse_assign_expr_2(s, ast));
    }
    return res;
}

static uint32_t parse_designation_init_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DESIGNATION_INIT, s->it);
    if (is_designator(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_designator_list_2(s, ast));

        CHECK_ERR(ParserState_accept(s, TOKEN_ASSIGN));
    }

    const uint32_t rhs = parse_initializer_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_init_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_INIT_LIST, s->it);
    CHECK_ERR(parse_designation_init_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);

        CHECK_ERR(parse_designation_init_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_BRACED_INITIALIZER, s->it);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACE));

    if (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        const uint32_t rhs = parse_init_list_2(s, ast);
        CHECK_ERR(rhs);

        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        ParserState_accept_it(s);
        return res;
    }
}

static uint32_t parse_compound_literal_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node_with_type(ast, AST_COMPOUND_LITERAL, s->it);
    ParserState_accept_it(s);
    CHECK_ERR(parse_compound_literal_type_2(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));

    const uint32_t rhs = parse_braced_initializer_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_arg_expr_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ARG_EXPR_LIST, s->it);
    // list is empty
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    CHECK_ERR(parse_assign_expr_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        CHECK_ERR(parse_assign_expr_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static bool is_postfix_op(TokenKind t) {
    switch (t) {
        case TOKEN_LINDEX:
        case TOKEN_LBRACKET:
        case TOKEN_DOT:
        case TOKEN_PTR_OP:
        case TOKEN_INC:
        case TOKEN_DEC:
            return true;

        default:
            return false;
    }
}

static uint32_t parse_postfix_expr_2(ParserState* s, AST* ast) {
    // TODO: might not need node
    const uint32_t res = add_node_with_type(ast, AST_POSTFIX_EXPR, s->it);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        CHECK_ERR(parse_compound_literal_2(s, ast));
    } else {
        CHECK_ERR(parse_primary_expr_2(s, ast));
    }

    uint32_t curr_node_idx = res;
    while (is_postfix_op(ParserState_curr_kind(s))) {
        const uint32_t rhs = add_node_with_type(ast,
                                                AST_TRANSLATION_UNIT,
                                                s->it);
        const TokenKind curr_kind = ParserState_curr_kind(s);
        switch (curr_kind) {
            case TOKEN_LINDEX:
                ast->kinds[rhs] = AST_POSTFIX_OP_INDEX;
                ParserState_accept_it(s);
                CHECK_ERR(parse_expr_2(s, ast));
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                break;
            case TOKEN_LBRACKET:
                ast->kinds[rhs] = AST_POSTFIX_OP_CALL;
                ParserState_accept_it(s);
                CHECK_ERR(parse_arg_expr_list_2(s, ast));
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                break;
            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                ast->kinds[rhs] = curr_kind == TOKEN_DOT
                                      ? AST_POSTFIX_OP_ACCESS
                                      : AST_POSTFIX_OP_PTR_ACCESS;
                ParserState_accept_it(s);
                const uint32_t id_idx = s->it;
                CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
                add_node_with_type(ast, AST_IDENTIFIER, id_idx);
                break;
            case TOKEN_INC:
            case TOKEN_DEC:
                ast->kinds[rhs] = curr_kind == TOKEN_INC ? AST_POSTFIX_OP_INC
                                                         : AST_POSTFIX_OP_DEC;
                ParserState_accept_it(s);
                break;
            default:
                UNREACHABLE();
        }
        ast->datas[curr_node_idx].rhs = rhs;
        assert(ast->kinds[rhs] != AST_TRANSLATION_UNIT);
        curr_node_idx = rhs;
    }

    return res;
}

static uint32_t parse_unary_expr_2(ParserState* s, AST* ast);

// TODO: does not work with compound literals
static uint32_t parse_cast_expr_2(ParserState* s, AST* ast) {
    // TODO: might not need a node
    const uint32_t start_token_idx = s->it;
    const uint32_t start_len = ast->len;
    const uint32_t res = add_node_with_type(ast, AST_CAST_EXPR, s->it);
    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_type_name_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
        if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
            // TODO: this is herrendous, fix this
            s->it = start_token_idx;
            ast->len = start_len;
            return parse_postfix_expr_2(s, ast);
        }

        const uint32_t rhs = parse_cast_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    } else {
        CHECK_ERR(parse_unary_expr_2(s, ast));
    }
    return res;
}

static ASTNodeKind unary_op_to_unary_node_kind(TokenKind k) {
    switch (k) {
        case TOKEN_AND:
            return AST_UNARY_EXPR_ADDRESSOF;
        case TOKEN_ASTERISK:
            return AST_UNARY_EXPR_DEREF;
        case TOKEN_ADD:
            return AST_UNARY_EXPR_PLUS;
        case TOKEN_SUB:
            return AST_UNARY_EXPR_MINUS;
        case TOKEN_BNOT:
            return AST_UNARY_EXPR_BNOT;
        case TOKEN_NOT:
            return AST_UNARY_EXPR_NOT;
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_unary_expr_2(ParserState* s, AST* ast) {
    const TokenKind curr_kind = ParserState_curr_kind(s);
    switch (curr_kind) {
        case TOKEN_INC:
        case TOKEN_DEC: {
            const ASTNodeKind kind = curr_kind == TOKEN_INC
                                         ? AST_UNARY_EXPR_INC
                                         : AST_UNARY_EXPR_DEC;
            const uint32_t res = add_node_with_type(ast, kind, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(parse_unary_expr_2(s, ast));
            return res;
        }
        case TOKEN_SIZEOF: {
            const uint32_t res = add_node_with_type(ast,
                                                    AST_UNARY_EXPR_SIZEOF,
                                                    s->it);
            ParserState_accept_it(s);
            const uint32_t ast_len = ast->len;
            const uint32_t start_idx = s->it;
            if (ParserState_curr_kind(s) == TOKEN_LBRACKET) {
                if (next_is_type_name(s)) {
                    ParserState_accept_it(s);
                    CHECK_ERR(parse_type_name_2(s, ast));
                    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));

                    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
                        // TODO: fix this herrendous shit
                        ast->len = ast_len;
                        s->it = start_idx;
                        CHECK_ERR(parse_postfix_expr_2(s, ast));
                    }
                } else {
                    CHECK_ERR(parse_unary_expr_2(s, ast));
                }
            } else {
                CHECK_ERR(parse_unary_expr_2(s, ast));
            }
            return res;
        }
        case TOKEN_ALIGNOF: {
            const uint32_t res = add_node_with_type(ast,
                                                    AST_UNARY_EXPR_ALIGNOF,
                                                    s->it);
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            CHECK_ERR(parse_type_name_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            return res;
        }
        case TOKEN_AND:
        case TOKEN_ASTERISK:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT: {
            const ASTNodeKind kind = unary_op_to_unary_node_kind(curr_kind);
            const uint32_t res = add_node_with_type(ast, kind, s->it);
            ParserState_accept_it(s);
            CHECK_ERR(parse_cast_expr_2(s, ast));
            return res;
        }
        default:
            return parse_postfix_expr_2(s, ast);
    }
}

static uint32_t parse_binary_expr_multiple_ops(
    ParserState* s,
    AST* ast,
    ASTNodeKind default_kind,
    uint32_t (*parse_child)(ParserState*, AST*),
    ASTNodeKind (*op_to_node_kind)(TokenKind)) {
    // TODO: might not need node
    const uint32_t res = add_node_with_type(ast, default_kind, s->it);
    CHECK_ERR(parse_child(s, ast));

    const ASTNodeKind kind = op_to_node_kind(ParserState_curr_kind(s));
    if (kind != AST_TRANSLATION_UNIT) {
        ast->kinds[res] = kind;
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_multiple_ops(s,
                                                            ast,
                                                            default_kind,
                                                            parse_child,
                                                            op_to_node_kind);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static ASTNodeKind mul_op_to_mul_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ASTERISK:
            return AST_MUL_EXPR;
        case TOKEN_DIV:
            return AST_DIV_EXPR;
        case TOKEN_MOD:
            return AST_MOD_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_mul_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_MUL_EXPR,
                                          parse_cast_expr_2,
                                          mul_op_to_mul_expr_kind);
}

static ASTNodeKind add_op_to_add_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ADD:
            return AST_ADD_EXPR;
        case TOKEN_SUB:
            return AST_SUB_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_add_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_ADD_EXPR,
                                          parse_mul_expr_2,
                                          add_op_to_add_expr_kind);
}

static ASTNodeKind shift_op_to_shift_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_LSHIFT:
            return AST_LSHIFT_EXPR;
        case TOKEN_RSHIFT:
            return AST_RSHIFT_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_shift_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_LSHIFT_EXPR,
                                          parse_add_expr_2,
                                          shift_op_to_shift_expr_kind);
}

static ASTNodeKind rel_op_to_rel_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_LT:
            return AST_REL_EXPR_LT;
        case TOKEN_GT:
            return AST_REL_EXPR_GT;
        case TOKEN_LE:
            return AST_REL_EXPR_LE;
        case TOKEN_GE:
            return AST_REL_EXPR_GE;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_rel_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_REL_EXPR_GT,
                                          parse_shift_expr_2,
                                          rel_op_to_rel_expr_kind);
}

static ASTNodeKind eq_op_to_eq_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_EQ:
            return AST_EQ_EXPR;
        case TOKEN_NE:
            return AST_NE_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_eq_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_EQ_EXPR,
                                          parse_rel_expr_2,
                                          eq_op_to_eq_expr_kind);
}

static uint32_t parse_binary_expr_single_op(
    ParserState* s,
    AST* ast,
    uint32_t (*parse_child)(ParserState*, AST* ast),
    TokenKind op,
    ASTNodeKind kind) {
    const uint32_t res = add_node_with_type(ast, kind, s->it);
    CHECK_ERR(parse_child(s, ast));

    if (ParserState_curr_kind(s) == op) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_single_op(s,
                                                         ast,
                                                         parse_child,
                                                         op,
                                                         kind);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_and_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_eq_expr_2,
                                       TOKEN_AND,
                                       AST_AND_EXPR);
}

static uint32_t parse_xor_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_and_expr_2,
                                       TOKEN_XOR,
                                       AST_XOR_EXPR);
}

static uint32_t parse_or_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_xor_expr_2,
                                       TOKEN_OR,
                                       AST_OR_EXPR);
}

static uint32_t parse_log_and_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_or_expr_2,
                                       TOKEN_LAND,
                                       AST_LOG_AND_EXPR);
}

static uint32_t parse_log_or_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_log_and_expr_2,
                                       TOKEN_LOR,
                                       AST_LOG_OR_EXPR);
}

static uint32_t parse_cond_expr_2(ParserState* s, AST* ast);

static uint32_t parse_cond_items_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node_with_type(ast, AST_COND_ITEMS, s->it);
    CHECK_ERR(parse_expr_2(s, ast));

    CHECK_ERR(ParserState_accept(s, TOKEN_COLON));

    const uint32_t rhs = parse_cond_expr_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_cond_expr_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node_with_type(ast, AST_COND_EXPR, s->it);
    CHECK_ERR(parse_log_or_expr_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_QMARK) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_cond_items_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static ASTNodeKind assign_op_to_assign_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ASSIGN:
            return AST_ASSIGN;
        case TOKEN_MUL_ASSIGN:
            return AST_ASSIGN_MUL;
        case TOKEN_DIV_ASSIGN:
            return AST_ASSIGN_DIV;
        case TOKEN_MOD_ASSIGN:
            return AST_ASSIGN_MOD;
        case TOKEN_ADD_ASSIGN:
            return AST_ASSIGN_ADD;
        case TOKEN_SUB_ASSIGN:
            return AST_ASSIGN_SUB;
        case TOKEN_LSHIFT_ASSIGN:
            return AST_ASSIGN_SHL;
        case TOKEN_RSHIFT_ASSIGN:
            return AST_ASSIGN_SHR;
        case TOKEN_AND_ASSIGN:
            return AST_ASSIGN_AND;
        case TOKEN_XOR_ASSIGN:
            return AST_ASSIGN_XOR;
        case TOKEN_OR_ASSIGN:
            return AST_ASSIGN_OR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

// In the grammar it the first element is actually a unary expression, but
// because differentiating is actually not that easy, leave that error to
// contextanalysis
static uint32_t parse_assign_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_ASSIGN,
                                          parse_cond_expr_2,
                                          assign_op_to_assign_expr_kind);
}

static uint32_t parse_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_assign_expr_2,
                                       TOKEN_COMMA,
                                       AST_EXPR);
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast) {
    return parse_cond_expr_2(s, ast);
}
