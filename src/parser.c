#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"

typedef struct {
    const Token* it;
} ParserState;

void expected_token_error(TokenType expected, TokenType got) {
    set_error(PARSER, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got));
}

bool accept(ParserState* s, TokenType expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it->type);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(ParserState* s) {
    ++s->it;
}

static void parse_expr(ParserState* s);

static void parse_primary_expr(ParserState* s) {
    switch (s->it->type) {
    case IDENTIFIER:
        accept_it(s);
        break;
    case CONSTANT:
        accept_it(s);
        break;
    case STRING_LITERAL:
        accept_it(s);
        break;
    case LBRACKET:
        accept_it(s);
        parse_expr(s);
        accept(s, RBRACKET);
        break;
    default:
        expected_token_error(INVALID, s->it->type); // TODO: multiple expected
        break;
    }
}

static void parse_argument_expr_list(ParserState* s) {
    assert(false);
}

static bool is_primary_expr(TokenType t) {
    return t == IDENTIFIER || t == CONSTANT || t == STRING_LITERAL || t == LBRACKET;
}

static void parse_postfix_expr(ParserState* s) {
    if (is_primary_expr(s->it->type)) {
        parse_primary_expr(s);
    } else {
        // ERROR
    }

    while (s->it->type == LINDEX || s->it->type == LBRACKET || s->it->type == DOT || s->it->type == PTR_OP || s->it->type == INC_OP || s->it->type == DEC_OP) {
        switch (s->it->type) {
        case LINDEX:
            accept_it(s);
            parse_expr(s);
            accept(s, RINDEX);
            break;
        case LBRACKET:
            accept_it(s);
            if (s->it->type != RBRACKET) {
                parse_argument_expr_list(s);
            }
            accept(s, RBRACKET);
            break;
        case DOT:
            accept_it(s);
            accept(s, IDENTIFIER);
            break;
        case INC_OP:
        case DEC_OP:
            accept_it(s);
            break;
        default:
            // ERROR
            break;
        }
    }
}

static bool is_unary_operator(TokenType t) {
    return t == AND || t == ASTERISK || t == ADD || t == SUB || t == BNOT || t == NOT;
}

static void parse_type_name(ParserState* s) {
    assert(false);
}

static void parse_cast_expr(ParserState* s);

static bool is_unary_expr(TokenType t) {
    return is_primary_expr(t) || is_unary_operator(t) || t == INC_OP || t == DEC_OP || t == SIZEOF;
}

static void parse_unary_expr(ParserState* s) {
    if (is_primary_expr(s->it->type)) {
        parse_postfix_expr(s);
    } else if (is_unary_operator(s->it->type)) {
        accept_it(s);
        parse_cast_expr(s);
    } else {
        switch (s->it->type) {
        case INC_OP:
        case DEC_OP:
            accept_it(s);
            parse_unary_expr(s);
            break;
        case SIZEOF:
            accept_it(s);
            if (s->it->type == LBRACKET) {
                accept_it(s);
                parse_type_name(s);
                accept(s, RBRACKET);
            } else {
                parse_unary_expr(s);
            }
            break;
        }
    }
}

static void parse_cast_expr(ParserState* s) {
    while (s->it->type == LBRACKET) {
        accept_it(s);
        parse_type_name(s);
        accept(s, RBRACKET);
    }

    parse_unary_expr(s);
}

static void parse_muliplicative_expr(ParserState* s) {
    parse_cast_expr(s);
    while (s->it->type == ASTERISK || s->it->type == DIV || s->it->type == MOD) {
        accept_it(s);
        parse_cast_expr(s);
    }
}

static void parse_additive_expr(ParserState* s) {
    parse_muliplicative_expr(s);
    while (s->it->type == ADD || s->it->type == SUB) {
        accept_it(s);
        parse_muliplicative_expr(s);
    }
}

static void parse_shift_expr(ParserState* s) {
    parse_additive_expr(s);
    while (s->it->type == LEFT_OP || s->it->type == RIGHT_OP) {
        accept_it(s);
        parse_additive_expr(s);
    }
}

static void parse_relational_expr(ParserState* s) {
    parse_shift_expr(s);
    while (s->it->type == LT || s->it->type == GT || s->it->type == LE_OP || s->it->type == GE_OP) {
        accept_it(s);
        parse_shift_expr(s);
    }
}

static void parse_equality_expr(ParserState* s) {
    parse_relational_expr(s);
    while (s->it->type == EQ_OP || s->it->type == NE_OP) {
        accept_it(s);
        parse_relational_expr(s);
    }
}

static void parse_and_expr(ParserState* s) {
    parse_equality_expr(s);
    while (s->it->type == AND) {
        accept_it(s);
        parse_equality_expr(s);
    }
}

static void parse_xor_expr(ParserState* s) {
    parse_and_expr(s);
    while (s->it->type == XOR) {
        accept_it(s);
        parse_and_expr(s);
    }
}

static void parse_or_expr(ParserState* s) {
    parse_xor_expr(s);
    while (s->it->type == XOR) {
        accept_it(s);
        parse_xor_expr(s);
    }
}

static void parse_log_and_expr(ParserState* s) {
    parse_or_expr(s);
    while (s->it->type == AND_OP) {
        accept_it(s);
        parse_or_expr(s);
    }
}

static void parse_log_or_expr(ParserState* s) {
    parse_log_and_expr(s);
    while (s->it->type == OR_OP) {
        accept_it(s);
        parse_log_or_expr(s);
    }
}

static void parse_cond_expr(ParserState* s) {
    parse_log_or_expr(s);
    while (s->it->type == QMARK) {
        accept_it(s);
        parse_expr(s);
        accept(s, COLON);
        parse_cond_expr(s);
    }
}

static bool is_assign_op(TokenType t) {
    return t == ASSIGN || t == MUL_ASSIGN || t == DIV_ASSIGN || t == MOD_ASSIGN || t == ADD_ASSIGN || t == SUB_ASSIGN || t == LEFT_ASSIGN || t == RIGHT_ASSIGN || t == AND_ASSIGN || t == XOR_ASSIGN || t == OR_ASSIGN;
}

static void parse_assign_expr(ParserState* s) {
    while (is_unary_expr(s->it->type)) {
        parse_unary_expr(s);
        if (is_assign_op(s->it->type)) {
            accept_it(s);
        } else {
            // ERR
        }
    }

    parse_cond_expr(s);
}

static void parse_expr(ParserState* s) {
    parse_assign_expr(s);

    while (s->it->type == COMMA) {
        accept_it(s);
        parse_assign_expr(s);
    }
}

static void parse_constexpr(ParserState* s) {
    parse_cond_expr(s);
}