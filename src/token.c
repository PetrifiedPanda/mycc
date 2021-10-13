#include "token.h"

#include <string.h>
#include <assert.h>

#include "error.h"

bool create_token(Token* t, TokenType type, const char* spelling, size_t line, size_t idx) {
    assert(t);
    t->spelling = malloc(sizeof(char) * (strlen(spelling) + 1));
    if (t->spelling) {
        strcpy(t->spelling, spelling);
        t->type = type;
        t->source_loc.line = line;
        t->source_loc.index = idx;
        return true;
    } else {
        set_error(ERR_ALLOC_FAIL, "Failed to allocate spelling for token");
        return false;
    }
}

void create_token_move(Token* t, TokenType type, char* spelling, size_t line, size_t idx) {
    assert(t);
    t->spelling = spelling;
    t->type = type;
    t->source_loc.line = line;
    t->source_loc.index = idx;
}

void free_token(Token* t) {
    assert(t);
    free(t->spelling);
}

const char* get_spelling(TokenType type) {
    switch (type) {
    
    case SIZEOF:
        return "sizeof";
    case PTR_OP:
        return "->";
    case INC_OP:
        return "++";
    case DEC_OP:
        return "--";
    case LEFT_OP:
        return "<<";
    case RIGHT_OP:
        return ">>";
    case LE_OP:
        return "<=";
    case GE_OP:
        return ">=";
    case EQ_OP:
        return "==";
    case NE_OP:
        return "!=";
    case AND_OP:
        return "&&";
    case OR_OP:
        return "||";
    case MUL_ASSIGN:
        return "*=";
    case DIV_ASSIGN:
        return "/=";
    case MOD_ASSIGN:
        return "%=";
    case ADD_ASSIGN:
        return "+=";
    case SUB_ASSIGN:
        return "-=";
    case LEFT_ASSIGN:
        return "<<=";
    case RIGHT_ASSIGN:
        return ">>=";
    case AND_ASSIGN:
        return "&=";
    case OR_ASSIGN:
        return "|=";
    case XOR_ASSIGN:
        return "^=";
    case TYPEDEF:
        return "typedef";
    case EXTERN:
        return "extern";
    case STATIC:
        return "static";
    case AUTO:
        return "auto";
    case REGISTER:
        return "register";
    case CHAR:
        return "char";
    case SHORT:
        return "short";
    case INT:
        return "int";
    case LONG:
        return "long";
    case SIGNED:
        return "signed";
    case UNSIGNED:
        return "unsigned";
    case FLOAT:
        return "float";
    case DOUBLE:
        return "double";
    case CONST:
        return "const";
    case VOLATILE:
        return "volatile";
    case VOID:
        return "void";
    case STRUCT:
        return "struct";
    case UNION:
        return "union";
    case ENUM:
        return "enum";
    case ELLIPSIS:
        return "...";
    case CASE:
        return "case";
    case DEFAULT:
        return "default";
    case IF:
        return "if";
    case ELSE:
        return "else";
    case SWITCH:
        return "switch";
    case WHILE:
        return "while";
    case DO:
        return "do";
    case FOR:
        return "for";
    case GOTO:
        return "goto";
    case CONTINUE:
        return "continue";
    case BREAK:
        return "break";
    case RETURN:
        return "return";
    case SEMICOLON:
        return ";";
    case LBRACKET:
        return "(";
    case RBRACKET:
        return ")";
    case LBRACE:
        return "{";
    case RBRACE:
        return "}";
    case LINDEX:
        return "[";
    case RINDEX:
        return "]";
    case DOT:
        return ".";
    case AND:
        return "&";
    case OR:
        return "|";
    case XOR:
        return "^";
    case NOT:
        return "!";
    case BNOT:
        return "~";
    case SUB:
        return "-";
    case ADD:
        return "+";
    case ASTERISK:
        return "*";
    case DIV:
        return "/";
    case MOD:
        return "%";
    case LT:
        return "<";
    case GT:
        return ">";
    case QMARK:
        return "?";
    case COLON:
        return ":";
    case ASSIGN:
        return "=";
    case COMMA:
        return ",";

    default:
        return "";

    }
}

const char* get_type_str(TokenType type) {
    switch (type) {
    
    case IDENTIFIER:
        return "IDENTIFIER";
    case CONSTANT:
        return "CONSTANT";
    case STRING_LITERAL:
        return "STRING_LITERAL";
    case SIZEOF:
        return "SIZEOF";
    case PTR_OP:
        return "PTR_OP";
    case INC_OP:
        return "INC_OP";
    case DEC_OP:
        return "DEC_OP";
    case LEFT_OP:
        return "LEFT_OP";
    case RIGHT_OP:
        return "RIGHT_OP";
    case LE_OP:
        return "LE_OP";
    case GE_OP:
        return "GE_OP";
    case EQ_OP:
        return "EQ_OP";
    case NE_OP:
        return "NE_OP";
    case AND_OP:
        return "AND_OP";
    case OR_OP:
        return "OR_OP";
    case MUL_ASSIGN:
        return "MUL_ASSIGN";
    case DIV_ASSIGN:
        return "DIV_ASSIGN";
    case MOD_ASSIGN:
        return "MOD_ASSIGN";
    case ADD_ASSIGN:
        return "ADD_ASSIGN";
    case SUB_ASSIGN:
        return "SUB_ASSIGN";
    case LEFT_ASSIGN:
        return "LEFT_ASSIGN";
    case RIGHT_ASSIGN:
        return "RIGHT_ASSIGN";
    case AND_ASSIGN:
        return "AND_ASSIGN";
    case OR_ASSIGN:
        return "OR_ASSIGN";
    case XOR_ASSIGN:
        return "XOR_ASSIGN";
    case TYPE_NAME:
        return "TYPE_NAME";
    case TYPEDEF:
        return "TYPEDEF";
    case EXTERN:
        return "EXTERN";
    case STATIC:
        return "STATIC";
    case AUTO:
        return "AUTO";
    case REGISTER:
        return "REGISTER";
    case CHAR:
        return "CHAR";
    case SHORT:
        return "SHORT";
    case INT:
        return "INT";
    case LONG:
        return "LONG";
    case SIGNED:
        return "SIGNED";
    case UNSIGNED:
        return "UNSIGNED";
    case FLOAT:
        return "FLOAT";
    case DOUBLE:
        return "DOUBLE";
    case CONST:
        return "CONST";
    case VOLATILE:
        return "VOLATILE";
    case VOID:
        return "VOID";
    case STRUCT:
        return "STRUCT";
    case UNION:
        return "UNION";
    case ENUM:
        return "ENUM";
    case ELLIPSIS:
        return "ELLIPSIS";
    case CASE:
        return "CASE";
    case DEFAULT:
        return "DEFAULT";
    case IF:
        return "IF";
    case ELSE:
        return "ELSE";
    case SWITCH:
        return "SWITCH";
    case WHILE:
        return "WHILE";
    case DO:
        return "DO";
    case FOR:
        return "FOR";
    case GOTO:
        return "GOTO";
    case CONTINUE:
        return "CONTINUE";
    case BREAK:
        return "BREAK";
    case RETURN:
        return "RETURN";
    case SEMICOLON:
        return "SEMICOLON";
    case LBRACKET:
        return "LBRACKET";
    case RBRACKET:
        return "RBRACKET";
    case LBRACE:
        return "LBRACE";
    case RBRACE:
        return "RBRACE";
    case LINDEX:
        return "LINDEX";
    case RINDEX:
        return "RINDEX";
    case DOT:
        return "DOT";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case XOR:
        return "XOR";
    case NOT:
        return "NOT";
    case BNOT:
        return "BNOT";
    case SUB:
        return "SUB";
    case ADD:
        return "ADD";
    case ASTERISK:
        return "ASTERISK";
    case DIV:
        return "DIV";
    case MOD:
        return "MOD";
    case LT:
        return "LT";
    case GT:
        return "GT";
    case QMARK:
        return "QMARK";
    case COLON:
        return "COLON";
    case ASSIGN:
        return "ASSIGN";
    case INVALID:
        return "INVALID";
    default:
        return "no applicable token type";
    }
}

bool is_unary_op(TokenType t) {
    switch (t) {
        case AND:
        case ASTERISK:
        case ADD:
        case SUB:
        case BNOT:
        case NOT:
           return true;
        default:
            return false; 
    }
}

bool is_assign_op(TokenType t) {
    switch (t) {
        case ASSIGN:
        case MUL_ASSIGN:
        case DIV_ASSIGN:
        case MOD_ASSIGN:
        case ADD_ASSIGN:
        case SUB_ASSIGN:
        case LEFT_ASSIGN:
        case RIGHT_ASSIGN:
        case AND_ASSIGN:
        case XOR_ASSIGN:
        case OR_ASSIGN:
            return true;
        default:
            return false;
    }
}

bool is_storage_class_spec(TokenType t) {
    switch (t) {
        case TYPEDEF:
        case EXTERN:
        case STATIC:
        case AUTO:
        case REGISTER:
            return true;
        default:
            return false; 
    }
}

bool is_keyword_type_spec(TokenType t) {
    switch (t) {
        case VOID:
        case CHAR:
        case SHORT:
        case INT:
        case LONG:
        case FLOAT:
        case DOUBLE:
        case SIGNED:
        case UNSIGNED:
            return true;
        default:
            return false;
    }
}
