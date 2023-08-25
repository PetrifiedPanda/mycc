#include "frontend/ast/ast.h"

#include <string.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void AST_ensure_capacity(AST* ast) {
    if (ast->len == ast->cap) {
        mycc_grow_alloc((void**)&ast->kinds, &ast->cap, sizeof *ast->kinds);
        ast->datas = mycc_realloc(ast->datas, sizeof *ast->datas * ast->cap);
    }
}

static uint32_t add_node(AST* ast,
                         ASTNodeKind kind,
                         uint32_t main_token,
                         bool alloc_type_data) {
    AST_ensure_capacity(ast);

    uint32_t type_data_idx = (uint32_t)-1;
    if (alloc_type_data) {
        if (ast->type_data_len == ast->type_data_cap) {
            mycc_grow_alloc((void**)&ast->type_data,
                            &ast->type_data_cap,
                            sizeof *ast->type_data);
        }
        type_data_idx = ast->type_data_len;
        ++ast->type_data_len;
    }
    const uint32_t idx = ast->len;
    ast->kinds[ast->len] = kind;
    ast->datas[ast->len] = (ASTNodeData){
        .main_token = main_token,
        .rhs = 0, // has to be initialized after (because we don't have rhs yet)
        .type_data_idx = type_data_idx,
    };

    ++ast->len;
    return idx;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast);

static uint32_t parse_atomic_type_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ATOMIC);
    const uint32_t res = add_node(ast, AST_ATOMIC_TYPE_SPEC, s->_it, false);
    ParserState_accept_it(s);
    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return 0;
    }
    if (parse_type_name_2(s, ast) == 0) {
        return 0;
    }
    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        return 0;
    }
    return res;
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast);

static uint32_t parse_static_assert_declaration_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT);
    const uint32_t res = add_node(ast,
                                  AST_STATIC_ASSERT_DECLARATION,
                                  s->_it,
                                  false);
    ParserState_accept_it(s);
    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return 0;
    }

    if (parse_const_expr_2(s, ast) == 0) {
        return 0;
    }

    // TODO: message not optional in earlier c versions
    if (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        const uint32_t lit_idx = s->_it;
        if (!ParserState_accept(s, TOKEN_STRING_LITERAL)) {
            return 0;
        }
        // TODO: type might not be necessary in this node
        ast->datas[res].rhs = add_node(ast, AST_STRING_LITERAL, lit_idx, true);
    }

    if (!(ParserState_accept(s, TOKEN_RBRACKET)
          && ParserState_accept(s, TOKEN_SEMICOLON))) {
        return 0;
    }
    return res;
}

static uint32_t parse_direct_declarator_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast);

static uint32_t parse_declarator_2(ParserState* s, AST* ast) {
    // TODO: unsure about whether this needs a type
    const uint32_t res = add_node(ast, AST_DECLARATOR, s->_it, true);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        if (parse_pointer_2(s, ast) == 0) {
            return 0;
        }
    }

    const uint32_t rhs = parse_direct_declarator_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATOR, s->_it, false);

    if (ParserState_curr_kind(s) != TOKEN_COLON) {
        if (parse_declarator_2(s, ast) == 0) {
            return 0;
        }
    }

    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_const_expr_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_member_declarator_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATOR_LIST,
                                  s->_it,
                                  false);
    if (parse_member_declarator_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        if (parse_member_declarator_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast);

static uint32_t parse_member_declaration_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATION_BODY,
                                  s->_it,
                                  false);
    if (parse_spec_qual_list_2(s, ast) == 0) {
        return 0;
    }

    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
        return res;
    }

    const uint32_t rhs = parse_member_declarator_list_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    if (!ParserState_accept(s, TOKEN_SEMICOLON)) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast);

static uint32_t parse_member_declaration_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }

    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATION, s->_it, false);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        if (parse_attribute_spec_sequence_2(s, ast) == 0) {
            return 0;
        }
    }

    const uint32_t rhs = parse_member_declaration_body_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declaration_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATION_LIST,
                                  s->_it,
                                  false);
    if (parse_member_declaration_2(s, ast) == 0) {
        return 0;
    }

    // TODO: condition may be wrong
    while (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        if (parse_member_declaration_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_struct_union_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_STRUCT_UNION_BODY, s->_it, false);
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        add_node(ast, AST_IDENTIFIER, s->_it, false);
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_member_declaration_list_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        if (!ParserState_accept(s, TOKEN_RBRACE)) {
            return 0;
        }
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
    const uint32_t res = add_node(ast, kind, s->_it, false);
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        if (parse_attribute_spec_sequence_2(s, ast) == 0) {
            return 0;
        }
    }

    const uint32_t rhs = parse_struct_union_body_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_enum_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ENUM);
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_assign_expr_2(ParserState* s, AST* ast);

static uint32_t parse_type_spec_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    const uint32_t start_idx = s->_it;
    switch (curr_kind) {
        case TOKEN_VOID:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_VOID, start_idx, false);
        case TOKEN_CHAR:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_CHAR, start_idx, false);
        case TOKEN_SHORT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SHORT, start_idx, false);
        case TOKEN_INT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_INT, start_idx, false);
        case TOKEN_LONG:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_LONG, start_idx, false);
        case TOKEN_FLOAT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_FLOAT, start_idx, false);
        case TOKEN_DOUBLE:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_DOUBLE, start_idx, false);
        case TOKEN_SIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SIGNED, start_idx, false);
        case TOKEN_UNSIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_UNSIGNED, start_idx, false);
        case TOKEN_BOOL:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_BOOL, start_idx, false);
        case TOKEN_COMPLEX:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_COMPLEX, start_idx, false);
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
            return add_node(ast, AST_TYPE_SPEC_TYPEDEF_NAME, start_idx, false);
        // TODO: typeof, typeof_unqual
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_align_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->_it, false);
    ParserState_accept_it(s);
    const bool is_type_name = next_is_type_name(s);
    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return 0;
    }
    uint32_t rhs;
    if (is_type_name) {
        rhs = parse_type_name_2(s, ast);
    } else {
        rhs = parse_const_expr_2(s, ast);
    }
    if (rhs == 0) {
        return 0;
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_type_spec_qual_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    if (is_type_spec(s)) {
        return parse_type_spec_2(s, ast);
    } else if (is_type_qual(ParserState_curr_kind(s))) {
        return add_node(ast, AST_TYPE_QUAL, s->_it, false);
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
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST, s->_it, false);
    if (parse_type_spec_qual_2(s, ast) == 0) {
        return 0;
    }

    while (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
        if (parse_type_spec_qual_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast) {
    // TODO: this may not be needed
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST_ATTR, s->_it, false);
    if (parse_spec_qual_list_without_attrs_2(s, ast) == 0) {
        return 0;
    }

    // TODO: might need more than that to check
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_attribute_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_attribute_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_LIST, s->_it, false);
    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    while (ParserState_curr_kind(s) != TOKEN_RINDEX) {
        if (parse_attribute_2(s, ast) == 0) {
            return 0;
        }
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_attribute_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    ParserState_accept_it(s);
    if (!ParserState_accept(s, TOKEN_LINDEX)) {
        return 0;
    }

    const uint32_t res = parse_attribute_list_2(s, ast);
    if (res == 0) {
        return 0;
    }

    if (!(ParserState_accept(s, TOKEN_RINDEX)
          && ParserState_accept(s, TOKEN_RINDEX))) {
        return 0;
    }

    return res;
}

static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast,
                                  AST_ATTRIBUTE_SPEC_SEQUENCE,
                                  s->_it,
                                  false);
    if (parse_attribute_spec_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        if (parse_attribute_spec_2(s, ast) == 0) {
            return 0;
        }
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_token_range(ParserState* s,
                                  AST* ast,
                                  ASTNodeKind kind,
                                  bool (*pred)(TokenKind)) {
    assert(pred(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, kind, s->_it, false);
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
    const uint32_t res = add_node(ast,
                                  AST_POINTER_ATTRS_AND_QUALS,
                                  s->_it,
                                  false);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        if (parse_attribute_spec_sequence_2(s, ast) == 0) {
            return 0;
        }
    }

    if (is_type_qual(ParserState_curr_kind(s))) {
        const uint32_t rhs = parse_type_qual_list_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ASTERISK);
    const uint32_t res = add_node(ast, AST_POINTER, s->_it, false);
    ParserState_accept_it(s);

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LINDEX || is_type_qual(curr_kind)) {
        if (parse_pointer_attrs_and_quals_2(s, ast) == 0) {
            return 0;
        }
    }

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        const uint32_t rhs = parse_pointer_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_direct_abs_declarator_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_abs_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ABS_DECLARATOR, s->_it, false);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        if (parse_pointer_2(s, ast) == 0) {
            return 0;
        }
    }

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LBRACKET || curr_kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_direct_abs_declarator_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }

    // If the ast len has not changed, lhs and rhs are empty
    if (ast->len == res + 1) {
        ParserErr_set(s->err,
                      PARSER_ERR_EMPTY_DIRECT_ABS_DECL,
                      ParserState_curr_idx(s));
        return 0;
    }

    return res;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TYPE_NAME, s->_it, false);
    if (parse_spec_qual_list_2(s, ast) == 0) {
        return 0;
    }

    const TokenKind kind = ParserState_curr_kind(s);
    if (kind == TOKEN_ASTERISK || kind == TOKEN_LBRACKET
        || kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_abs_declarator_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_generic_assoc_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC, s->_it, true);
    if (ParserState_curr_kind(s) == TOKEN_DEFAULT) {
        ParserState_accept_it(s);
    } else {
        if (parse_type_name_2(s, ast) == 0) {
            return 0;
        }
    }

    if (!ParserState_accept(s, TOKEN_COLON)) {
        return 0;
    }

    const uint32_t rhs = parse_assign_expr_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_generic_assoc_list_2(ParserState* s, AST* ast) {
    // TODO: this probably does not need a node
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC_LIST, s->_it, true);

    if (parse_generic_assoc_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (parse_generic_assoc_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_generic_sel_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    const uint32_t res = add_node(ast, AST_GENERIC_SEL, s->_it, true);
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

static uint32_t parse_expr_2(ParserState* s, AST* ast);

static uint32_t parse_primary_expr_2(ParserState* s, AST* ast) {
    // TODO: this might not need a node
    const uint32_t res = add_node(ast, AST_PRIMARY_EXPR, s->_it, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_enum_constant(s, ParserState_curr_spell(s))) {
                add_node(ast, AST_ENUM_CONSTANT, s->_it, true);
            } else {
                add_node(ast, AST_IDENTIFIER, s->_it, true);
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_I_CONSTANT:
        case TOKEN_F_CONSTANT:
            add_node(ast, AST_CONSTANT, s->_it, true);
            ParserState_accept_it(s);
            break;
        case TOKEN_STRING_LITERAL:
            add_node(ast, AST_STRING_LITERAL, s->_it, true);
            ParserState_accept_it(s);
            break;
        case TOKEN_FUNC_NAME:
            add_node(ast, AST_FUNC, s->_it, true);
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
    const uint32_t res = add_node(ast,
                                  AST_COMPOUND_LITERAL_TYPE,
                                  s->_it,
                                  false);
    if (is_storage_class_spec(ParserState_curr_kind(s))) {
        if (parse_storage_class_specs_2(s, ast) == 0) {
            return 0;
        }
    }

    const uint32_t rhs = parse_type_name_2(s, ast);
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
    const uint32_t res = add_node(ast, AST_DESIGNATOR, s->_it, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            ParserState_accept_it(s);
            if (parse_const_expr_2(s, ast) == 0) {
                return 0;
            }
            if (!ParserState_accept(s, TOKEN_RINDEX)) {
                return 0;
            }
            break;
        case TOKEN_DOT:
            ParserState_accept_it(s);
            const uint32_t id_idx = s->_it;
            if (!ParserState_accept(s, TOKEN_IDENTIFIER)) {
                return 0;
            }
            add_node(ast, AST_IDENTIFIER, id_idx, true);
            break;
        default:
            UNREACHABLE();
    }
    return res;
}

static uint32_t parse_designator_list_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_DESIGNATOR_LIST, s->_it, false);
    if (parse_designator_2(s, ast) == 0) {
        return 0;
    }

    while (is_designator(ParserState_curr_kind(s))) {
        if (parse_designator_2(s, ast) == 0) {
            return 0;
        }
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast);

static uint32_t parse_initializer_2(ParserState* s, AST* ast) {
    // TODO: Might not need a node
    const uint32_t res = add_node(ast, AST_INITIALIZER, s->_it, false);
    if (ParserState_curr_kind(s) == TOKEN_RBRACE) {
        if (parse_braced_initializer_2(s, ast) == 0) {
            return 0;
        }
    } else {
        if (parse_assign_expr_2(s, ast) == 0) {
            return 0;
        }
    }
    return res;
}

static uint32_t parse_designation_init_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DESIGNATION_INIT, s->_it, false);
    if (is_designator(ParserState_curr_kind(s))) {
        if (parse_designator_list_2(s, ast) == 0) {
            return 0;
        }

        if (!ParserState_accept(s, TOKEN_ASSIGN)) {
            return 0;
        }
    }

    const uint32_t rhs = parse_initializer_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_init_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_INIT_LIST, s->_it, false);
    if (parse_designation_init_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);

        if (parse_designation_init_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_BRACED_INITIALIZER, s->_it, false);
    if (!ParserState_accept(s, TOKEN_LBRACE)) {
        return 0;
    }

    if (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        const uint32_t rhs = parse_init_list_2(s, ast);
        if (rhs == 0) {
            return 0;
        }

        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        if (!ParserState_accept(s, TOKEN_RBRACE)) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        ParserState_accept_it(s);
        return res;
    }
}

static uint32_t parse_compound_literal_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node(ast, AST_COMPOUND_LITERAL, s->_it, true);
    ParserState_accept_it(s);
    if (parse_compound_literal_type_2(s, ast) == 0) {
        return 0;
    }
    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        return 0;
    }

    const uint32_t rhs = parse_braced_initializer_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_arg_expr_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ARG_EXPR_LIST, s->_it, false);
    // list is empty
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    if (parse_assign_expr_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (parse_assign_expr_2(s, ast) == 0) {
            return 0;
        }
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
    const uint32_t res = add_node(ast, AST_POSTFIX_EXPR, s->_it, true);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        if (parse_compound_literal_2(s, ast) == 0) {
            return 0;
        }
    } else if (parse_primary_expr_2(s, ast) == 0) {
        return 0;
    }

    uint32_t curr_node_idx = res;
    while (is_postfix_op(ParserState_curr_kind(s))) {
        const uint32_t rhs = add_node(ast, AST_TRANSLATION_UNIT, s->_it, true);
        const TokenKind curr_kind = ParserState_curr_kind(s);
        switch (curr_kind) {
            case TOKEN_LINDEX:
                ast->kinds[rhs] = AST_POSTFIX_OP_INDEX;
                ParserState_accept_it(s);
                if (parse_expr_2(s, ast) == 0) {
                    return 0;
                }
                if (!ParserState_accept(s, TOKEN_RINDEX)) {
                    return 0;
                }
                break;
            case TOKEN_LBRACKET:
                ast->kinds[rhs] = AST_POSTFIX_OP_CALL;
                ParserState_accept_it(s);
                if (parse_arg_expr_list_2(s, ast) == 0) {
                    return 0;
                }
                if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                    return 0;
                }
                break;
            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                ast->kinds[rhs] = curr_kind == TOKEN_DOT
                                      ? AST_POSTFIX_OP_ACCESS
                                      : AST_POSTFIX_OP_PTR_ACCESS;
                ParserState_accept_it(s);
                const uint32_t id_idx = s->_it;
                if (!ParserState_accept(s, TOKEN_IDENTIFIER)) {
                    return 0;
                }
                add_node(ast, AST_IDENTIFIER, id_idx, true);
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

static uint32_t parse_cast_expr_2(ParserState* s, AST* ast) {
    // TODO: might not need a node
    const uint32_t res = add_node(ast, AST_CAST_EXPR, s->_it, true);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        ParserState_accept_it(s);
        if (parse_type_name_2(s, ast) == 0) {
            return 0;
        }
        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            return 0;
        }

        const uint32_t rhs = parse_cast_expr_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
    } else {
        if (parse_unary_expr_2(s, ast) == 0) {
            return 0;
        }
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
            const uint32_t res = add_node(ast, kind, s->_it, true);
            ParserState_accept_it(s);
            if (parse_unary_expr_2(s, ast) == 0) {
                return 0;
            }
            return res;
        }
        case TOKEN_SIZEOF: {
            const uint32_t res = add_node(ast,
                                          AST_UNARY_EXPR_SIZEOF,
                                          s->_it,
                                          true);
            ParserState_accept_it(s);
            if (ParserState_curr_kind(s) == TOKEN_LBRACKET) {
                if (next_is_type_name(s)) {
                    ParserState_accept_it(s);
                    if (parse_type_name_2(s, ast) == 0) {
                        return 0;
                    }
                    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                        return 0;
                    }
                } else if (parse_unary_expr_2(s, ast) == 0) {
                    return 0;
                }
            } else if (parse_unary_expr_2(s, ast) == 0) {
                return 0;
            }
            return res;
        }
        case TOKEN_ALIGNOF: {
            const uint32_t res = add_node(ast,
                                          AST_UNARY_EXPR_ALIGNOF,
                                          s->_it,
                                          true);
            ParserState_accept_it(s);
            if (!ParserState_accept(s, TOKEN_LBRACKET)) {
                return 0;
            }
            if (parse_type_name_2(s, ast) == 0) {
                return 0;
            }
            if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                return 0;
            }
            return res;
        }
        case TOKEN_AND:
        case TOKEN_ASTERISK:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT: {
            const ASTNodeKind kind = unary_op_to_unary_node_kind(curr_kind);
            const uint32_t res = add_node(ast, kind, s->_it, true);
            ParserState_accept_it(s);
            if (parse_cast_expr_2(s, ast) == 0) {
                return 0;
            }
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
    const uint32_t res = add_node(ast, default_kind, s->_it, true);
    if (parse_child(s, ast) == 0) {
        return 0;
    }

    const ASTNodeKind kind = op_to_node_kind(ParserState_curr_kind(s));
    if (kind != AST_TRANSLATION_UNIT) {
        ast->kinds[res] = kind;
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_multiple_ops(s,
                                                            ast,
                                                            default_kind,
                                                            parse_child,
                                                            op_to_node_kind);
        if (rhs == 0) {
            return 0;
        }
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
    const uint32_t res = add_node(ast, kind, s->_it, true);
    if (parse_child(s, ast) == 0) {
        return 0;
    }

    if (ParserState_curr_kind(s) == op) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_single_op(s,
                                                         ast,
                                                         parse_child,
                                                         op,
                                                         kind);
        if (rhs == 0) {
            return 0;
        }
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
    const uint32_t res = add_node(ast, AST_COND_ITEMS, s->_it, true);
    if (parse_expr_2(s, ast) == 0) {
        return 0;
    }

    if (!ParserState_accept(s, TOKEN_COLON)) {
        return 0;
    }

    const uint32_t rhs = parse_cond_expr_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_cond_expr_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_COND_EXPR, s->_it, true);
    if (parse_log_or_expr_2(s, ast) == 0) {
        return 0;
    }

    if (ParserState_curr_kind(s) == TOKEN_QMARK) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_cond_items_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
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
