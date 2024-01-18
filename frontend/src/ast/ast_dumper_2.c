#include "frontend/ast/ast_dumper_2.h"

#include "util/macro_util.h"

static bool dump_ast_rec(const AST* ast, uint32_t node_idx, File f);

bool dump_ast_2(const AST* ast, const FileInfo* file_info, File f) {
    (void)file_info;
    return dump_ast_rec(ast, 0, f);
}

typedef enum {
    // Has lhs and optional rhs
    AST_NODE_KIND_TYPE_DEFAULT,
    AST_NODE_KIND_TYPE_SUBRANGE,
    AST_NODE_KIND_TYPE_NO_CHILDREN,
    AST_NODE_KIND_TYPE_TOKEN_RANGE,
    // All tokens where the relevant data is the spelling of an identifier
    AST_NODE_KIND_TYPE_IDENTIFIER,
    AST_NODE_KIND_TYPE_STRING_LITERAL,
    AST_NODE_KIND_TYPE_CONSTANT,
} ASTNodeKindType;

static ASTNodeKindType get_ast_node_kind_type(ASTNodeKind k);

static Str get_node_kind_str(ASTNodeKind k);

static void dump_value(File f, const Value* val) {
    File_put_str("value:", f);

    File_printf(f, "type: {Str}\n", ValueKind_str(val->kind));
    if (ValueKind_is_sint(val->kind)) {
        File_printf(f, "sint_val: {i64}\n", val->sint_val);
    } else if (ValueKind_is_uint(val->kind)) {
        File_printf(f, "uint_val: {u64}\n", val->uint_val);
    } else {
        File_printf(f, "float_val: {floatg}\n", val->float_val);
    }
}

static bool dump_ast_rec(const AST* ast, uint32_t node_idx, File f) {
    if (node_idx == ast->len) {
        return true;
    }
    const ASTNodeKind kind = ast->kinds[node_idx];
    const Str node_kind_str = get_node_kind_str(kind);

    File_printf(f, "{Str}:\n", node_kind_str);
    const ASTNodeKindType type = get_ast_node_kind_type(kind);

    const ASTNodeData data = ast->datas[node_idx];
    switch (type) {
        case AST_NODE_KIND_TYPE_DEFAULT:
            File_put_str("lhs: ", f);
            if (!dump_ast_rec(ast, node_idx + 1, f)) {
                return false;
            }
            const uint32_t rhs = data.rhs;
            if (rhs != 0) {
                File_put_str("rhs: ", f);
                return dump_ast_rec(ast, rhs, f);
            }
            return true;
        case AST_NODE_KIND_TYPE_NO_CHILDREN:
            return true;
        case AST_NODE_KIND_TYPE_SUBRANGE:
            // TODO:
            return false;
        case AST_NODE_KIND_TYPE_TOKEN_RANGE:
            // TODO:
            return false;
        case AST_NODE_KIND_TYPE_IDENTIFIER: {
            const uint32_t token_idx = data.main_token;
            const Str spell = StrBuf_as_str(&ast->toks.vals[token_idx].spelling);
            File_printf(f, "spelling: {Str}\n", spell);
            return true;
        }
        case AST_NODE_KIND_TYPE_STRING_LITERAL: {
            const uint32_t token_idx = data.main_token;
            const StrLit* lit = &ast->toks.vals[token_idx].str_lit;
            File_printf(f, "str_lit: {Str}\n", StrBuf_as_str(&lit->contents));
            return true;
        }
        case AST_NODE_KIND_TYPE_CONSTANT: {
            const uint32_t token_idx = data.main_token;
            const Value val = ast->toks.vals[token_idx].val;
            dump_value(f, &val);
            return true;
        }
    }
    UNREACHABLE();
}

static ASTNodeKindType get_ast_node_kind_type(ASTNodeKind k) {
    // TODO: balanced token, func_spec, storage_class_spec
    switch (k) {
        case AST_TRANSLATION_UNIT:
        case AST_DECLARATION_LIST:
        case AST_COMPOUND_STATEMENT:
        case AST_DECLARATION_SPECS:
        case AST_ENUM_LIST:
        case AST_MEMBER_DECLARATION_LIST:
        case AST_MEMBER_DECLARATOR_LIST:
        case AST_INIT_DECLARATOR_LIST:
        case AST_ATTRIBUTE_SPEC_SEQUENCE:
        case AST_ATTRIBUTE_LIST:
        case AST_BALANCED_TOKEN_SEQUENCE:
        case AST_ARR_OR_FUNC_SUFFIX_LIST:
        case AST_IDENTIFIER_LIST:
        case AST_PARAM_TYPE_LIST:
        case AST_PARAM_TYPE_LIST_VARIADIC:
        case AST_ABS_ARR_OR_FUNC_SUFFIX_LIST:
        case AST_INIT_LIST:
        case AST_DESIGNATOR_LIST:
        case AST_SPEC_QUAL_LIST:
        case AST_ARG_EXPR_LIST:
        case AST_GENERIC_ASSOC_LIST:
            return AST_NODE_KIND_TYPE_SUBRANGE;
        case AST_POSTFIX_OP_INC:
        case AST_POSTFIX_OP_DEC:
        case AST_TYPE_QUAL:
        case AST_ABS_ARR_SUFFIX_ASTERISK:
        case AST_TYPE_SPEC_VOID:
        case AST_TYPE_SPEC_CHAR:
        case AST_TYPE_SPEC_SHORT:
        case AST_TYPE_SPEC_INT:
        case AST_TYPE_SPEC_LONG:
        case AST_TYPE_SPEC_FLOAT:
        case AST_TYPE_SPEC_DOUBLE:
        case AST_TYPE_SPEC_SIGNED:
        case AST_TYPE_SPEC_UNSIGNED:
        case AST_TYPE_SPEC_BOOL:
        case AST_TYPE_SPEC_COMPLEX:
        case AST_TYPE_SPEC_IMAGINARY:
        case AST_BREAK_STATEMENT:
        case AST_CONTINUE_STATEMENT:
        case AST_FUNC:
            return AST_NODE_KIND_TYPE_NO_CHILDREN;
        case AST_TYPE_QUAL_LIST:
        case AST_STORAGE_CLASS_SPECS:
            return AST_NODE_KIND_TYPE_TOKEN_RANGE;
        case AST_IDENTIFIER:
        case AST_TYPE_SPEC_TYPEDEF_NAME:
        case AST_ENUM_CONSTANT:
            return AST_NODE_KIND_TYPE_IDENTIFIER;
        case AST_STRING_LITERAL:
            return AST_NODE_KIND_TYPE_STRING_LITERAL;
        case AST_CONSTANT:
            return AST_NODE_KIND_TYPE_CONSTANT;
        default:
            return AST_NODE_KIND_TYPE_DEFAULT;
    }
}

static Str get_node_kind_str(ASTNodeKind k) {
    switch (k) {
        case AST_TRANSLATION_UNIT:
            return STR_LIT("translation unit");
        case AST_FUNC_DEF:
            return STR_LIT("function definition");
        case AST_FUNC_DEF_IMPL:
            return STR_LIT("function definition implementation");
        case AST_FUNC_DEF_SUB_IMPL:
            return STR_LIT("function definition sub implementation");
        case AST_FUNC_DECLARATOR_AND_DECL_LIST:
            return STR_LIT("function declarator and declaration list");
        case AST_DECLARATION:
            return STR_LIT("declaration");
        case AST_DECLARATION_LIST:
            return STR_LIT("declaration list");
        case AST_DECLARATION_SPECS_AND_INIT_DECLARATOR_LIST:
            return STR_LIT("declaration list and initializer declarator list");
        case AST_ATTRIBUTE_DECLARATION:
            return STR_LIT("attribute declaration");
        case AST_STATIC_ASSERT_DECLARATION:
            return STR_LIT("static assert declaration");
        case AST_COMPOUND_STATEMENT:
            return STR_LIT("compound statement");
        case AST_LABEL:
            return STR_LIT("label");
        case AST_UNLABELED_STATEMENT:
            return STR_LIT("unlabeled statement");
        case AST_LABELED_STATEMENT:
            return STR_LIT("labeled statement");
        case AST_LABELED_STATEMENT_LABEL:
            return STR_LIT("labeled statement label");
        case AST_LABELED_STATEMENT_CASE:
            return STR_LIT("labeled statement case");
        case AST_SWITCH_STATEMENT:
            return STR_LIT("switch statement");
        case AST_SIMPLE_IF:
            return STR_LIT("simple if statement");
        case AST_IF_ELSE:
            return STR_LIT("if else statement");
        case AST_WHILE:
            return STR_LIT("while statement");
        case AST_DO_WHILE:
            return STR_LIT("do while statement");
        case AST_FOR:
            return STR_LIT("for statement");
        case AST_FOR_CLAUSE:
            return STR_LIT("for clause");
        case AST_FOR_LOOP_ACTIONS:
            return STR_LIT("for loop actions");
        case AST_GOTO_STATEMENT:
            return STR_LIT("goto statement");
        case AST_CONTINUE_STATEMENT:
            return STR_LIT("continue statement");
        case AST_BREAK_STATEMENT:
            return STR_LIT("break statement");
        case AST_RETURN_STATEMENT:
            return STR_LIT("return statement");
        case AST_DECLARATION_SPECS:
            return STR_LIT("declaration specifiers");
        case AST_STORAGE_CLASS_SPEC:
            return STR_LIT("storage class specifiers");
        case AST_TYPE_SPEC:
            return STR_LIT("type specifiers");
        case AST_TYPE_SPEC_VOID:
            return STR_LIT("void type specifier");
        case AST_TYPE_SPEC_CHAR:
            return STR_LIT("char type specifier");
        case AST_TYPE_SPEC_SHORT:
            return STR_LIT("short type specifier");
        case AST_TYPE_SPEC_INT:
            return STR_LIT("int type specifier");
        case AST_TYPE_SPEC_LONG:
            return STR_LIT("long type specifier");
        case AST_TYPE_SPEC_FLOAT:
            return STR_LIT("float type specifier");
        case AST_TYPE_SPEC_DOUBLE:
            return STR_LIT("double type specifier");
        case AST_TYPE_SPEC_SIGNED:
            return STR_LIT("signed type specifier");
        case AST_TYPE_SPEC_UNSIGNED:
            return STR_LIT("unsigned type specifier");
        case AST_TYPE_SPEC_BOOL:
            return STR_LIT("_Bool type specifier");
        case AST_TYPE_SPEC_COMPLEX:
            return STR_LIT("_Complex type specifier");
        case AST_TYPE_SPEC_IMAGINARY:
            return STR_LIT("_Imaginary type specifier");
        case AST_TYPE_SPEC_TYPEDEF_NAME:
            return STR_LIT("typedef name type specifier");
        case AST_ENUM_SPEC:
            return STR_LIT("enum specifier");
        case AST_ATTRIBUTE_ID:
            return STR_LIT("attribute identifier");
        case AST_ENUM_BODY:
            return STR_LIT("enum body");
        case AST_ENUM_LIST:
            return STR_LIT("enumerator list");
        case AST_ENUMERATOR:
            return STR_LIT("enumerator");
        case AST_ENUM_CONSTANT_AND_ATTRIBUTE:
            return STR_LIT("enum constant and attribute");
        case AST_STRUCT_SPEC:
            return STR_LIT("struct specifier");
        case AST_UNION_SPEC:
            return STR_LIT("union specifier");
        case AST_STRUCT_UNION_BODY:
            return STR_LIT("struct union body");
        case AST_MEMBER_DECLARATION_LIST:
            return STR_LIT("member declaration list");
        case AST_MEMBER_DECLARATION:
            return STR_LIT("member declaration");
        case AST_MEMBER_DECLARATION_BODY:
            return STR_LIT("member declaration body");
        case AST_MEMBER_DECLARATOR_LIST:
            return STR_LIT("member declarator list");
        case AST_MEMBER_DECLARATOR:
            return STR_LIT("member declarator");
        case AST_ATOMIC_TYPE_SPEC:
            return STR_LIT("atomic type specifier");
        case AST_FUNC_SPEC:
            return STR_LIT("function specifier");
        case AST_ALIGN_SPEC:
            return STR_LIT("alignment specifier");
        case AST_INIT_DECLARATOR_LIST:
            return STR_LIT("initializer declarator list");
        case AST_INIT_DECLARATOR:
            return STR_LIT("initializer declarator");
        case AST_DECLARATOR:
            return STR_LIT("declarator");
        case AST_POINTER:
            return STR_LIT("pointer");
        case AST_POINTER_ATTRS_AND_QUALS:
            return STR_LIT("pointer attributes and qualifiers");
        case AST_ATTRIBUTE_SPEC_SEQUENCE:
            return STR_LIT("attribute specifier sequence");
        case AST_ATTRIBUTE_SPEC:
            return STR_LIT("attribute specifier");
        case AST_ATTRIBUTE_LIST:
            return STR_LIT("attribute list");
        case AST_ATTRIBUTE:
            return STR_LIT("attribute");
        case AST_ATTRIBUTE_PREFIXED_TOKEN:
            return STR_LIT("attribute prefixed token");
        case AST_ATTRIBUTE_ARGUMENT_CLAUSE:
            return STR_LIT("attribute argument clause");
        case AST_BALANCED_TOKEN_SEQUENCE:
            return STR_LIT("balanced token sequence");
        case AST_BALANCED_TOKEN_BRACKET:
            return STR_LIT("balanced token bracket");
        case AST_BALANCED_TOKEN:
            return STR_LIT("balanced token");
        case AST_TYPE_QUAL_LIST:
            return STR_LIT("type qualifier list");
        case AST_DIRECT_DECLARATOR:
            return STR_LIT("direct declarator");
        case AST_ID_ATTRIBUTE:
            return STR_LIT("identifier attribute");
        case AST_ARR_OR_FUNC_SUFFIX_LIST:
            return STR_LIT("array or function suffix list");
        case AST_ARR_OR_FUNC_SUFFIX:
            return STR_LIT("array or function suffix");
        case AST_ARR_SUFFIX:
            return STR_LIT("array suffix");
        case AST_ARR_SUFFIX_STATIC:
            return STR_LIT("array suffix static");
        case AST_ARR_SUFFIX_ASTERISK:
            return STR_LIT("array suffix asterisk");
        case AST_FUNC_SUFFIX:
            return STR_LIT("function suffix");
        case AST_FUNC_SUFFIX_OLD:
            return STR_LIT("function suffix old");
        case AST_IDENTIFIER_LIST:
            return STR_LIT("identifier list");
        case AST_PARAM_TYPE_LIST:
            return STR_LIT("parameter type list");
        case AST_PARAM_TYPE_LIST_VARIADIC:
            return STR_LIT("parameter type list variadic");
        case AST_PARAM_DECLARATION:
            return STR_LIT("parameter declaration");
        case AST_ATTRS_AND_DECLARATION_SPECS:
            return STR_LIT("attributes and declaration specs");
        case AST_ABS_DECLARATOR:
            return STR_LIT("abstract declarator");
        case AST_DIRECT_ABS_DECLARATOR:
            return STR_LIT("direct abstract declarator");
        case AST_ABS_ARR_OR_FUNC_SUFFIX_LIST:
            return STR_LIT("abstract array or function suffix list");
        case AST_ABS_ARR_OR_FUNC_SUFFIX:
            return STR_LIT("abstract array or function suffix");
        case AST_ABS_ARR_SUFFIX:
            return STR_LIT("abstract array suffix");
        case AST_ABS_ARR_SUFFIX_STATIC:
            return STR_LIT("abstract array suffix static");
        case AST_ABS_ARR_SUFFIX_ASTERISK:
            return STR_LIT("abstract array suffix asterisk");
        case AST_ABS_FUNC_SUFFIX:
            return STR_LIT("abstract function suffix");
        case AST_INITIALIZER:
            return STR_LIT("initializer");
        case AST_INIT_LIST:
            return STR_LIT("initializer list");
        case AST_DESIGNATION_INIT:
            return STR_LIT("designation initializer");
        case AST_DESIGNATOR_LIST:
            return STR_LIT("designator list");
        case AST_DESIGNATOR:
            return STR_LIT("designator");
        case AST_CONST_EXPR:
            return STR_LIT("constant expression");
        case AST_EXPR:
            return STR_LIT("expression");
        case AST_ASSIGN:
            return STR_LIT("assign expression");
        case AST_ASSIGN_MUL:
            return STR_LIT("multiply assign expression");
        case AST_ASSIGN_DIV:
            return STR_LIT("divide assign expression");
        case AST_ASSIGN_MOD:
            return STR_LIT("modulo assign expression");
        case AST_ASSIGN_ADD:
            return STR_LIT("add assign expression");
        case AST_ASSIGN_SUB:
            return STR_LIT("subtract assign expression");
        case AST_ASSIGN_SHL:
            return STR_LIT("left shift assign expression");
        case AST_ASSIGN_SHR:
            return STR_LIT("right shift assign expression");
        case AST_ASSIGN_AND:
            return STR_LIT("and assign expression");
        case AST_ASSIGN_XOR:
            return STR_LIT("xor assign expression");
        case AST_ASSIGN_OR:
            return STR_LIT("or assign expression");
        case AST_COND_EXPR:
            return STR_LIT("conditional expression");
        case AST_COND_ITEMS:
            return STR_LIT("conditional expression items");
        case AST_LOG_OR_EXPR:
            return STR_LIT("logical or expression");
        case AST_LOG_AND_EXPR:
            return STR_LIT("logical and expression");
        case AST_OR_EXPR:
            return STR_LIT("or expression");
        case AST_XOR_EXPR:
            return STR_LIT("xor expression");
        case AST_AND_EXPR:
            return STR_LIT("and expression");
        case AST_EQ_EXPR:
            return STR_LIT("equals expression");
        case AST_NE_EXPR:
            return STR_LIT("not equals expression");
        case AST_REL_EXPR_LT:
            return STR_LIT("less than relational expression");
        case AST_REL_EXPR_GT:
            return STR_LIT("greater than relational expression");
        case AST_REL_EXPR_LE:
            return STR_LIT("less than or equal relational expression");
        case AST_REL_EXPR_GE:
            return STR_LIT("greater than or equal relational expression");
        case AST_LSHIFT_EXPR:
            return STR_LIT("left shift expression");
        case AST_RSHIFT_EXPR:
            return STR_LIT("right shift expression");
        case AST_ADD_EXPR:
            return STR_LIT("add expression");
        case AST_SUB_EXPR:
            return STR_LIT("subtract expression");
        case AST_MUL_EXPR:
            return STR_LIT("multiply expression");
        case AST_DIV_EXPR:
            return STR_LIT("divide expression");
        case AST_MOD_EXPR:
            return STR_LIT("modulo expression");
        case AST_CAST_EXPR:
            return STR_LIT("cast expression");
        case AST_TYPE_NAME:
            return STR_LIT("type name");
        case AST_SPEC_QUAL_LIST_ATTR:
            return STR_LIT("specifier qualifier list attribute");
        case AST_SPEC_QUAL_LIST:
            return STR_LIT("specifier qualifier list");
        case AST_TYPE_QUAL:
            return STR_LIT("type qualifier");
        case AST_UNARY_EXPR_INC:
            return STR_LIT("increment unary expression");
        case AST_UNARY_EXPR_DEC:
            return STR_LIT("decrement unary expression");
        case AST_UNARY_EXPR_SIZEOF:
            return STR_LIT("sizeof unary expression");
        case AST_UNARY_EXPR_ALIGNOF:
            return STR_LIT("alignof unary expression");
        case AST_UNARY_EXPR_ADDRESSOF:
            return STR_LIT("addressof unary expression");
        case AST_UNARY_EXPR_DEREF:
            return STR_LIT("dereference unary expression");
        case AST_UNARY_EXPR_PLUS:
            return STR_LIT("plus unary expression");
        case AST_UNARY_EXPR_MINUS:
            return STR_LIT("minus unary expression");
        case AST_UNARY_EXPR_BNOT:
            return STR_LIT("binary not unary expression");
        case AST_UNARY_EXPR_NOT:
            return STR_LIT("not unary expression");
        case AST_POSTFIX_EXPR:
            return STR_LIT("postfix expression");
        case AST_POSTFIX_OP_INDEX:
            return STR_LIT("postfix operation index");
        case AST_POSTFIX_OP_CALL:
            return STR_LIT("postfix operation call");
        case AST_POSTFIX_OP_ACCESS:
            return STR_LIT("postfix operation access");
        case AST_POSTFIX_OP_PTR_ACCESS:
            return STR_LIT("postfix operation pointer access");
        case AST_POSTFIX_OP_INC:
            return STR_LIT("postfix operation increment");
        case AST_POSTFIX_OP_DEC:
            return STR_LIT("postfix operation decrement");
        case AST_ARG_EXPR_LIST:
            return STR_LIT("argument expression list");
        case AST_PRIMARY_EXPR:
            return STR_LIT("primary expression");
        case AST_GENERIC_SEL:
            return STR_LIT("generic selection");
        case AST_GENERIC_ASSOC_LIST:
            return STR_LIT("generic association list");
        case AST_GENERIC_ASSOC:
            return STR_LIT("generic association");
        case AST_CONSTANT:
            return STR_LIT("constant");
        case AST_ENUM_CONSTANT:
            return STR_LIT("enum constant");
        case AST_STRING_LITERAL:
            return STR_LIT("string literal");
        case AST_FUNC:
            return STR_LIT("function constant");
        case AST_COMPOUND_LITERAL:
            return STR_LIT("compound literal");
        case AST_BRACED_INITIALIZER:
            return STR_LIT("braced initializer");
        case AST_COMPOUND_LITERAL_TYPE:
            return STR_LIT("compound literal type");
        case AST_STORAGE_CLASS_SPECS:
            return STR_LIT("storage class specifier");
        case AST_IDENTIFIER:
            return STR_LIT("identifier");
    }
    UNREACHABLE();
}
