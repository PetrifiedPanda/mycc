#include "frontend/token_type.h"

#include <stddef.h>

const char* get_spelling(enum token_type type) {
    switch (type) {
        case FUNC_NAME:
            return "__func__";
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
        case INLINE:
            return "inline";
        case BOOL:
            return "_Bool";
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
        case VOID:
            return "void";
        case COMPLEX:
            return "_Complex";
        case IMAGINARY:
            return "_Imaginary";
        case CONST:
            return "const";
        case VOLATILE:
            return "volatile";
        case RESTRICT:
            return "restrict";
        case ATOMIC:
            return "_Atomic";
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
        case ALIGNAS:
            return "_Alignas";
        case ALIGNOF:
            return "_Alignof";
        case GENERIC:
            return "_Generic";
        case NORETURN:
            return "_Noreturn";
        case STATIC_ASSERT:
            return "_Static_assert";
        case THREAD_LOCAL:
            return "_Thread_local";
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
        case STRINGIFY_OP:
            return "#";
        case CONCAT_OP:
            return "##";

        default:
            return NULL;
    }
}

const char* get_type_str(enum token_type type) {
    switch (type) {
        case IDENTIFIER:
            return "IDENTIFIER";
        case I_CONSTANT:
            return "I_CONSTANT";
        case F_CONSTANT:
            return "F_CONSTANT";
        case STRING_LITERAL:
            return "STRING_LITERAL";
        case FUNC_NAME:
            return "FUNC_NAME";
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
        case TYPEDEF_NAME:
            return "TYPEDEF_NAME";
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
        case INLINE:
            return "INLINE";
        case BOOL:
            return "BOOL";
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
        case VOID:
            return "VOID";
        case COMPLEX:
            return "COMPLEX";
        case IMAGINARY:
            return "IMAGINARY";
        case CONST:
            return "CONST";
        case VOLATILE:
            return "VOLATILE";
        case RESTRICT:
            return "RESTRICT";
        case ATOMIC:
            return "ATOMIC";
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
        case ALIGNAS:
            return "ALIGNAS";
        case ALIGNOF:
            return "ALIGNOF";
        case GENERIC:
            return "GENERIC";
        case NORETURN:
            return "NORETURN";
        case STATIC_ASSERT:
            return "STATIC_ASSERT";
        case THREAD_LOCAL:
            return "THREAD_LOCAL";
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
        case COMMA:
            return "COMMA";
        case STRINGIFY_OP:
            return "STRINGIFY_OP";
        case CONCAT_OP:
            return "CONCAT_OP";
        case INVALID:
            return "INVALID";
    }
}

bool is_unary_op(enum token_type t) {
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

bool is_assign_op(enum token_type t) {
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

bool is_storage_class_spec(enum token_type t) {
    switch (t) {
        case TYPEDEF:
        case EXTERN:
        case STATIC:
        case THREAD_LOCAL:
        case AUTO:
        case REGISTER:
            return true;
        default:
            return false;
    }
}

bool is_keyword_type_spec(enum token_type t) {
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
        case BOOL:
        case COMPLEX:
        case IMAGINARY:
        case ATOMIC:
        case STRUCT:
        case UNION:
        case ENUM:
            return true;
        default:
            return false;
    }
}

bool is_type_qual(enum token_type t) {
    switch (t) {
        case CONST:
        case RESTRICT:
        case VOLATILE:
        case ATOMIC:
            return true;

        default:
            return false;
    }
}

bool is_func_spec(enum token_type t) {
    return t == INLINE || t == NORETURN;
}

bool is_shift_op(enum token_type t) {
    switch (t) {
        case LEFT_OP:
        case RIGHT_OP:
            return true;
        default:
            return false;
    }
}

bool is_rel_op(enum token_type t) {
    switch (t) {
        case LE_OP:
        case GE_OP:
        case LT:
        case GT:
            return true;
        default:
            return false;
    }
}

bool is_mul_op(enum token_type t) {
    switch (t) {
        case ASTERISK:
        case DIV:
        case MOD:
            return true;
        default:
            return false;
    }
}

bool is_add_op(enum token_type t) {
    switch (t) {
        case ADD:
        case SUB:
            return true;
        default:
            return false;
    }
}

bool is_eq_op(enum token_type t) {
    switch (t) {
        case EQ_OP:
        case NE_OP:
            return true;
        default:
            return false;
    }
}
