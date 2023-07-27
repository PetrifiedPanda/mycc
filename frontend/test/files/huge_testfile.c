
#define NULL 0
#define EOF -1

typedef _Bool bool;

typedef unsigned long size_t;
typedef unsigned char uint8_t;
typedef unsigned long long uintmax_t;
typedef long long intmax_t;

enum {true = 1, false = 0};

extern int errno;

enum {
    ERANGE
};

typedef struct FILE FILE; 

const bool EXIT_FAILURE = -1;

extern FILE* stderr;
extern FILE* stdout;

#define CHAR_BIT 8

extern int fprintf(FILE*, const char*, ...);
extern FILE* fopen(const char*, const char*);
extern int fclose(FILE*);
extern void fflush(FILE*);

extern void* malloc(size_t);
extern void* calloc(size_t, size_t);
extern void* realloc(void*, size_t);
extern void free(void*);
extern void abort(void);
extern char* strcpy(char*, const char*);
extern void* memmove(void*, const void*, size_t);
extern void* memcpy(void*, const void*, size_t);
extern long double strtold(const char*, char**);
extern int feof(FILE*);
extern int tolower(int);
extern int printf(const char*, ...);
extern int strncmp(const char*, const char*, size_t);
extern int strcmp(const char*, const char*);
extern int isdigit(int);
extern int isspace(int);
extern int getc(FILE*);
extern _Noreturn void exit(int);

extern size_t strlen(const char*);

void assert(bool x) {
    if (!x) {
        abort();
    }
}

extern int isalpha(int);

typedef struct { char buf[100]; } jmp_buf;

extern _Noreturn void longjmp();
extern int setjmp(jmp_buf buf);

extern unsigned long long strtoull(const char*, char**, int);

struct str {
    union {
        struct {
            size_t _is_static_buf : 1;
            size_t _len : sizeof(size_t) * CHAR_BIT - 1;
            size_t _cap;
            char* _data;
        };
        struct {
            bool _is_static_buf_dup : 1;
            uint8_t _small_len : sizeof(uint8_t) * CHAR_BIT - 1;
            char _static_buf[sizeof(size_t) * 2 - sizeof(uint8_t)
                             + sizeof(char*)];
        };
    };
};

struct str create_null_str(void);
struct str create_empty_str(void);
struct str create_str(size_t len, const char* str);

bool str_is_valid(const struct str* str);
size_t str_len(const struct str* str);

const char* str_get_data(const struct str* str);
char str_char_at(const struct str* str, size_t i);

void str_push_back(struct str* str, char c);
void str_shrink_to_fit(struct str* str);

struct str str_concat(size_t len1, const char* s1, size_t len2, const char* s2);

struct str str_take(struct str* str);
struct str str_copy(const struct str* str);

void free_str(const struct str* str);

struct file_info {
    size_t len;
    struct str* paths;
};

struct file_info create_file_info(const struct str* start_file);

void file_info_add(struct file_info* i, const struct str* path);

void free_file_info(struct file_info* i);

enum token_type {
    IDENTIFIER,
    I_CONSTANT,
    F_CONSTANT,
    STRING_LITERAL,
    TYPEDEF_NAME,

    // Keywords
    KEYWORDS_START,
    FUNC_NAME = KEYWORDS_START, // __func__
    SIZEOF,
    TYPEDEF,
    EXTERN,
    STATIC,
    AUTO,
    REGISTER,
    INLINE,
    BOOL, // _Bool
    CHAR,
    SHORT,
    INT,
    LONG,
    SIGNED,
    UNSIGNED,
    FLOAT,
    DOUBLE,
    VOID,
    COMPLEX,   // _Complex
    IMAGINARY, // _Imaginary
    CONST,
    VOLATILE,
    RESTRICT,
    ATOMIC,
    STRUCT,
    UNION,
    ENUM,
    CASE,
    DEFAULT,
    IF,
    ELSE,
    SWITCH,
    WHILE,
    DO,
    FOR,
    GOTO,
    CONTINUE,
    BREAK,
    RETURN,
    ALIGNAS,       // _Alignas
    ALIGNOF,       // _Alignof
    GENERIC,       // _Generic
    NORETURN,      // _Noreturn
    STATIC_ASSERT, // _Static_assert
    THREAD_LOCAL,  // _Thread_local
    KEYWORDS_END,

    // Punctuation
    SEMICOLON = KEYWORDS_END,
    LBRACKET, // (
    RBRACKET, // )
    LBRACE,   // {
    RBRACE,   // }
    LINDEX,   // [
    RINDEX,   // ]
    DOT,
    AND,  // &
    OR,   // |
    XOR,  // ^
    NOT,  // !
    BNOT, // ~
    SUB,
    ADD,
    ASTERISK,
    DIV,
    MOD,
    LT,     // <
    GT,     // >
    QMARK,  // ?
    COLON,  // :
    ASSIGN, // =
    COMMA,
    PTR_OP, // ->
    INC_OP,
    DEC_OP,
    LEFT_OP,  // <<
    RIGHT_OP, // >>
    LE_OP,
    GE_OP,
    EQ_OP,
    NE_OP,
    AND_OP, // &&
    OR_OP,  // ||
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    LEFT_ASSIGN,
    RIGHT_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    ELLIPSIS, // ...

    // Preprocessor only
    STRINGIFY_OP, // #
    CONCAT_OP,    // ##

    INVALID
};

/**
 * @brief Gets a spelling for the given token_type
 *
 * @param type Type to get the spelling for
 * @return const char* The spelling of the given token type, if it is
 * unambiguous, otherwise NULL
 */
const char* get_spelling(enum token_type type);

/**
 * @brief Gets a string to identify the token_type
 *
 * @param type enum token_type value
 * @return const char* A string that is identical to the spelling of the enum
 * value
 */
const char* get_type_str(enum token_type type);

bool is_unary_op(enum token_type t);
bool is_assign_op(enum token_type t);
bool is_storage_class_spec(enum token_type t);
bool is_keyword_type_spec(enum token_type t);
bool is_type_qual(enum token_type t);
bool is_func_spec(enum token_type t);

bool is_shift_op(enum token_type t);
bool is_rel_op(enum token_type t);
bool is_mul_op(enum token_type t);
bool is_add_op(enum token_type t);
bool is_eq_op(enum token_type t);

enum value_type {
    VALUE_CHAR,
    VALUE_SINT,
    VALUE_INT,
    VALUE_LINT,
    VALUE_LLINT,
    VALUE_UCHAR,
    VALUE_USINT,
    VALUE_UINT,
    VALUE_ULINT,
    VALUE_ULLINT,
    VALUE_FLOAT,
    VALUE_DOUBLE,
    VALUE_LDOUBLE,
};

struct value {
    enum value_type type;
    union {
        intmax_t int_val;
        uintmax_t uint_val;
        long double float_val;
    };
};

bool value_is_uint(enum value_type t);
bool value_is_int(enum value_type t);
bool value_is_float(enum value_type t);

struct value create_int_value(enum value_type t, intmax_t val);
struct value create_uint_value(enum value_type t, uintmax_t val);
struct value create_float_value(enum value_type t, long double val);

const char* get_value_type_str(enum value_type t);

struct file_loc {
    size_t line, index;
};

struct source_loc {
    size_t file_idx;
    struct file_loc file_loc;
};

struct token {
    enum token_type type;
    union {
        struct str spelling;
        struct value val;
    };
    struct source_loc loc;
};

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 *        type have only one spelling
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token(enum token_type type,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx);

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 *        NULL
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token_copy(enum token_type type,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx);

struct str take_spelling(struct token* t);

void free_token(struct token* t);

struct err_base {
    struct source_loc loc;
};

struct err_base create_err_base(struct source_loc loc);

void print_err_base(FILE* out,
                    const struct file_info* file_info,
                    const struct err_base* err);

enum arch {
    ARCH_X86_64,
};

struct arch_int_info {
    size_t sint_size;
    size_t int_size;
    size_t lint_size;
    size_t llint_size;
};

struct arch_float_info {
    size_t float_size;
    size_t double_size;
    size_t ldouble_size;
};

struct arch_type_info {
    struct arch_int_info int_info;
    struct arch_float_info float_info;
};

struct arch_type_info get_arch_type_info(enum arch a, bool is_windows);




enum float_const_err_type {
    FLOAT_CONST_ERR_NONE,
    FLOAT_CONST_ERR_TOO_LARGE,
    FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
    FLOAT_CONST_ERR_INVALID_CHAR,
};

struct float_const_err {
    enum float_const_err_type type;
    char invalid_char;
};

struct parse_float_const_res {
    struct float_const_err err;
    struct value res;
};

struct parse_float_const_res parse_float_const(const char* spell);

void print_float_const_err(FILE* out, const struct float_const_err* err);

enum int_const_err_type {
    INT_CONST_ERR_NONE,
    INT_CONST_ERR_TOO_LARGE,
    INT_CONST_ERR_SUFFIX_TOO_LONG,
    INT_CONST_ERR_CASE_MIXING,
    INT_CONST_ERR_U_BETWEEN_LS,
    INT_CONST_ERR_TRIPLE_LONG,
    INT_CONST_ERR_DOUBLE_U,
    INT_CONST_ERR_INVALID_CHAR,
};

struct int_const_err {
    enum int_const_err_type type;
    char invalid_char;
};

struct parse_int_const_res {
    struct int_const_err err;
    struct value res;
};

struct parse_int_const_res parse_int_const(const char* spell,
                                           const struct arch_int_info* info);

void print_int_const_err(FILE* out, const struct int_const_err* err);

enum char_const_err_type {
    CHAR_CONST_ERR_NONE,
    CHAR_CONST_ERR_EXPECTED_CHAR,
    CHAR_CONST_ERR_INVALID_ESCAPE,
};

struct char_const_err {
    enum char_const_err_type type;
    union {
        struct {
            uint8_t num_expected;
            char expected_chars[4];
            char got_char;
        };
        char invalid_escape;
    };
};

struct parse_char_const_res {
    struct char_const_err err;
    struct value res;
};

struct parse_char_const_res parse_char_const(const char* spell,
                                             const struct arch_int_info* info);

void print_char_const_err(FILE* out, const struct char_const_err* err);






enum preproc_err_type {
    PREPROC_ERR_NONE = 0,
    PREPROC_ERR_FILE_FAIL,
    PREPROC_ERR_UNTERMINATED_LIT,
    PREPROC_ERR_INVALID_ID,
    PREPROC_ERR_INVALID_NUMBER,
    PREPROC_ERR_MACRO_ARG_COUNT,
    PREPROC_ERR_UNTERMINATED_MACRO,
    PREPROC_ERR_ARG_COUNT,
    PREPROC_ERR_NOT_IDENTIFIER,
    PREPROC_ERR_MISSING_IF,
    PREPROC_ERR_INVALID_PREPROC_DIR,
    PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
    PREPROC_ERR_MISPLACED_PREPROC_TOKEN,
    PREPROC_ERR_INT_CONST,
    PREPROC_ERR_FLOAT_CONST,
    PREPROC_ERR_CHAR_CONST,
};

enum else_op_type {
    ELSE_OP_ELIF,
    ELSE_OP_ELSE,
    ELSE_OP_ENDIF,
};

enum single_macro_op_type {
    SINGLE_MACRO_OP_IFDEF,
    SINGLE_MACRO_OP_IFNDEF,
    SINGLE_MACRO_OP_UNDEF,
};

struct preproc_err {
    enum preproc_err_type type;
    struct err_base base;
    union {
        struct {
            bool open_fail;
            size_t fail_file;
        };
        bool is_char_lit;
        struct str invalid_id;
        struct str invalid_num;
        struct {
            size_t expected_arg_count;
            bool too_few_args;
            bool is_variadic;
        };
        struct {
            enum single_macro_op_type count_dir_type;
            bool count_empty;
        };
        struct {
            enum single_macro_op_type not_identifier_op;
            enum token_type not_identifier_got;
        };
        enum else_op_type missing_if_op;
        struct {
            enum else_op_type elif_after_else_op;
            struct source_loc prev_else_loc;
        };
        enum token_type misplaced_preproc_tok;
        struct {
            struct str constant_spell;
            union {
                struct int_const_err int_const_err;
                struct float_const_err float_const_err;
                struct char_const_err char_const_err;
            };
        };
    };
};

struct preproc_err create_preproc_err(void);

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc loc);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err);

void free_preproc_err(struct preproc_err* err);


struct preproc_res {
    struct token* toks;
    struct file_info file_info;
};

/**
 * @param path path to file
 *
 * @return preprocessed tokens from this file, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct preproc_res preproc(const char* path, struct preproc_err* err);

/**
 * @param str a string containing source code
 * @param path path to be entered into the resulting tokens
 *
 * @return preprocessed tokens from this string, or NULL if an error occurred
 *         note that these tokens still need to be converted
 */
struct preproc_res preproc_string(const char* str,
                                  const char* path,
                                  struct preproc_err* err);

/**
 * Converts the given preprocessor tokens to parser tokens
 */
bool convert_preproc_tokens(struct token* tokens, const struct arch_int_info* info, struct preproc_err* err);

/**
 * Frees tokens before calling convert_preproc_tokens
 */
void free_preproc_res_preproc_tokens(struct preproc_res* res);

/**
 * Frees tokens after calling convert_preproc_tokens
 */
void free_preproc_res(struct preproc_res* res);

enum parser_err_type {
    PARSER_ERR_NONE = 0,
    PARSER_ERR_EXPECTED_TOKENS,
    PARSER_ERR_REDEFINED_SYMBOL,
    PARSER_ERR_ARR_DOUBLE_STATIC,
    PARSER_ERR_ARR_STATIC_NO_LEN,
    PARSER_ERR_ARR_STATIC_ASTERISK,
    PARSER_ERR_TYPEDEF_INIT,
    PARSER_ERR_TYPEDEF_FUNC_DEF,
    PARSER_ERR_TYPEDEF_PARAM_DECL,
    PARSER_ERR_TYPEDEF_STRUCT,
    PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
    PARSER_ERR_INCOMPATIBLE_TYPE_SPECS,
    PARSER_ERR_DISALLOWED_TYPE_QUALS,
    PARSER_ERR_EXPECTED_TYPEDEF_NAME,
};

struct parser_err {
    enum parser_err_type type;
    struct err_base base;
    union {
        struct { // expected tokens
            enum token_type got;
            size_t num_expected;
            enum token_type* expected;
        };
        struct { // redefined symbol
            struct str redefined_symbol;
            bool was_typedef_name;
            size_t prev_def_file;
            struct file_loc prev_def_loc;
        };
        struct { // incompatible type specs
            enum token_type type_spec, prev_type_spec;
        };
        // disallowed type specs
        enum token_type incompatible_type;
        struct str non_typedef_spelling;
    };
};

struct parser_err create_parser_err(void);

void set_parser_err(struct parser_err* err,
                    enum parser_err_type type,
                    struct source_loc loc);

void print_parser_err(FILE* out,
                      const struct file_info* file_info,
                      const struct parser_err* err);

void free_parser_err(struct parser_err* err);

struct string_hash_map_key;

struct string_hash_map {
    size_t _len;
    size_t _cap;
    size_t _item_size;
    bool _free_keys;
    void (*_item_free)(void*);
    struct string_hash_map_key* _keys;
    void* _items;
};

struct string_hash_map create_string_hash_map(size_t elem_size,
                                              size_t init_cap,
                                              bool free_keys,
                                              void (*item_free)(void*));
void free_string_hash_map(struct string_hash_map* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* string_hash_map_insert(struct string_hash_map* map,
                                   const struct str* key,
                                   const void* item);

/**
 * @brief Inserts item and key into this map, overwriting the key
 *        if it is already present
 *
 * @return true if an existing entry was overwritten
 */
bool string_hash_map_insert_overwrite(struct string_hash_map* map,
                                      const struct str* key,
                                      const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* string_hash_map_get(const struct string_hash_map* map,
                                const struct str* key);

void string_hash_map_remove(struct string_hash_map* map, const struct str* key);

struct parser_state {
    struct token* it;
    size_t _len;
    struct string_hash_map* _scope_maps;
    struct parser_err* err;
};

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err);
void free_parser_state(struct parser_state* s);

bool accept(struct parser_state* s, enum token_type expected);
void accept_it(struct parser_state* s);

void parser_push_scope(struct parser_state* s);
void parser_pop_scope(struct parser_state* s);

bool register_enum_constant(struct parser_state* s, const struct token* token);
bool register_typedef_name(struct parser_state* s, const struct token* token);

bool is_enum_constant(const struct parser_state* s, const struct str* spell);
bool is_typedef_name(const struct parser_state* s, const struct str* spell);

struct external_declaration;

struct translation_unit {
    size_t len;
    struct external_declaration* external_decls;
};

struct translation_unit parse_translation_unit(struct parser_state* s);

void free_translation_unit(struct translation_unit* u);

struct declaration;

struct declaration_list {
    size_t len;
    struct declaration* decls;
};

struct declaration_list parse_declaration_list(struct parser_state* s);

void free_declaration_list(struct declaration_list* l);

struct init_declarator;

struct init_declarator_list {
    size_t len;
    struct init_declarator* decls;
};

/**
 *
 * @param s The current state
 * @param first_decl A heap allocated init declarator
 * @return struct init_declarator_list A list parsed with first_decl as the
 * first element in the list
 */
struct init_declarator_list parse_init_declarator_list_first(
    struct parser_state* s,
    struct init_declarator* first_decl);

struct init_declarator_list parse_init_declarator_list(struct parser_state* s);

struct init_declarator_list parse_init_declarator_list_typedef_first(
    struct parser_state* s,
    struct init_declarator* first_decl);

struct init_declarator_list parse_init_declarator_list_typedef(
    struct parser_state* s);

void free_init_declarator_list(struct init_declarator_list* l);

struct declarator;
struct initializer;

struct init_declarator {
    struct declarator* decl;
    struct initializer* init;
};

bool parse_init_declarator_typedef_inplace(struct parser_state* s,
                                           struct init_declarator* res);
bool parse_init_declarator_inplace(struct parser_state* s,
                                   struct init_declarator* res);

void free_init_declarator_children(struct init_declarator* d);

struct pointer;
struct direct_declarator;

struct declarator {
    struct pointer* ptr;
    struct direct_declarator* direct_decl;
};

struct declarator* parse_declarator(struct parser_state* s);
struct declarator* parse_declarator_typedef(struct parser_state* s);

void free_declarator(struct declarator* d);

struct type_quals {
    bool is_const;
    bool is_restrict;
    bool is_volatile;
    bool is_atomic;
};

struct type_quals create_type_quals(void);

void update_type_quals(struct parser_state* s, struct type_quals* quals);

struct type_quals parse_type_qual_list(struct parser_state* s);

bool is_valid_type_quals(const struct type_quals* q);

struct ast_node_info {
    struct source_loc loc; 
};

struct ast_node_info create_ast_node_info(struct source_loc loc);




struct pointer {
    struct ast_node_info info;
    size_t num_indirs;
    struct type_quals* quals_after_ptr;
};

struct pointer* parse_pointer(struct parser_state* s);

void free_pointer(struct pointer* p);

struct param_list;

struct param_type_list {
    bool is_variadic;
    struct param_list* param_list;
};

struct param_type_list parse_param_type_list(struct parser_state* s);

void free_param_type_list(struct param_type_list* l);

struct param_declaration;

struct param_list {
    size_t len;
    struct param_declaration* decls;
};

struct param_list* parse_param_list(struct parser_state* s);

void free_param_list(struct param_list* l);

struct declaration_specs;
struct declarator;
struct abs_declarator;

enum param_decl_type {
    PARAM_DECL_DECL,
    PARAM_DECL_ABSTRACT_DECL,
    PARAM_DECL_NONE
};

struct param_declaration {
    struct declaration_specs* decl_specs;
    enum param_decl_type type;
    union {
        struct declarator* decl;
        struct abs_declarator* abstract_decl;
    };
};

bool parse_param_declaration_inplace(struct parser_state* s,
                                     struct param_declaration* res);

void free_param_declaration_children(struct param_declaration* d);

struct atomic_type_spec;
struct struct_union_spec;
struct enum_spec;
struct identifier;

enum type_spec_type {
    TYPE_SPEC_NONE,
    TYPE_SPEC_VOID,
    TYPE_SPEC_CHAR,
    TYPE_SPEC_INT,
    TYPE_SPEC_FLOAT,
    TYPE_SPEC_DOUBLE,
    TYPE_SPEC_BOOL,
    TYPE_SPEC_ATOMIC,
    TYPE_SPEC_STRUCT,
    TYPE_SPEC_ENUM,
    TYPE_SPEC_TYPENAME
};

struct type_modifiers {
    bool is_unsigned;
    bool is_signed;
    bool is_short;
    int num_long;
    bool is_complex;
    bool is_imaginary;
};

struct type_specs {
    struct type_modifiers mods;
    enum type_spec_type type;
    union {
        struct atomic_type_spec* atomic_spec;
        struct struct_union_spec* struct_union_spec;
        struct enum_spec* enum_spec;
        struct identifier* typedef_name;
    };
};

struct type_specs create_type_specs(void);

bool update_type_specs(struct parser_state* s, struct type_specs* q);

void free_type_specs_children(struct type_specs* s);

bool is_valid_type_specs(const struct type_specs* s);

struct type_name;

struct atomic_type_spec {
    struct ast_node_info info;
    struct type_name* type_name;
};

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s);

void free_atomic_type_spec(struct atomic_type_spec* s);

struct spec_qual_list;

struct abs_declarator;

struct type_name {
    struct spec_qual_list* spec_qual_list;
    struct abs_declarator* abstract_decl;
};

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res);
struct type_name* parse_type_name(struct parser_state* s);

void free_type_name_children(struct type_name* n);
void free_type_name(struct type_name* n);

struct spec_qual_list {
    struct ast_node_info info;
    struct type_quals quals;
    struct type_specs specs;
};

struct spec_qual_list parse_spec_qual_list(struct parser_state* s);

void free_spec_qual_list_children(struct spec_qual_list* l);
void free_spec_qual_list(struct spec_qual_list* l);

bool is_valid_spec_qual_list(struct spec_qual_list* l);

struct pointer;
struct direct_abs_declarator;

struct abs_declarator {
    struct pointer* ptr;
    struct direct_abs_declarator* direct_abs_decl;
};

struct abs_declarator* parse_abs_declarator(struct parser_state* s);

void free_abs_declarator(struct abs_declarator* d);

struct abs_declarator;
struct assign_expr;

enum abs_arr_or_func_suffix_type {
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY, // either [] or [*]
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN,
    ABS_ARR_OR_FUNC_SUFFIX_FUNC
};

struct abs_arr_or_func_suffix {
    struct ast_node_info info;
    enum abs_arr_or_func_suffix_type type;
    union {
        bool has_asterisk;
        struct {
            bool is_static; // only if assign != NULL
            struct type_quals type_quals;
            struct assign_expr* assign;
        };
        struct param_type_list func_types;
    };
};

struct direct_abs_declarator {
    struct ast_node_info info;
    struct abs_declarator* bracket_decl;

    size_t len;
    struct abs_arr_or_func_suffix* following_suffixes;
};

struct direct_abs_declarator* parse_direct_abs_declarator(
    struct parser_state* s);

void free_direct_abs_declarator(struct direct_abs_declarator* d);

struct unary_expr;
struct cond_expr;

enum assign_expr_op {
    ASSIGN_EXPR_ASSIGN,
    ASSIGN_EXPR_MUL,
    ASSIGN_EXPR_DIV,
    ASSIGN_EXPR_MOD,
    ASSIGN_EXPR_ADD,
    ASSIGN_EXPR_SUB,
    ASSIGN_EXPR_LSHIFT,
    ASSIGN_EXPR_RSHIFT,
    ASSIGN_EXPR_AND,
    ASSIGN_EXPR_XOR,
    ASSIGN_EXPR_OR,
};

struct unary_and_op {
    struct unary_expr* unary;
    enum assign_expr_op op;
};

struct assign_expr {
    size_t len;
    struct unary_and_op* assign_chain;
    struct cond_expr* value;
};

bool parse_assign_expr_inplace(struct parser_state* s, struct assign_expr* res);
struct assign_expr* parse_assign_expr(struct parser_state* s);

void free_assign_expr_children(struct assign_expr* e);
void free_assign_expr(struct assign_expr* e);

struct log_or_expr;
struct expr;

struct log_or_and_expr {
    struct log_or_expr* log_or;
    struct expr* expr;
};

struct cond_expr {
    size_t len;
    struct log_or_and_expr* conditionals;
    struct log_or_expr* last_else;
};

bool parse_cond_expr_inplace(struct parser_state* s, struct cond_expr* res);

struct cast_expr;

struct cond_expr* parse_cond_expr_cast(struct parser_state* s,
                                       struct cast_expr* start);

void free_cond_expr_children(struct cond_expr* e);
void free_cond_expr(struct cond_expr* e);

struct assign_expr;

struct expr {
    size_t len;
    struct assign_expr* assign_exprs;
};

bool parse_expr_inplace(struct parser_state* s, struct expr* res);

struct expr* parse_expr(struct parser_state* s);

void free_expr_children(struct expr* expr);

void free_expr(struct expr* expr);

struct log_and_expr;

struct log_or_expr {
    size_t len;
    struct log_and_expr* log_ands;
};

struct log_or_expr* parse_log_or_expr(struct parser_state* s);

struct cast_expr;

struct log_or_expr* parse_log_or_expr_cast(struct parser_state* s,
                                           struct cast_expr* start);

void free_log_or_expr(struct log_or_expr* e);

struct or_expr;

struct log_and_expr {
    size_t len;
    struct or_expr* or_exprs;
};

bool parse_log_and_expr_inplace(struct parser_state* s,
                                struct log_and_expr* res);

struct cast_expr;

struct log_and_expr* parse_log_and_expr_cast(struct parser_state* s,
                                             struct cast_expr* start);

void free_log_and_expr_children(struct log_and_expr* e);

struct xor_expr;

struct or_expr {
    size_t len;
    struct xor_expr* xor_exprs;
};

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res);

struct cast_expr;

struct or_expr* parse_or_expr_cast(struct parser_state* s,
                                   struct cast_expr* start);

void free_or_expr_children(struct or_expr* e);

struct and_expr;

struct xor_expr {
    size_t len;
    struct and_expr* and_exprs;
};

bool parse_xor_expr_inplace(struct parser_state* s, struct xor_expr* res);

struct cast_expr;

struct xor_expr* parse_xor_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_xor_expr_children(struct xor_expr* e);

struct eq_expr;

struct and_expr {
    size_t len;
    struct eq_expr* eq_exprs;
};

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res);

struct cast_expr;

struct and_expr* parse_and_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_and_expr_children(struct and_expr* e);

struct rel_expr;

enum eq_expr_op {
    EQ_EXPR_EQ,
    EQ_EXPR_NE,
};

struct rel_expr_and_op {
    enum eq_expr_op op;
    struct rel_expr* rhs;
};

struct eq_expr {
    struct rel_expr* lhs;
    size_t len;
    struct rel_expr_and_op* eq_chain;
};

bool parse_eq_expr_inplace(struct parser_state* s, struct eq_expr* res);

struct cast_expr;

struct eq_expr* parse_eq_expr_cast(struct parser_state* s,
                                   struct cast_expr* start);

void free_eq_expr_children(struct eq_expr* e);

struct shift_expr;

enum rel_expr_op {
    REL_EXPR_LT,
    REL_EXPR_GT,
    REL_EXPR_LE,
    REL_EXPR_GE,
};

struct shift_expr_and_op {
    enum rel_expr_op op;
    struct shift_expr* rhs;
};

struct rel_expr {
    struct shift_expr* lhs;
    size_t len;
    struct shift_expr_and_op* rel_chain;
};

struct rel_expr* parse_rel_expr(struct parser_state* s);

struct cast_expr;

struct rel_expr* parse_rel_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_rel_expr_children(struct rel_expr* e);
void free_rel_expr(struct rel_expr* e);

struct add_expr;

enum shift_expr_op {
    SHIFT_EXPR_LEFT,
    SHIFT_EXPR_RIGHT,
};

struct add_expr_and_op {
    enum shift_expr_op op;
    struct add_expr* rhs;
};

struct shift_expr {
    struct add_expr* lhs;
    size_t len;
    struct add_expr_and_op* shift_chain;
};

struct shift_expr* parse_shift_expr(struct parser_state* s);

struct cast_expr;

struct shift_expr* parse_shift_expr_cast(struct parser_state* s,
                                         struct cast_expr* start);

void free_shift_expr(struct shift_expr* e);

struct mul_expr;

enum add_expr_op {
    ADD_EXPR_ADD,
    ADD_EXPR_SUB,
};

struct mul_expr_and_op {
    enum add_expr_op op;
    struct mul_expr* rhs;
};

struct add_expr {
    struct mul_expr* lhs;
    size_t len;
    struct mul_expr_and_op* add_chain;
};

struct add_expr* parse_add_expr(struct parser_state* s);

struct cast_expr;

struct add_expr* parse_add_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_add_expr(struct add_expr* e);

struct cast_expr;

enum mul_expr_op {
    MUL_EXPR_MUL,
    MUL_EXPR_DIV,
    MUL_EXPR_MOD,
};

struct cast_expr_and_op {
    enum mul_expr_op op;
    struct cast_expr* rhs;
};

struct mul_expr {
    struct cast_expr* lhs;
    size_t len;
    struct cast_expr_and_op* mul_chain;
};

struct mul_expr* parse_mul_expr(struct parser_state* s);

struct cast_expr;

struct mul_expr* parse_mul_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_mul_expr(struct mul_expr* e);

struct unary_expr;
struct type_name;

struct cast_expr {
    struct ast_node_info info;
    size_t len;
    struct type_name* type_names;
    struct unary_expr* rhs;
};

struct cast_expr* parse_cast_expr(struct parser_state* s);
struct cast_expr* parse_cast_expr_type_name(struct parser_state* s,
                                            struct type_name* type_name,
                                            struct source_loc start_bracket_loc);

struct cast_expr* create_cast_expr_unary(struct unary_expr* start);

void free_cast_expr(struct cast_expr* e);

struct postfix_expr;
struct cast_expr;
struct type_name;

enum unary_expr_type {
    UNARY_POSTFIX,
    UNARY_ADDRESSOF,
    UNARY_DEREF,
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_BNOT,
    UNARY_NOT,
    UNARY_SIZEOF_TYPE,
    UNARY_ALIGNOF,
};

enum unary_expr_op {
    UNARY_OP_INC,
    UNARY_OP_DEC,
    UNARY_OP_SIZEOF,
};

struct unary_expr {
    struct ast_node_info info;
    size_t len;
    enum unary_expr_op* ops_before; // only SIZEOF INC_OP or DEC_OP
    enum unary_expr_type type;
    union {
        struct postfix_expr* postfix;
        struct cast_expr* cast_expr;
        struct type_name* type_name;
    };
};

struct unary_expr* parse_unary_expr(struct parser_state* s);

/**
 *
 * @param s current state
 * @param ops_before array of len tokens
 * @param len length of ops_before
 * @param type_name the already parsed type_name, with which this starts
 * @param start_bracket_loc location of the bracket before the type name
 *
 * @return struct unary_expr* unary expression created with the given parameters
 *         NULL on fail This does not free any of the parameters
 */
struct unary_expr* parse_unary_expr_type_name(struct parser_state* s,
                                              enum unary_expr_op* ops_before,
                                              size_t len,
                                              struct type_name* type_name,
                                              struct source_loc start_bracket_loc);

void free_unary_expr_children(struct unary_expr* u);
void free_unary_expr(struct unary_expr* u);

struct initializer;
struct designation;

struct designation_init {
    struct designation* designation;
    struct initializer* init;
};

struct assign_expr;

struct arg_expr_list {
    size_t len;
    struct assign_expr* assign_exprs;
};

struct arg_expr_list parse_arg_expr_list(struct parser_state* s);

void free_arg_expr_list(struct arg_expr_list* l);




struct init_list {
    size_t len;
    struct designation_init* inits;
};

struct init_list parse_init_list(struct parser_state* s);

void free_init_list_children(struct init_list* l);

struct assign_expr;

struct initializer {
    struct ast_node_info info;
    bool is_assign;
    union {
        struct assign_expr* assign;
        struct init_list init_list;
    };
};

struct initializer* parse_initializer(struct parser_state* s);

void free_initializer_children(struct initializer* i);
void free_initializer(struct initializer* i);

struct designator;

struct designator_list {
    size_t len;
    struct designator* designators;
};

struct designator_list parse_designator_list(struct parser_state* s);

void free_designator_list(struct designator_list* l);

struct const_expr;
struct identifier;

struct designator {
    struct ast_node_info info;
    bool is_index;
    union {
        struct const_expr* arr_index;
        struct identifier* identifier;
    };
};

bool parse_designator_inplace(struct parser_state* s, struct designator* res);

void free_designator_children(struct designator* d);

struct const_expr {
    struct cond_expr expr;
};

struct const_expr* parse_const_expr(struct parser_state* s);

void free_const_expr(struct const_expr* e);

struct designation {
    struct designator_list designators;
};

struct designation* parse_designation(struct parser_state* s);

void free_designation(struct designation* d);



struct primary_expr;
struct expr;
struct identifier;

enum postfix_suffix_type {
    POSTFIX_INDEX,
    POSTFIX_BRACKET,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC_DEC
};

struct postfix_suffix {
    enum postfix_suffix_type type;
    union {
        struct expr* index_expr;
        struct arg_expr_list bracket_list;
        struct identifier* identifier;
        bool is_inc;
    };
};

struct postfix_expr {
    bool is_primary;
    union {
        struct primary_expr* primary;
        struct {
            struct ast_node_info info;
            struct type_name* type_name;
            struct init_list init_list;
        };
    };
    size_t len;
    struct postfix_suffix* suffixes;
};

struct postfix_expr* parse_postfix_expr(struct parser_state* s);

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @param start_bracket_loc Location of the bracket starting this expr
 *
 * @return A postfix_expr that uses the given type_name
 */
struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s,
                                                  struct type_name* type_name,
                                                  struct source_loc start_bracket_loc);

void free_postfix_expr(struct postfix_expr* p);

enum constant_type {
    CONSTANT_ENUM,
    CONSTANT_INT,
    CONSTANT_FLOAT,
};

struct constant {
    struct ast_node_info info;
    enum constant_type type;
    union {
        struct str spelling;
        struct value val;
    };
};

struct constant create_constant(struct value val, struct source_loc loc);

struct constant create_enum_constant(const struct str* spelling,
                                     struct source_loc loc);

void free_constant(struct constant* c);

struct string_literal {
    struct ast_node_info info;
    struct str spelling;
};

struct string_literal create_string_literal(const struct str* spelling,
                                            struct source_loc loc);

void free_string_literal(struct string_literal* l);

struct string_constant {
    bool is_func;
    union {
        struct string_literal lit;
        struct ast_node_info info;
    };
};

struct string_constant create_string_constant(const struct str* spelling, struct source_loc loc);

struct string_constant create_func_name(struct source_loc loc);

void free_string_constant(struct string_constant* c);

struct expr;
struct identifier;
struct generic_sel;

enum primary_expr_type {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET,
    PRIMARY_EXPR_GENERIC
};

struct primary_expr {
    enum primary_expr_type type;
    union {
        struct constant constant;
        struct string_constant string;
        struct identifier* identifier;
        struct {
            struct ast_node_info info;
            struct expr* bracket_expr;
        };
        struct generic_sel* generic;
    };
};

struct primary_expr* parse_primary_expr(struct parser_state* s);

void free_primary_expr(struct primary_expr* bracket_expr);

struct generic_assoc;

struct generic_assoc_list {
    struct ast_node_info info;
    size_t len;
    struct generic_assoc* assocs;
};

struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s);

void free_generic_assoc_list(struct generic_assoc_list* l);

struct type_name;
struct assign_expr;

struct generic_assoc {
    struct ast_node_info info;
    struct type_name* type_name; // if NULL this is the default case
    struct assign_expr* assign;
};

bool parse_generic_assoc_inplace(struct parser_state* s,
                                 struct generic_assoc* res);

void free_generic_assoc_children(struct generic_assoc* a);


struct assign_expr;

struct generic_sel {
    struct ast_node_info info;
    struct assign_expr* assign;
    struct generic_assoc_list assocs;
};

struct generic_sel* parse_generic_sel(struct parser_state* s);

void free_generic_sel(struct generic_sel* s);

struct struct_declaration;

struct struct_declaration_list {
    size_t len;
    struct struct_declaration* decls;
};

struct struct_declaration_list parse_struct_declaration_list(
    struct parser_state* s);

void free_struct_declaration_list(struct struct_declaration_list* l);

struct struct_declarator;

struct struct_declarator_list {
    size_t len;
    struct struct_declarator* decls;
};

struct struct_declarator_list parse_struct_declarator_list(
    struct parser_state* s);

void free_struct_declarator_list(struct struct_declarator_list* l);

struct declarator;
struct const_expr;

struct struct_declarator {
    struct declarator* decl;
    struct const_expr* bit_field;
};

bool parse_struct_declarator_inplace(struct parser_state* s,
                                     struct struct_declarator* res);

void free_struct_declarator_children(struct struct_declarator* d);






struct declaration_specs;
struct static_assert_declaration;

struct struct_declaration {
    bool is_static_assert;
    union {
        struct {
            struct declaration_specs* decl_specs;
            struct struct_declarator_list decls;
        };
        struct static_assert_declaration* assert;
    };
};

bool parse_struct_declaration_inplace(struct parser_state* s,
                                      struct struct_declaration* res);

void free_struct_declaration_children(struct struct_declaration* d);

struct align_spec;

struct storage_class {
    bool is_typedef;
    bool is_extern;
    bool is_static;
    bool is_thread_local;
    bool is_auto;
    bool is_register;
};

struct func_specs {
    bool is_inline;
    bool is_noreturn;
};

struct declaration_specs {
    struct ast_node_info info;
    struct func_specs func_specs;
    struct storage_class storage_class;
    struct type_quals type_quals;
    

    size_t num_align_specs;
    struct align_spec* align_specs;
    
    struct type_specs type_specs;
};

struct declaration_specs* parse_declaration_specs(struct parser_state* s,
                                                  bool* found_typedef);

void free_declaration_specs(struct declaration_specs* s);

struct type_name;
struct const_expr;

struct align_spec {
    struct ast_node_info info;
    bool is_type_name;
    union {
        struct type_name* type_name;
        struct const_expr* const_expr;
    };
};

bool parse_align_spec_inplace(struct parser_state* s, struct align_spec* res);

void free_align_spec_children(struct align_spec* s);

struct const_expr;

struct static_assert_declaration {
    struct const_expr* const_expr;
    struct string_literal err_msg;
};

struct static_assert_declaration* parse_static_assert_declaration(
    struct parser_state* s);

void free_static_assert_declaration(struct static_assert_declaration* d);

struct identifier;

struct struct_union_spec {
    struct ast_node_info info;
    bool is_struct;
    struct identifier* identifier;
    struct struct_declaration_list decl_list;
};

struct struct_union_spec* parse_struct_union_spec(struct parser_state* s);

void free_struct_union_spec(struct struct_union_spec* s);

struct enumerator;

struct enum_list {
    size_t len;
    struct enumerator* enums;
};

struct enum_list parse_enum_list(struct parser_state* s);

void free_enum_list(struct enum_list* l);

struct identifier;
struct const_expr;

struct enumerator {
    struct identifier* identifier;
    struct const_expr* enum_val;
};

bool parse_enumerator_inplace(struct parser_state* s, struct enumerator* res);

void free_enumerator_children(struct enumerator* e);



struct identifier;

struct enum_spec {
    struct ast_node_info info;
    struct identifier* identifier;
    struct enum_list enum_list;
};

struct enum_spec* parse_enum_spec(struct parser_state* s);

void free_enum_spec(struct enum_spec* s);

struct identifier {
    struct ast_node_info info;
    struct str spelling;
};

void init_identifier(struct identifier* res,
                     const struct str* spelling,
                     struct source_loc loc);
struct identifier* create_identifier(const struct str* spelling, struct source_loc loc);

void free_identifier_children(struct identifier* i);

void free_identifier(struct identifier* i);

struct identifier;

struct identifier_list {
    size_t len;
    struct identifier* identifiers;
};

struct identifier_list parse_identifier_list(struct parser_state* s);

void free_identifier_list(struct identifier_list* l);

struct assign_expr;
struct const_expr;
struct declarator;
struct identifier;

enum arr_or_func_suffix_type {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_PARAMS,
    ARR_OR_FUNC_FUN_OLD_PARAMS,
    ARR_OR_FUNC_FUN_EMPTY
};

struct arr_suffix {
    bool is_static;
    struct type_quals type_quals;
    bool is_asterisk; // if this is true arr_len should be NULL
    struct assign_expr* arr_len;
};

struct arr_or_func_suffix {
    struct ast_node_info info;
    enum arr_or_func_suffix_type type;
    union {
        struct arr_suffix arr_suffix;
        struct param_type_list fun_types;
        struct identifier_list fun_params;
    };
};

struct direct_declarator {
    struct ast_node_info info;
    bool is_id;
    union {
        struct identifier* id;
        struct declarator* decl;
    };
    size_t len;
    struct arr_or_func_suffix* suffixes;
};

struct direct_declarator* parse_direct_declarator(struct parser_state* s);
struct direct_declarator* parse_direct_declarator_typedef(
    struct parser_state* s);

void free_direct_declarator(struct direct_declarator* d);

struct declaration_specs;
struct static_assert_declaration;

struct declaration {
    bool is_normal_decl;
    union {
        struct {
            struct declaration_specs* decl_specs;
            struct init_declarator_list init_decls;
        };
        struct static_assert_declaration* static_assert_decl;
    };
};

bool parse_declaration_inplace(struct parser_state* s, struct declaration* res);

struct declaration* parse_declaration(struct parser_state* s);

void free_declaration_children(struct declaration* d);
void free_declaration(struct declaration* d);

struct declaration_specs;
struct declarator;
struct compound_statement;

struct func_def {
    struct declaration_specs* specs;
    struct declarator* decl;
    struct declaration_list decl_list;
    struct compound_statement* comp;
};

void free_func_def_children(struct func_def* d);

struct block_item;

struct compound_statement {
    struct ast_node_info info;
    size_t len;
    struct block_item* items;
};

struct compound_statement* parse_compound_statement(struct parser_state* s);

void free_compound_statement(struct compound_statement* s);

struct labeled_statement;
struct compound_statement;
struct expr_statement;
struct selection_statement;
struct iteration_statement;
struct jump_statement;

enum statement_type {
    STATEMENT_LABELED,
    STATEMENT_COMPOUND,
    STATEMENT_EXPRESSION,
    STATEMENT_SELECTION,
    STATEMENT_ITERATION,
    STATEMENT_JUMP
};

struct statement {
    enum statement_type type;
    union {
        struct labeled_statement* labeled;
        struct compound_statement* comp;
        struct expr_statement* expr;
        struct selection_statement* sel;
        struct iteration_statement* it;
        struct jump_statement* jmp;
    };
};

bool parse_statement_inplace(struct parser_state* s, struct statement* res);

struct statement* parse_statement(struct parser_state* s);

void free_statement_children(struct statement* s);

void free_statement(struct statement* s);

struct const_expr;
struct statement;
struct identifier;

enum labeled_statement_type {
    LABELED_STATEMENT_CASE,
    LABELED_STATEMENT_LABEL,
    LABELED_STATEMENT_DEFAULT,
};

struct labeled_statement {
    struct ast_node_info info;
    enum labeled_statement_type type;
    union {
        struct identifier* label;
        struct const_expr* case_expr;
    };
    struct statement* stat;
};

struct labeled_statement* parse_labeled_statement(struct parser_state* s);

void free_labeled_statement(struct labeled_statement* s);

struct expr_statement {
    struct ast_node_info info;
    struct expr expr;
};

struct expr_statement* parse_expr_statement(struct parser_state* s);

void free_expr_statement(struct expr_statement* s);

struct expr;
struct statement;

struct selection_statement {
    struct ast_node_info info;
    bool is_if;
    struct expr* sel_expr;
    struct statement* sel_stat;
    struct statement* else_stat;
};

struct selection_statement* parse_selection_statement(struct parser_state* s);

void free_selection_statement(struct selection_statement* s);

struct declaration;
struct expr_statement;
struct expr;
struct statement;

struct for_loop {
    bool is_decl;
    union {
        struct declaration* init_decl;
        struct expr_statement* init_expr;
    };
    struct expr_statement* cond;
    struct expr* incr_expr;
};

enum iteration_statement_type {
    ITERATION_STATEMENT_WHILE,
    ITERATION_STATEMENT_DO,
    ITERATION_STATEMENT_FOR,
};

struct iteration_statement {
    struct ast_node_info info;
    enum iteration_statement_type type;
    struct statement* loop_body;
    union {
        struct expr* while_cond;
        struct for_loop for_loop;
    };
};

struct iteration_statement* parse_iteration_statement(struct parser_state* s);

void free_iteration_statement(struct iteration_statement* s);

struct expr;
struct identifier;

enum jump_statement_type {
    JUMP_STATEMENT_GOTO,
    JUMP_STATEMENT_CONTINUE,
    JUMP_STATEMENT_BREAK,
    JUMP_STATEMENT_RETURN,
};

struct jump_statement {
    struct ast_node_info info;
    enum jump_statement_type type;
    union {
        struct identifier* goto_label;
        struct expr* ret_val;
    };
};

struct jump_statement* parse_jump_statement(struct parser_state* s);

void free_jump_statement(struct jump_statement* s);


struct block_item {
    bool is_decl;
    union {
        struct declaration decl;
        struct statement stat;
    };
};

bool parse_block_item_inplace(struct parser_state* s, struct block_item* res);

void free_block_item_children(struct block_item* i);

struct external_declaration {
    bool is_func_def;
    union {
        struct func_def func_def;
        struct declaration decl;
    };
};

bool parse_external_declaration_inplace(struct parser_state* s,
                                        struct external_declaration* res);

void free_external_declaration_children(struct external_declaration* d);

struct translation_unit parse_tokens(struct token* tokens,
                                     struct parser_err* err);

bool dump_ast(const struct translation_unit* tl, const struct file_info* file_info, FILE* f);

static bool is_file_sep(char c) {
    switch (c) {
        case '/':
            return true;
        default:
            return false;
    }
}

static const char* strip_file_location(const char* filename) {
    const char* res = filename;

    const char* it = filename;
    while (*it != '\0') {
        if (is_file_sep(*it)) {
            res = it + 1;
        }
        ++it;
    }
    return res;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return EXIT_FAILURE;
    }

    const bool is_windows = false;

    const struct arch_type_info type_info = get_arch_type_info(ARCH_X86_64,
                                                               is_windows);

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];

        struct preproc_err preproc_err = create_preproc_err();
        struct preproc_res preproc_res = preproc(filename, &preproc_err);
        if (preproc_err.type != PREPROC_ERR_NONE) {
            print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
            free_preproc_err(&preproc_err);
            return EXIT_FAILURE;
        }
        if (!convert_preproc_tokens(preproc_res.toks,
                                    &type_info.int_info,
                                    &preproc_err)) {
            print_preproc_err(stderr, &preproc_res.file_info, &preproc_err);
            free_preproc_err(&preproc_err);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        struct parser_err parser_err = create_parser_err();
        struct translation_unit tl = parse_tokens(preproc_res.toks,
                                                  &parser_err);
        if (parser_err.type != PARSER_ERR_NONE) {
            print_parser_err(stderr, &preproc_res.file_info, &parser_err);
            free_parser_err(&parser_err);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        const char* filename_only = strip_file_location(filename);

        const char suffix_str[] = ".ast";
        const struct str out_filename_str = str_concat(strlen(filename_only),
                                                       filename_only,
                                                       sizeof suffix_str
                                                           / sizeof *suffix_str,
                                                       suffix_str);
        const char* out_filename = str_get_data(&out_filename_str);
        FILE* outfile = fopen(out_filename, "w");
        if (!outfile) {
            fprintf(stderr, "Failed to open output file %s\n", out_filename);
            free_str(&out_filename_str);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        if (!dump_ast(&tl, &preproc_res.file_info, outfile)) {
            fprintf(stderr, "Failed to write ast to file %s\n", out_filename);

            if (fclose(outfile) != 0) {
                fprintf(stderr,
                        "Failed to close output file %s\n",
                        out_filename);
            }

            free_str(&out_filename_str);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }

        fflush(outfile);
        if (fclose(outfile) != 0) {
            fprintf(stderr, "Failed to close output file %s\n", out_filename);
            free_str(&out_filename_str);
            free_translation_unit(&tl);
            free_preproc_res(&preproc_res);
            return EXIT_FAILURE;
        }
        free_str(&out_filename_str);

        free_translation_unit(&tl);
        free_preproc_res(&preproc_res);
    }
}

struct token_arr {
    size_t len, cap;
    struct token* tokens;
};

struct preproc_cond {
    bool had_true_branch;
    bool had_else;
    struct source_loc loc;
};

struct preproc_state {
    struct token_arr res;
    size_t conds_len, conds_cap;
    struct preproc_cond* conds;
    struct preproc_err* err;
    struct string_hash_map _macro_map;
    struct file_info file_info;
};

struct preproc_state create_preproc_state(const char* start_file,
                                          struct preproc_err* err);

struct preproc_macro;

const struct preproc_macro* find_preproc_macro(const struct preproc_state* state,
                                               const struct str* spelling);

void register_preproc_macro(struct preproc_state* state,
                            const struct str* spelling,
                            const struct preproc_macro* macro);

void remove_preproc_macro(struct preproc_state* state, const struct str* spelling);

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true);

void pop_preproc_cond(struct preproc_state* state);

struct preproc_cond* peek_preproc_cond(struct preproc_state* state);

void free_token_arr(struct token_arr* arr);

void free_preproc_state(struct preproc_state* state);

struct token_or_arg {
    bool is_arg;
    union {
        size_t arg_num;
        struct token token;
    };
};

struct preproc_macro {
    const char* spell;
    bool is_func_macro;
    size_t num_args;
    bool is_variadic;

    size_t expansion_len;
    struct token_or_arg* expansion;
};

bool expand_preproc_macro(struct preproc_state* state,
                          struct token_arr* res,
                          const struct preproc_macro* macro,
                          size_t macro_idx,
                          size_t macro_end);

struct preproc_macro parse_preproc_macro(struct token_arr* arr,
                                         const char* spell,
                                         struct preproc_err* err);

void free_preproc_macro(struct preproc_macro* m);




void UNREACHABLE() {}

void FALLTHROUGH() {}

struct arch_type_info get_arch_type_info(enum arch a, bool is_windows) {
    // if x86 supported add it to assert
    assert(a == ARCH_X86_64 || !is_windows);
    switch (a) {
        case ARCH_X86_64: {
            struct arch_int_info int_info;
            if (is_windows) {
                int_info = (struct arch_int_info){
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 4,
                    .llint_size = 8,
                };
            } else {
                int_info = (struct arch_int_info){
                    .sint_size = 2,
                    .int_size = 4,
                    .lint_size = 8,
                    .llint_size = 8,
                };
            }

            return (struct arch_type_info){
                .int_info = int_info,
                .float_info = {
                    .float_size = 4,
                    .double_size = 8,
                    .ldouble_size = 8,
                },
            };
        }
    }
    UNREACHABLE();
}

/**
 * @brief Calls malloc(), exiting when malloc() fails
 *
 * @param bytes Size of allocation
 * @return void* Pointer to allocated storage
 */
void* xmalloc(size_t bytes);

/**
 * @brief calls calloc(), exiting when calloc() fails
 *
 * @param len Number of elements to allocate
 * @param elem_size Size of one element in bytes
 * @return void* Pointer to allocated storage, or NULL if len is zero
 */
void* xcalloc(size_t len, size_t elem_size);

/**
 * @brief Calls realloc(), exiting when realloc() fails
 * If bytes is zero, the given buffer is freed and NULL is returned
 *
 * @param alloc Existing allocation, or NULL
 * @param bytes New size for the allocation
 * @return void* Resized allocation
 */
void* xrealloc(void* alloc, size_t bytes);

/**
 * @brief Grows an existing allocation, writing the new allocation and its size
 * in the given pointers
 *
 * @param alloc Pointer to existing allocation, to which the resulting
 * allocation will be written
 * @param alloc_len Pointer to number of allocated elements, to which the new
 * number of elements will be written
 * @param elem_size Size of one element in bytes
 */
void grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size);

/**
 * @brief Allocates a copy of the given string
 *
 * @param str A zero-terminated string that must not be NULL
 * @return char* A heap allocated copy of str
 */
char* alloc_string_copy(const char* str);



struct err_base create_err_base(struct source_loc loc) {
    return (struct err_base){
        .loc = loc
    };
}

void print_err_base(FILE* out, const struct file_info* file_info, const struct err_base* err) {
    assert(err->loc.file_idx < file_info->len);
    const char* path = str_get_data(&file_info->paths[err->loc.file_idx]);
    fprintf(out,
            "%s(%zu, %zu):\n",
            path,
            err->loc.file_loc.line,
            err->loc.file_loc.index);
}

struct file_info create_file_info(const struct str* start_file) {
    struct file_info res = {
        .len = 1,
        .paths = xmalloc(sizeof *res.paths),
    };
    res.paths[0] = *start_file;
    return res;
}

void file_info_add(struct file_info* info, const struct str* path) {
    assert(path);

    ++info->len;
    info->paths = xrealloc(info->paths, sizeof *info->paths * info->len);
    info->paths[info->len - 1] = *path;
}

void free_file_info(struct file_info* info) {
    for (size_t i = 0;  i < info->len; ++i) {
        free_str(&info->paths[i]); 
    }
    free(info->paths);
}

struct token create_token(enum token_type type,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx) {
    assert(spelling);
    assert(file_idx != (size_t)-1);
    if (get_spelling(type) == NULL) {
        assert(str_is_valid(spelling));
    } else {
        assert(!str_is_valid(spelling));
    }

    return (struct token){
        .type = type,
        .spelling = *spelling,
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct token create_token_copy(enum token_type type,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx) {
    assert(spelling);
    assert(str_is_valid(spelling));

    return (struct token){
        .type = type,
        .spelling = str_copy(spelling),
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct str take_spelling(struct token* t) {
    assert(str_is_valid(&t->spelling));
    struct str spelling = str_take(&t->spelling);
    return spelling;
}

void free_token(struct token* t) {
    assert(t);
    if (t->type != I_CONSTANT && t->type != F_CONSTANT) {
        free_str(&t->spelling);
    }
}

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

    return NULL;
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

bool value_is_int(enum value_type t) {
    switch (t) {
        case VALUE_CHAR:
        case VALUE_SINT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT:
            return true;
        default:
            return false;
    }
}

bool value_is_uint(enum value_type t) {
    switch (t) {
        case VALUE_UCHAR:
        case VALUE_USINT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT:
            return true;
        default:
            return false;
    }
}

bool value_is_float(enum value_type t) {
    switch (t) {
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE:
            return true;
        default:
            return false;
    }
}

struct value create_int_value(enum value_type t, intmax_t val) {
    assert(value_is_int(t));
    return (struct value){
        .type = t,
        .int_val = val,
    };
}

struct value create_uint_value(enum value_type t, uintmax_t val) {
    assert(value_is_uint(t));
    return (struct value){
        .type = t,
        .uint_val = val,
    };
}

struct value create_float_value(enum value_type t, long double val) {
    assert(value_is_float(t));
    return (struct value){
        .type = t,
        .float_val = val,
    };
}

const char* get_value_type_str(enum value_type t) {
    switch (t) {
        case VALUE_CHAR:
            return "VALUE_CHAR";
        case VALUE_SINT:
            return "VALUE_SINT";
        case VALUE_INT:
            return "VALUE_INT";
        case VALUE_LINT:
            return "VALUE_LINT";
        case VALUE_LLINT:
            return "VALUE_LLINT";
        case VALUE_UCHAR:
            return "VALUE_UCHAR";
        case VALUE_USINT:
            return "VALUE_USINT";
        case VALUE_UINT:
            return "VALUE_UINT";
        case VALUE_ULINT:
            return "VALUE_ULINT";
        case VALUE_ULLINT:
            return "VALUE_ULLINT";
        case VALUE_FLOAT:
            return "VALUE_FLOAT";
        case VALUE_DOUBLE:
            return "VALUE_DOUBLE";
        case VALUE_LDOUBLE:
            return "VALUE_LDOUBLE";
    }

    UNREACHABLE();
}

struct ast_dumper {
    jmp_buf err_buf;
    FILE* file;
    size_t num_indents;
    const struct file_info* file_info;
};

static void add_indent(struct ast_dumper* d) {
    d->num_indents += 1;
}

static void remove_indent(struct ast_dumper* d) {
    d->num_indents -= 1;
}

static void print_indents(struct ast_dumper* d) {
    for (size_t i = 0; i < d->num_indents; ++i) {
        if (fprintf(d->file, "  ") < 0) {
            longjmp(d->err_buf, 0);
        }
    }
}

static void dumper_println(struct ast_dumper* d, const char* format, ...) {
    assert(format);
    print_indents(d);
    /* 
    va_list args;
    va_start(args, format);
    int res = vfprintf(d->file, format, args);
    va_end(args);

    if (res < 0) {
        longjmp(d->err_buf, 0);
    }

    if (fprintf(d->file, "\n") < 0) {
        longjmp(d->err_buf, 0);
    }
    */
}

static void dumper_print_node_head(struct ast_dumper* d,
                                   const char* name,
                                   const struct ast_node_info* node) {
    const struct source_loc* loc = &node->loc;
    assert(loc->file_idx < d->file_info->len);
    const char* file_path = str_get_data(&d->file_info->paths[loc->file_idx]);
    dumper_println(d,
                   "%s: %s:%zu,%zu",
                   name,
                   file_path,
                   loc->file_loc.line,
                   loc->file_loc.index);
}

static void dump_translation_unit(struct ast_dumper* d,
                                  const struct translation_unit* tl);

bool dump_ast(const struct translation_unit* tl,
              const struct file_info* file_info,
              FILE* f) {
    struct ast_dumper d = {
        .file = f,
        .file_info = file_info,
        .num_indents = 0,
    };

    if (setjmp(d.err_buf) == 0) {
        dump_translation_unit(&d, tl);
    } else {
        return false;
    }

    return true;
}

static const char* bool_to_str(bool b) {
    return b ? "true" : "false";
}

static void dump_func_specs(struct ast_dumper* d, const struct func_specs* s) {
    assert(s);

    dumper_println(d, "func_specs:");

    add_indent(d);

    dumper_println(d, "is_inline: %s", bool_to_str(s->is_inline));
    dumper_println(d, "is_noreturn: %s", bool_to_str(s->is_noreturn));

    remove_indent(d);
}

static void dump_storage_class(struct ast_dumper* d,
                               const struct storage_class* c) {
    assert(c);

    dumper_println(d, "storage_class:");

    add_indent(d);

    dumper_println(d, "is_typedef: %s", bool_to_str(c->is_typedef));
    dumper_println(d, "is_extern: %s", bool_to_str(c->is_extern));
    dumper_println(d, "is_static: %s", bool_to_str(c->is_static));
    dumper_println(d, "is_thread_local: %s", bool_to_str(c->is_thread_local));
    dumper_println(d, "is_auto: %s", bool_to_str(c->is_auto));
    dumper_println(d, "is_register: %s", bool_to_str(c->is_register));

    remove_indent(d);
}

static void dump_type_quals(struct ast_dumper* d, const struct type_quals* q) {
    assert(q);

    dumper_println(d, "type_quals:");

    add_indent(d);

    dumper_println(d, "is_const: %s", bool_to_str(q->is_const));
    dumper_println(d, "is_restrict: %s", bool_to_str(q->is_restrict));
    dumper_println(d, "is_volatile: %s", bool_to_str(q->is_volatile));
    dumper_println(d, "is_atomic: %s", bool_to_str(q->is_atomic));

    remove_indent(d);
}

static void dump_type_name(struct ast_dumper* d, const struct type_name* n);
static void dump_const_expr(struct ast_dumper* d, const struct const_expr* e);

static void dump_align_spec(struct ast_dumper* d, const struct align_spec* s) {
    assert(s);

    dumper_print_node_head(d, "align_spec", &s->info);

    add_indent(d);

    if (s->is_type_name) {
        dump_type_name(d, s->type_name);
    } else {
        dump_const_expr(d, s->const_expr);
    }

    remove_indent(d);
}

static void dump_type_specs(struct ast_dumper* d, const struct type_specs* s);

static void dump_declaration_specs(struct ast_dumper* d,
                                   const struct declaration_specs* s) {
    assert(s);

    dumper_print_node_head(d, "declaration_specs", &s->info);

    add_indent(d);

    dump_func_specs(d, &s->func_specs);
    dump_storage_class(d, &s->storage_class);

    dump_type_quals(d, &s->type_quals);

    dumper_println(d, "num_align_specs: %zu", s->num_align_specs);
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        dump_align_spec(d, &s->align_specs[i]);
    }

    dump_type_specs(d, &s->type_specs);

    remove_indent(d);
}

static void dump_pointer(struct ast_dumper* d, const struct pointer* p) {
    assert(p);

    dumper_print_node_head(d, "pointer", &p->info);

    add_indent(d);

    dumper_println(d, "num_indirs: %zu", p->num_indirs);
    for (size_t i = 0; i < p->num_indirs; ++i) {
        dump_type_quals(d, &p->quals_after_ptr[i]);
    }

    remove_indent(d);
}

static void dump_identifier(struct ast_dumper* d, struct identifier* i) {
    assert(i);

    dumper_print_node_head(d, "identifier", &i->info);

    add_indent(d);
    dumper_println(d, "spelling: %s", str_get_data(&i->spelling));
    remove_indent(d);
}

static void dump_value(struct ast_dumper* d, struct value val) {
    dumper_println(d, "value:");

    add_indent(d);
    
    dumper_println(d, "type: %s", get_value_type_str(val.type));
    if (value_is_int(val.type)) {
        dumper_println(d, "int_val: %jd", val.int_val);
    } else if (value_is_uint(val.type)) {
        dumper_println(d, "uint_val: %ju", val.uint_val);
    } else {
        dumper_println(d, "float_val: %Lg", val.float_val);
    }

    remove_indent(d);
}

static void dump_constant(struct ast_dumper* d, const struct constant* c) {
    assert(c);

    dumper_print_node_head(d, "constant", &c->info);

    add_indent(d);

    switch (c->type) {
        case CONSTANT_ENUM:
            dumper_println(d, "enum: %s", str_get_data(&c->spelling));
            break;
        case CONSTANT_FLOAT:
        case CONSTANT_INT:
            dump_value(d, c->val);
            break;
    }

    remove_indent(d);
}

static void dump_string_literal(struct ast_dumper* d,
                                const struct string_literal* l) {
    assert(l);
    dumper_print_node_head(d, "string_literal", &l->info);

    add_indent(d);
    dumper_println(d, "%s", str_get_data(&l->spelling));
    remove_indent(d);
}

static void dump_string_constant(struct ast_dumper* d,
                                 const struct string_constant* c) {
    assert(c);

    add_indent(d);

    if (c->is_func) {
        dumper_print_node_head(d, "string_constant", &c->info);
        dumper_println(d, "%s", get_spelling(FUNC_NAME));
    } else {
        dumper_println(d, "string_constant:");
        dump_string_literal(d, &c->lit);
    }

    remove_indent(d);
}

static void dump_assign_expr(struct ast_dumper* d, const struct assign_expr* e);

static void dump_generic_assoc(struct ast_dumper* d,
                               const struct generic_assoc* a) {
    assert(a);

    dumper_print_node_head(d, "generic_assoc", &a->info);

    add_indent(d);

    if (a->type_name) {
        dump_type_name(d, a->type_name);
    } else {
        dumper_println(d, "default");
    }

    dump_assign_expr(d, a->assign);

    remove_indent(d);
}

static void dump_generic_assoc_list(struct ast_dumper* d,
                                    const struct generic_assoc_list* l) {
    assert(l);

    dumper_print_node_head(d, "generic_assoc_list", &l->info);

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);

    for (size_t i = 0; i < l->len; ++i) {
        dump_generic_assoc(d, &l->assocs[i]);
    }

    remove_indent(d);
}

static void dump_generic_sel(struct ast_dumper* d,
                             const struct generic_sel* s) {
    assert(d);

    dumper_print_node_head(d, "generic_sel", &s->info);

    add_indent(d);

    dump_assign_expr(d, s->assign);

    dump_generic_assoc_list(d, &s->assocs);

    remove_indent(d);
}

static void dump_expr(struct ast_dumper* d, const struct expr* e);

static void dump_primary_expr(struct ast_dumper* d,
                              const struct primary_expr* e) {
    assert(e);

    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_identifier(d, e->identifier);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_CONSTANT:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_constant(d, &e->constant);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_string_constant(d, &e->string);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_BRACKET:
            dumper_print_node_head(d, "primary_expr", &e->info);

            add_indent(d);

            dump_expr(d, e->bracket_expr);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_GENERIC:
            dumper_println(d, "primary_expr:");
            add_indent(d);

            dump_generic_sel(d, e->generic);

            remove_indent(d);
            break;
    }
}

static void dump_type_modifiers(struct ast_dumper* d,
                                const struct type_modifiers* m) {
    assert(m);

    dumper_println(d, "type_modifiers:");

    add_indent(d);

    dumper_println(d, "is_unsigned: %s", bool_to_str(m->is_unsigned));
    dumper_println(d, "is_signed: %s", bool_to_str(m->is_signed));
    dumper_println(d, "is_short: %s", bool_to_str(m->is_short));
    dumper_println(d, "num_long: %d", m->num_long);
    dumper_println(d, "is_complex: %s", bool_to_str(m->is_complex));
    dumper_println(d, "is_imaginary: %s", bool_to_str(m->is_imaginary));

    remove_indent(d);
}

static void dump_atomic_type_spec(struct ast_dumper* d,
                                  const struct atomic_type_spec* s) {
    assert(s);

    dumper_print_node_head(d, "atomic_type_spec", &s->info);

    add_indent(d);

    dump_type_name(d, s->type_name);

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl);

static void dump_struct_declarator(struct ast_dumper* d,
                                   struct struct_declarator* decl) {
    assert(decl);

    dumper_println(d, "struct_declarator:");

    add_indent(d);

    if (decl->decl) {
        dump_declarator(d, decl->decl);
    }
    if (decl->bit_field) {
        dump_const_expr(d, decl->bit_field);
    }

    remove_indent(d);
}

static void dump_struct_declarator_list(
    struct ast_dumper* d,
    const struct struct_declarator_list* l) {
    assert(l);

    dumper_println(d, "struct_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);

    for (size_t i = 0; i < l->len; ++i) {
        dump_struct_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_static_assert_declaration(
    struct ast_dumper* d,
    const struct static_assert_declaration* decl);

static void dump_struct_declaration(struct ast_dumper* d,
                                    const struct struct_declaration* decl) {
    assert(decl);

    dumper_println(d, "struct_declaration");

    add_indent(d);

    if (decl->is_static_assert) {
        dump_static_assert_declaration(d, decl->assert);
    } else {
        dump_declaration_specs(d, decl->decl_specs);
        dump_struct_declarator_list(d, &decl->decls);
    }

    remove_indent(d);
}

static void dump_struct_declaration_list(
    struct ast_dumper* d,
    const struct struct_declaration_list* l) {
    assert(l);

    dumper_println(d, "struct_declaration_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_struct_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_struct_union_spec(struct ast_dumper* d,
                                   const struct struct_union_spec* s) {
    assert(d);

    dumper_print_node_head(d, "struct_union_spec", &s->info);

    add_indent(d);

    if (s->is_struct) {
        dumper_println(d, "struct");
    } else {
        dumper_println(d, "union");
    }

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_struct_declaration_list(d, &s->decl_list);

    remove_indent(d);
}

static void dump_enumerator(struct ast_dumper* d, const struct enumerator* e) {
    assert(e);

    dumper_println(d, "enumerator:");

    add_indent(d);

    dump_identifier(d, e->identifier);
    if (e->enum_val) {
        dump_const_expr(d, e->enum_val);
    }

    remove_indent(d);
}

static void dump_enum_list(struct ast_dumper* d, const struct enum_list* l) {
    assert(l);

    dumper_println(d, "enum_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_enumerator(d, &l->enums[i]);
    }

    remove_indent(d);
}

static void dump_enum_spec(struct ast_dumper* d, const struct enum_spec* s) {
    assert(s);

    dumper_print_node_head(d, "enum_spec", &s->info);

    add_indent(d);

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_enum_list(d, &s->enum_list);

    remove_indent(d);
}

static const char* type_spec_type_str(enum type_spec_type t) {
    switch (t) {
        case TYPE_SPEC_NONE:
            return "NO_TYPE_SPEC";
        case TYPE_SPEC_VOID:
            return get_spelling(VOID);
        case TYPE_SPEC_CHAR:
            return get_spelling(CHAR);
        case TYPE_SPEC_INT:
            return get_spelling(INT);
        case TYPE_SPEC_FLOAT:
            return get_spelling(FLOAT);
        case TYPE_SPEC_DOUBLE:
            return get_spelling(DOUBLE);
        case TYPE_SPEC_BOOL:
            return get_spelling(BOOL);
        default:
            UNREACHABLE();
    }
}

static void dump_type_specs(struct ast_dumper* d, const struct type_specs* s) {
    assert(s);

    dumper_println(d, "type_specs:");

    add_indent(d);

    dump_type_modifiers(d, &s->mods);

    switch (s->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            dumper_println(d, "%s", type_spec_type_str(s->type));
            break;
        case TYPE_SPEC_ATOMIC:
            dump_atomic_type_spec(d, s->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            dump_struct_union_spec(d, s->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            dump_enum_spec(d, s->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            dump_identifier(d, s->typedef_name);
            break;
    }

    remove_indent(d);
}

static void dump_spec_qual_list(struct ast_dumper* d,
                                const struct spec_qual_list* l) {
    assert(l);

    dumper_print_node_head(d, "spec_qual_list", &l->info);

    add_indent(d);

    dump_type_quals(d, &l->quals);
    dump_type_specs(d, &l->specs);

    remove_indent(d);
}

static void dump_abs_declarator(struct ast_dumper* d,
                                const struct abs_declarator* decl);

static void dump_type_name(struct ast_dumper* d, const struct type_name* n) {
    assert(n);

    dumper_println(d, "type_name:");

    add_indent(d);

    dump_spec_qual_list(d, n->spec_qual_list);
    if (n->abstract_decl) {
        dump_abs_declarator(d, n->abstract_decl);
    }

    remove_indent(d);
}

static void dump_arg_expr_list(struct ast_dumper* d,
                               const struct arg_expr_list* l) {
    assert(l);

    dumper_println(d, "arg_expr_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_assign_expr(d, &l->assign_exprs[i]);
    }

    remove_indent(d);
}

static void dump_postfix_suffix(struct ast_dumper* d,
                                const struct postfix_suffix* s) {
    assert(s);

    dumper_println(d, "postfix_suffix:");

    add_indent(d);

    switch (s->type) {
        case POSTFIX_INDEX:
            dump_expr(d, s->index_expr);
            break;
        case POSTFIX_BRACKET:
            dump_arg_expr_list(d, &s->bracket_list);
            break;
        case POSTFIX_ACCESS:
            dumper_println(d, "access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_PTR_ACCESS:
            dumper_println(d, "pointer_access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_INC_DEC:
            dumper_println(d, "%s", s->is_inc ? "++" : "--");
            break;
    }

    remove_indent(d);
}

static void dump_init_list(struct ast_dumper* d, const struct init_list* l);

static void dump_postfix_expr(struct ast_dumper* d, struct postfix_expr* e) {
    assert(e);

    if (e->is_primary) {
        dumper_println(d, "postfix_expr:");
    } else {
        dumper_print_node_head(d, "postfix_expr", &e->info);
    }

    add_indent(d);

    if (e->is_primary) {
        dump_primary_expr(d, e->primary);
    } else {
        dump_type_name(d, e->type_name);
        dump_init_list(d, &e->init_list);
    }

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_postfix_suffix(d, &e->suffixes[i]);
    }

    remove_indent(d);
}

static void dump_unary_expr(struct ast_dumper* d, const struct unary_expr* e);

static void dump_cast_expr(struct ast_dumper* d, const struct cast_expr* e) {
    assert(e);

    dumper_print_node_head(d, "cast_expr", &e->info);

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_type_name(d, &e->type_names[i]);
    }

    dump_unary_expr(d, e->rhs);

    remove_indent(d);
}

static const char* unary_expr_type_str(enum unary_expr_type t) {
    assert(t != UNARY_POSTFIX && t != UNARY_SIZEOF_TYPE && t != UNARY_ALIGNOF);
    switch (t) {
        case UNARY_ADDRESSOF:
            return get_spelling(AND);
        case UNARY_DEREF:
            return get_spelling(ASTERISK);
        case UNARY_PLUS:
            return get_spelling(ADD);
        case UNARY_MINUS:
            return get_spelling(SUB);
        case UNARY_BNOT:
            return get_spelling(BNOT);
        case UNARY_NOT:
            return get_spelling(NOT);

        default:
            UNREACHABLE();
    }
}

static const char* unary_expr_op_str(enum unary_expr_op o) {
    switch (o) {
        case UNARY_OP_INC:
            return get_spelling(INC_OP);
        case UNARY_OP_DEC:
            return get_spelling(DEC_OP);
        case UNARY_OP_SIZEOF:
            return get_spelling(SIZEOF);
    }
    UNREACHABLE();
}

static void dump_unary_expr(struct ast_dumper* d, const struct unary_expr* e) {
    assert(e);

    dumper_print_node_head(d, "unary_expr", &e->info);

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dumper_println(d, "%s", unary_expr_op_str(e->ops_before[i]));
    }

    switch (e->type) {
        case UNARY_POSTFIX:
            dump_postfix_expr(d, e->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            dumper_println(d, "%s", unary_expr_type_str(e->type));
            dump_cast_expr(d, e->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
            dumper_println(d, "sizeof");
            dump_type_name(d, e->type_name);
            break;
        case UNARY_ALIGNOF:
            dumper_println(d, "_Alignof");
            dump_type_name(d, e->type_name);
            break;
    }

    remove_indent(d);
}

static const char* assign_expr_op_str(enum assign_expr_op o) {
    switch (o) {
        case ASSIGN_EXPR_ASSIGN:
            return get_spelling(ASSIGN);
        case ASSIGN_EXPR_MUL:
            return get_spelling(MUL_ASSIGN);
        case ASSIGN_EXPR_DIV:
            return get_spelling(DIV_ASSIGN);
        case ASSIGN_EXPR_MOD:
            return get_spelling(MOD_ASSIGN);
        case ASSIGN_EXPR_ADD:
            return get_spelling(ADD_ASSIGN);
        case ASSIGN_EXPR_SUB:
            return get_spelling(SUB_ASSIGN);
        case ASSIGN_EXPR_LSHIFT:
            return get_spelling(LEFT_ASSIGN);
        case ASSIGN_EXPR_RSHIFT:
            return get_spelling(RIGHT_ASSIGN);
        case ASSIGN_EXPR_AND:
            return get_spelling(AND_ASSIGN);
        case ASSIGN_EXPR_XOR:
            return get_spelling(XOR_ASSIGN);
        case ASSIGN_EXPR_OR:
            return get_spelling(OR_ASSIGN);
    };

    UNREACHABLE();
}

static void dump_cond_expr(struct ast_dumper* d, const struct cond_expr* e);

static void dump_assign_expr(struct ast_dumper* d,
                             const struct assign_expr* e) {
    assert(e);

    dumper_println(d, "assign_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dumper_println(d, "unary_and_op:");
        add_indent(d);

        struct unary_and_op* item = &e->assign_chain[i];

        dump_unary_expr(d, item->unary);

        dumper_println(d, "%s", assign_expr_op_str(item->op));

        remove_indent(d);
    }
    dump_cond_expr(d, e->value);

    remove_indent(d);
}
static void dump_param_type_list(struct ast_dumper* d,
                                 const struct param_type_list* l);

static void dump_abs_arr_or_func_suffix(
    struct ast_dumper* d,
    const struct abs_arr_or_func_suffix* s) {
    assert(s);

    dumper_print_node_head(d, "abs_arr_or_func_suffix", &s->info);

    add_indent(d);

    switch (s->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            dumper_println(d, "has_asterisk: %s", bool_to_str(s->has_asterisk));
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            dumper_println(d, "is_static: %s", bool_to_str(s->is_static));
            dump_type_quals(d, &s->type_quals);
            if (s->assign) {
                dump_assign_expr(d, s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            dump_param_type_list(d, &s->func_types);
            break;
    }

    remove_indent(d);
}

static void dump_direct_abs_declarator(
    struct ast_dumper* d,
    const struct direct_abs_declarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_abs_declarator", &decl->info);

    add_indent(d);

    if (decl->bracket_decl) {
        dump_abs_declarator(d, decl->bracket_decl);
    }
    for (size_t i = 0; i < decl->len; ++i) {
        dump_abs_arr_or_func_suffix(d, &decl->following_suffixes[i]);
    }

    remove_indent(d);
}

static void dump_abs_declarator(struct ast_dumper* d,
                                const struct abs_declarator* decl) {
    assert(decl);

    dumper_println(d, "abs_declarator:");

    add_indent(d);
    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    if (decl->direct_abs_decl) {
        dump_direct_abs_declarator(d, decl->direct_abs_decl);
    }

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl);

static void dump_param_declaration(struct ast_dumper* d,
                                   const struct param_declaration* decl) {
    assert(decl);

    dumper_println(d, "param_declaration:");

    add_indent(d);

    dump_declaration_specs(d, decl->decl_specs);
    switch (decl->type) {
        case PARAM_DECL_DECL:
            dump_declarator(d, decl->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            dump_abs_declarator(d, decl->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            dumper_println(d, "no_decl");
            break;
    }

    remove_indent(d);
}

static void dump_param_list(struct ast_dumper* d, const struct param_list* l) {
    assert(l);

    dumper_println(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_param_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_param_type_list(struct ast_dumper* d,
                                 const struct param_type_list* l) {
    assert(l);

    dumper_println(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "is_variadic: %s", bool_to_str(l->is_variadic));
    dump_param_list(d, l->param_list);

    remove_indent(d);
}

static void dump_identifier_list(struct ast_dumper* d,
                                 const struct identifier_list* l) {
    assert(l);
    dumper_println(d, "identifier_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_identifier(d, &l->identifiers[i]);
    }

    remove_indent(d);
}

static void dump_arr_suffix(struct ast_dumper* d, const struct arr_suffix* s) {
    assert(s);

    dumper_println(d, "arr_suffix:");

    add_indent(d);

    dumper_println(d, "is_static: %s", bool_to_str(s->is_static));
    dump_type_quals(d, &s->type_quals);
    dumper_println(d, "is_asterisk: %s", bool_to_str(s->is_asterisk));
    if (s->arr_len) {
        dump_assign_expr(d, s->arr_len);
    }

    remove_indent(d);
}

static void dump_arr_or_func_suffix(struct ast_dumper* d,
                                    const struct arr_or_func_suffix* s) {
    dumper_print_node_head(d, "arr_or_func_suffix", &s->info);

    add_indent(d);

    switch (s->type) {
        case ARR_OR_FUNC_ARRAY:
            dump_arr_suffix(d, &s->arr_suffix);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            dump_param_type_list(d, &s->fun_types);
            break;
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            dump_identifier_list(d, &s->fun_params);
            break;
        case ARR_OR_FUNC_FUN_EMPTY:
            dumper_println(d, "empty_func_suffix");
            break;
    }

    remove_indent(d);
}

static void dump_direct_declarator(struct ast_dumper* d,
                                   struct direct_declarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_declarator", &decl->info);

    add_indent(d);

    if (decl->is_id) {
        dump_identifier(d, decl->id);
    } else {
        dump_declarator(d, decl->decl);
    }

    for (size_t i = 0; i < decl->len; ++i) {
        struct arr_or_func_suffix* item = &decl->suffixes[i];
        dump_arr_or_func_suffix(d, item);
    }

    remove_indent(d);
}

static void dump_declarator(struct ast_dumper* d,
                            const struct declarator* decl) {
    assert(decl);

    dumper_println(d, "declarator:");

    add_indent(d);

    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    dump_direct_declarator(d, decl->direct_decl);

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* d,
                             const struct declaration* decl);

static void dump_declaration_list(struct ast_dumper* d,
                                  const struct declaration_list* l) {
    assert(l);

    dumper_println(d, "declaration_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_statement(struct ast_dumper* d, const struct statement* s);

static void dump_labeled_statement(struct ast_dumper* d,
                                   const struct labeled_statement* s) {
    assert(s);

    dumper_print_node_head(d, "labeled_statement", &s->info);

    add_indent(d);

    switch (s->type) {
        case LABELED_STATEMENT_CASE:
            dump_const_expr(d, s->case_expr);
            break;
        case LABELED_STATEMENT_LABEL:
            dump_identifier(d, s->label);
            break;
        case LABELED_STATEMENT_DEFAULT:
            dumper_println(d, "default");
            break;
        default:
            UNREACHABLE();
    }

    dump_statement(d, s->stat);

    remove_indent(d);
}

static void dump_expr(struct ast_dumper* d, const struct expr* e) {
    assert(e);

    dumper_println(d, "expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_assign_expr(d, &e->assign_exprs[i]);
    }

    remove_indent(d);
}

static void dump_compound_statement(struct ast_dumper* d,
                                    const struct compound_statement* s);

static void dump_expr_statement(struct ast_dumper* d,
                                const struct expr_statement* s) {
    assert(s);
    dumper_print_node_head(d, "expr_statement", &s->info);

    add_indent(d);

    if (s->expr.len != 0) {
        dump_expr(d, &s->expr);
    }

    remove_indent(d);
}

static void dump_selection_statement(struct ast_dumper* d,
                                     const struct selection_statement* s) {
    assert(s);

    dumper_print_node_head(d, "selection_statement", &s->info);

    add_indent(d);

    dumper_println(d, "is_if: %s", bool_to_str(s->is_if));

    dump_statement(d, s->sel_stat);

    if (s->else_stat) {
        assert(s->is_if);
        dump_statement(d, s->else_stat);
    }

    remove_indent(d);
}

static void dump_iteration_statement(struct ast_dumper* d,
                                     const struct iteration_statement* s) {
    assert(s);

    dumper_print_node_head(d, "iteration_statement", &s->info);

    add_indent(d);

    switch (s->type) {
        case ITERATION_STATEMENT_WHILE:
            dumper_println(d, "type: while");
            dump_expr(d, s->while_cond);
            dump_statement(d, s->loop_body);
            break;
        case ITERATION_STATEMENT_DO:
            dumper_println(d, "type: do");
            dump_statement(d, s->loop_body);
            dump_expr(d, s->while_cond);
            break;
        case ITERATION_STATEMENT_FOR:
            dumper_println(d, "type: for");
            if (s->for_loop.is_decl) {
                dump_declaration(d, s->for_loop.init_decl);
            } else {
                dump_expr_statement(d, s->for_loop.init_expr);
            }
            dump_expr_statement(d, s->for_loop.cond);
            dump_expr(d, s->for_loop.incr_expr);
            dump_statement(d, s->loop_body);
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_jump_statement(struct ast_dumper* d,
                                const struct jump_statement* s) {
    assert(s);
    dumper_print_node_head(d, "jump_statement", &s->info);

    add_indent(d);

    switch (s->type) {
        case JUMP_STATEMENT_GOTO:
            dumper_println(d, "type: goto");
            dump_identifier(d, s->goto_label);
            break;
        case JUMP_STATEMENT_CONTINUE:
            dumper_println(d, "type: continue");
            break;
        case JUMP_STATEMENT_BREAK:
            dumper_println(d, "type: break");
            break;
        case JUMP_STATEMENT_RETURN:
            dumper_println(d, "type: return");
            if (s->ret_val) {
                dump_expr(d, s->ret_val);
            }
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_statement(struct ast_dumper* d, const struct statement* s) {
    assert(s);

    dumper_println(d, "statement:");

    add_indent(d);

    switch (s->type) {
        case STATEMENT_LABELED:
            dump_labeled_statement(d, s->labeled);
            break;
        case STATEMENT_COMPOUND:
            dump_compound_statement(d, s->comp);
            break;
        case STATEMENT_EXPRESSION:
            dump_expr_statement(d, s->expr);
            break;
        case STATEMENT_SELECTION:
            dump_selection_statement(d, s->sel);
            break;
        case STATEMENT_ITERATION:
            dump_iteration_statement(d, s->it);
            break;
        case STATEMENT_JUMP:
            dump_jump_statement(d, s->jmp);
            break;
    }

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* s,
                             const struct declaration* decl);

static void dump_block_item(struct ast_dumper* d, const struct block_item* i) {
    assert(i);

    dumper_println(d, "block_item:");

    add_indent(d);

    if (i->is_decl) {
        dump_declaration(d, &i->decl);
    } else {
        dump_statement(d, &i->stat);
    }

    remove_indent(d);
}

static void dump_compound_statement(struct ast_dumper* d,
                                    const struct compound_statement* s) {
    assert(s);

    dumper_print_node_head(d, "compound_statement", &s->info);

    add_indent(d);

    dumper_println(d, "len: %zu", s->len);
    for (size_t i = 0; i < s->len; ++i) {
        dump_block_item(d, &s->items[i]);
    }

    remove_indent(d);
}

static void dump_func_def(struct ast_dumper* d, const struct func_def* f) {
    assert(f);

    dumper_println(d, "func_def:");

    add_indent(d);

    dump_declaration_specs(d, f->specs);
    dump_declarator(d, f->decl);
    dump_declaration_list(d, &f->decl_list);
    dump_compound_statement(d, f->comp);

    remove_indent(d);
}

static void dump_designator(struct ast_dumper* d,
                            const struct designator* des) {
    assert(des);
    dumper_print_node_head(d, "designator", &des->info);

    add_indent(d);

    if (des->is_index) {
        dump_const_expr(d, des->arr_index);
    } else {
        dump_identifier(d, des->identifier);
    }

    remove_indent(d);
}

static void dump_designator_list(struct ast_dumper* d,
                                 const struct designator_list* l) {
    assert(l);

    dumper_println(d, "designator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_designator(d, &l->designators[i]);
    }

    remove_indent(d);
}

static void dump_designation(struct ast_dumper* d,
                             const struct designation* des) {
    assert(des);

    dumper_println(d, "designation:");

    add_indent(d);

    dump_designator_list(d, &des->designators);

    remove_indent(d);
}

static void dump_initializer(struct ast_dumper* d, const struct initializer* i);

static void dump_designation_init(struct ast_dumper* d,
                                  const struct designation_init* i) {
    assert(i);

    dumper_println(d, "designation_init:");

    add_indent(d);

    if (i->designation) {
        dump_designation(d, i->designation);
    }
    dump_initializer(d, i->init);

    remove_indent(d);
}

static void dump_init_list(struct ast_dumper* d, const struct init_list* l) {
    assert(l);

    dumper_println(d, "init_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_designation_init(d, &l->inits[i]);
    }

    remove_indent(d);
}

static void dump_initializer(struct ast_dumper* d,
                             const struct initializer* i) {
    assert(i);

    dumper_print_node_head(d, "initializer", &i->info);

    add_indent(d);

    if (i->is_assign) {
        dump_assign_expr(d, i->assign);
    } else {
        dump_init_list(d, &i->init_list);
    }

    remove_indent(d);
}

static void dump_init_declarator(struct ast_dumper* d,
                                 const struct init_declarator* decl) {
    assert(decl);

    dumper_println(d, "init_declarator:");

    add_indent(d);

    dump_declarator(d, decl->decl);
    if (decl->init) {
        dump_initializer(d, decl->init);
    }

    remove_indent(d);
}

static void dump_init_declarator_list(struct ast_dumper* d,
                                      const struct init_declarator_list* l) {
    assert(l);

    dumper_println(d, "init_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: %zu", l->len);
    for (size_t i = 0; i < l->len; ++i) {
        dump_init_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static const char* mul_expr_op_str(enum mul_expr_op o) {
    switch (o) {
        case MUL_EXPR_MUL:
            return get_spelling(ASTERISK);
        case MUL_EXPR_DIV:
            return get_spelling(DIV);
        case MUL_EXPR_MOD:
            return get_spelling(MOD);
    }
    UNREACHABLE();
}

static void dump_mul_expr(struct ast_dumper* d, const struct mul_expr* e) {
    assert(e);

    dumper_println(d, "mul_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_cast_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct cast_expr_and_op* item = &e->mul_chain[i];
        dumper_println(d, "mul_op: %s", mul_expr_op_str(item->op));
        dump_cast_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* add_expr_op_str(enum add_expr_op op) {
    switch (op) {
        case ADD_EXPR_ADD:
            return get_spelling(ADD);
        case ADD_EXPR_SUB:
            return get_spelling(SUB);
    }
    UNREACHABLE();
}

static void dump_add_expr(struct ast_dumper* d, const struct add_expr* e) {
    assert(e);

    dumper_println(d, "add_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_mul_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct mul_expr_and_op* item = &e->add_chain[i];
        dumper_println(d, "add_op: %s", add_expr_op_str(item->op));
        dump_mul_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* shift_expr_op_str(enum shift_expr_op o) {
    switch (o) {
        case SHIFT_EXPR_LEFT:
            return get_spelling(LEFT_OP);
        case SHIFT_EXPR_RIGHT:
            return get_spelling(RIGHT_OP);
    }
    UNREACHABLE();
}

static void dump_shift_expr(struct ast_dumper* d, const struct shift_expr* e) {
    assert(e);

    dumper_println(d, "shift_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_add_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct add_expr_and_op* item = &e->shift_chain[i];
        dumper_println(d, "shift_op: %s", shift_expr_op_str(item->op));
        dump_add_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* rel_expr_op_str(enum rel_expr_op o) {
    switch (o) {
        case REL_EXPR_LT:
            return get_spelling(LT);
        case REL_EXPR_GT:
            return get_spelling(GT);
        case REL_EXPR_LE:
            return get_spelling(LE_OP);
        case REL_EXPR_GE:
            return get_spelling(GE_OP);
    }
    UNREACHABLE();
}

static void dump_rel_expr(struct ast_dumper* d, const struct rel_expr* e) {
    assert(e);

    dumper_println(d, "rel_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_shift_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct shift_expr_and_op* item = &e->rel_chain[i];
        dumper_println(d, "rel_op: %s", rel_expr_op_str(item->op));
        dump_shift_expr(d, item->rhs);
    }

    remove_indent(d);
}

static const char* eq_expr_op_str(enum eq_expr_op o) {
    switch (o) {
        case EQ_EXPR_EQ:
            return get_spelling(EQ_OP);
        case EQ_EXPR_NE:
            return get_spelling(NE_OP);
    }
    UNREACHABLE();
}

static void dump_eq_expr(struct ast_dumper* d, const struct eq_expr* e) {
    assert(e);

    dumper_println(d, "eq_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    dump_rel_expr(d, e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        struct rel_expr_and_op* item = &e->eq_chain[i];
        dumper_println(d, "eq_op: %s", eq_expr_op_str(item->op));
        dump_rel_expr(d, item->rhs);
    }

    remove_indent(d);
}

static void dump_and_expr(struct ast_dumper* d, const struct and_expr* e) {
    assert(e);

    dumper_println(d, "and_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_eq_expr(d, &e->eq_exprs[i]);
    }

    remove_indent(d);
}

static void dump_xor_expr(struct ast_dumper* d, const struct xor_expr* e) {
    assert(e);

    dumper_println(d, "xor_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_and_expr(d, &e->and_exprs[i]);
    }

    remove_indent(d);
}

static void dump_or_expr(struct ast_dumper* d, const struct or_expr* e) {
    assert(e);

    dumper_println(d, "or_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_xor_expr(d, &e->xor_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_and_expr(struct ast_dumper* d,
                              const struct log_and_expr* e) {
    assert(e);

    dumper_println(d, "log_and_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_or_expr(d, &e->or_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_or_expr(struct ast_dumper* d,
                             const struct log_or_expr* e) {
    assert(e);

    dumper_println(d, "log_or_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        dump_log_and_expr(d, &e->log_ands[i]);
    }

    remove_indent(d);
}

static void dump_cond_expr(struct ast_dumper* d, const struct cond_expr* e) {
    assert(e);

    dumper_println(d, "cond_expr:");

    add_indent(d);

    dumper_println(d, "len: %zu", e->len);
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        dump_log_or_expr(d, item->log_or);
        dump_expr(d, item->expr);
    }
    dump_log_or_expr(d, e->last_else);

    remove_indent(d);
}

static void dump_const_expr(struct ast_dumper* d, const struct const_expr* e) {
    assert(e);

    dumper_println(d, "const_expr:");

    add_indent(d);

    dump_cond_expr(d, &e->expr);

    remove_indent(d);
}

static void dump_static_assert_declaration(
    struct ast_dumper* d,
    const struct static_assert_declaration* decl) {
    assert(decl);

    dumper_println(d, "static_assert_declaration:");

    add_indent(d);

    dump_const_expr(d, decl->const_expr);
    dump_string_literal(d, &decl->err_msg);

    remove_indent(d);
}

static void dump_declaration(struct ast_dumper* d,
                             const struct declaration* decl) {
    assert(decl);

    dumper_println(d, "declaration:");

    add_indent(d);

    if (decl->is_normal_decl) {
        dump_declaration_specs(d, decl->decl_specs);
        dump_init_declarator_list(d, &decl->init_decls);
    } else {
        dump_static_assert_declaration(d, decl->static_assert_decl);
    }

    remove_indent(d);
}

static void dump_external_declaration(struct ast_dumper* d,
                                      const struct external_declaration* decl) {
    assert(decl);

    dumper_println(d, "external_declaration:");

    add_indent(d);

    if (decl->is_func_def) {
        dump_func_def(d, &decl->func_def);
    } else {
        dump_declaration(d, &decl->decl);
    }

    remove_indent(d);
}

static void dump_translation_unit(struct ast_dumper* d,
                                  const struct translation_unit* tl) {
    assert(tl);

    dumper_println(d, "translation_unit:");

    add_indent(d);

    dumper_println(d, "len: %zu", tl->len);

    for (size_t i = 0; i < tl->len; ++i) {
        dump_external_declaration(d, &tl->external_decls[i]);
    }

    remove_indent(d);
}

struct ast_node_info create_ast_node_info(struct source_loc loc) {
    assert(loc.file_idx != (size_t)-1);
    assert(loc.file_loc.line != 0);
    assert(loc.file_loc.index != 0);
    return (struct ast_node_info) {
        .loc = loc,
    };
}

void init_identifier(struct identifier* res, const struct str* spelling, struct source_loc loc) {
    assert(res);
    res->info = create_ast_node_info(loc);
    res->spelling = *spelling;
}

struct identifier* create_identifier(const struct str* spelling, struct source_loc loc) {
    struct identifier* res = xmalloc(sizeof(struct identifier));
    init_identifier(res, spelling, loc);
    return res;
}

void free_identifier_children(struct identifier* i) {
    free_str(&i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    free(i);
}

static bool parse_spec_or_qual(struct parser_state* s,
                               struct spec_qual_list* res) {
    assert(res);

    if (is_type_qual(s->it->type)) {
        update_type_quals(s, &res->quals);
    } else if (!update_type_specs(s, &res->specs)) {
        return false;
    }

    return true;
}

void expected_token_error(struct parser_state* s, enum token_type expected);

void expected_tokens_error(struct parser_state* s,
                           const enum token_type* expected,
                           size_t num_expected);

/**
 *
 * @param s current state
 * @return bool whether the next token could be the start of a type name
 */
bool next_is_type_name(const struct parser_state* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is a type specifier
 */
bool is_type_spec(const struct parser_state* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is the start of a declaration
 */
bool is_declaration(const struct parser_state* s);



struct spec_qual_list parse_spec_qual_list(struct parser_state* s) {
    struct spec_qual_list res = {
        .info = create_ast_node_info(s->it->loc),
        .quals = create_type_quals(),
        .specs = create_type_specs(),
    };

    if (!parse_spec_or_qual(s, &res)) {
        return (struct spec_qual_list){
            .quals = create_type_quals(),
            .specs = create_type_specs(),
        };
    }

    while (is_type_spec(s) || is_type_qual(s->it->type)) {
        if (!parse_spec_or_qual(s, &res)) {
            free_spec_qual_list_children(&res);
            return (struct spec_qual_list){.quals = create_type_quals(),
                                           .specs = create_type_specs()};
        }
    }

    return res;
}

void free_spec_qual_list_children(struct spec_qual_list* l) {
    free_type_specs_children(&l->specs);
}

void free_spec_qual_list(struct spec_qual_list* l) {
    free_spec_qual_list_children(l);
}

bool is_valid_spec_qual_list(struct spec_qual_list* l) {
    return is_valid_type_quals(&l->quals) || is_valid_type_specs(&l->specs);
}

struct string_literal create_string_literal(const struct str* spelling,
                                            struct source_loc loc) {
    return (struct string_literal){
        .info = create_ast_node_info(loc),
        .spelling = *spelling,
    };
}

void free_string_literal(struct string_literal* l) {
    free_str(&l->spelling);
}

struct translation_unit parse_translation_unit(struct parser_state* s) {
    struct translation_unit res;
    size_t alloc_num = 1;
    res.len = 0;
    res.external_decls = xmalloc(sizeof(struct external_declaration)
                                 * alloc_num);

    while (s->it->type != INVALID) {
        if (res.len == alloc_num) {
            grow_alloc((void**)&res.external_decls,
                       &alloc_num,
                       sizeof(struct external_declaration));
        }

        if (!parse_external_declaration_inplace(s,
                                                &res.external_decls[res.len])) {
            free_translation_unit(&res);
            return (struct translation_unit){.len = 0, .external_decls = NULL};
        }

        ++res.len;
    }

    if (res.len != alloc_num) {
        res.external_decls = xrealloc(
            res.external_decls,
            res.len * sizeof(struct external_declaration));
    }

    return res;
}

static void free_translation_unit_children(struct translation_unit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    free(u->external_decls);
}

void free_translation_unit(struct translation_unit* u) {
    free_translation_unit_children(u);
}

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res) {
    assert(res);
    
    if (is_type_spec(s) || is_type_qual(s->it->type)) {
        res->spec_qual_list = xmalloc(sizeof(struct spec_qual_list));
        *res->spec_qual_list = parse_spec_qual_list(s);
        if (!is_valid_spec_qual_list(res->spec_qual_list)) {
            return false;
        }
    } else {
        // might be better for the error to just say "Expected type specifier or
        // type qualifier"
        enum token_type expected[] = {
            VOID,     CHAR,     SHORT,    INT,  LONG,         FLOAT,
            DOUBLE,   SIGNED,   UNSIGNED, BOOL, COMPLEX,      IMAGINARY,
            ATOMIC,   STRUCT,   UNION,    ENUM, TYPEDEF_NAME, CONST,
            RESTRICT, VOLATILE, ATOMIC,
        };
        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
        return false;
    }

    if (s->it->type == ASTERISK || s->it->type == LBRACKET
        || s->it->type == LINDEX) {
        res->abstract_decl = parse_abs_declarator(s);
        if (!res->abstract_decl) {
            free_spec_qual_list(res->spec_qual_list);
            return false;
        }
    } else {
        res->abstract_decl = NULL;
    }

    return true;
}

struct type_name* parse_type_name(struct parser_state* s) {
    struct type_name* res = xmalloc(sizeof(struct type_name));
    if (!parse_type_name_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_type_name_children(struct type_name* n) {
    free_spec_qual_list(n->spec_qual_list);
    free(n->spec_qual_list);
    if (n->abstract_decl) {
        free_abs_declarator(n->abstract_decl);
    }
}

void free_type_name(struct type_name* n) {
    free_type_name_children(n);
    free(n);
}

struct abs_declarator* parse_abs_declarator(struct parser_state* s) {
    struct abs_declarator* res = xmalloc(sizeof(struct abs_declarator));
    if (s->it->type == ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    if (s->it->type == ASTERISK || s->it->type == LBRACKET
        || s->it->type == LINDEX) {
        res->direct_abs_decl = parse_direct_abs_declarator(s);
        if (!res->direct_abs_decl) {
            if (res->ptr) {
                free_pointer(res->ptr);
            }
            free(res);
            return NULL;
        }
    } else {
        res->direct_abs_decl = NULL;
    }

    if (res->direct_abs_decl == NULL && res->ptr == NULL) {
        free(res);
        return NULL;
    }

    return res;
}

static void free_abs_declarator_children(struct abs_declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abs_declarator(d->direct_abs_decl);
    }
}

void free_abs_declarator(struct abs_declarator* d) {
    free_abs_declarator_children(d);
    free(d);
}

static void free_align_spec(struct align_spec* s);

bool parse_align_spec_inplace(struct parser_state* s, struct align_spec* res) {
    assert(res);
    res->info = create_ast_node_info(s->it->loc);
    if (!(accept(s, ALIGNAS) && accept(s, LBRACKET))) {
        return false;
    }

    // TODO: this condition may be wrong
    if (is_type_spec(s) || is_type_qual(s->it->type)) {
        res->is_type_name = true;
        res->type_name = parse_type_name(s);

        if (!res->type_name) {
            return false;
        }
    } else {
        res->is_type_name = false;
        res->const_expr = parse_const_expr(s);
        if (!res->const_expr) {
            return false;
        }
    }

    if (!accept(s, RBRACKET)) {
        free_align_spec(res);
        return false;
    }

    return true;
}

void free_align_spec_children(struct align_spec* s) {
    if (s->is_type_name) {
        free_type_name(s->type_name);
    } else {
        free_const_expr(s->const_expr);
    }
}

static void free_align_spec(struct align_spec* s) {
    free_align_spec_children(s);
    free(s);
}

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, ATOMIC)) {
        return NULL;
    }

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct type_name* type_name = parse_type_name(s);
    if (!type_name) {
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_type_name(type_name);
        return NULL;
    }

    struct atomic_type_spec* res = xmalloc(sizeof(struct atomic_type_spec));
    res->info = create_ast_node_info(loc);
    res->type_name = type_name;
    return res;
}

void free_atomic_type_spec(struct atomic_type_spec* s) {
    free_type_name(s->type_name);
    free(s);
}

bool parse_declaration_inplace(struct parser_state* s,
                               struct declaration* res) {
    assert(res);
    if (s->it->type == STATIC_ASSERT) {
        res->is_normal_decl = false;
        res->static_assert_decl = parse_static_assert_declaration(s);
        if (!res->static_assert_decl) {
            return false;
        }
    } else {
        res->is_normal_decl = true;

        bool found_typedef = false;
        res->decl_specs = parse_declaration_specs(s, &found_typedef);
        if (!res->decl_specs) {
            return false;
        }

        if (s->it->type != SEMICOLON) {
            if (found_typedef) {
                res->init_decls = parse_init_declarator_list_typedef(s);
            } else {
                res->init_decls = parse_init_declarator_list(s);
            }
            if (res->init_decls.len == 0) {
                free_declaration_specs(res->decl_specs);
                return false;
            }
        } else {
            res->init_decls = (struct init_declarator_list){
                .len = 0,
                .decls = NULL,
            };
        }
        if (!accept(s, SEMICOLON)) {
            free_declaration_specs(res->decl_specs);
            free_init_declarator_list(&res->init_decls);
            return false;
        }
    }

    return true;
}

struct declaration* parse_declaration(struct parser_state* s) {
    struct declaration* res = xmalloc(sizeof(struct declaration));
    if (!parse_declaration_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_declaration_children(struct declaration* d) {
    if (d->is_normal_decl) {
        free_declaration_specs(d->decl_specs);
        free_init_declarator_list(&d->init_decls);
    } else {
        free_static_assert_declaration(d->static_assert_decl);
    }
}

void free_declaration(struct declaration* d) {
    free_declaration_children(d);
    free(d);
}

struct declaration_list parse_declaration_list(struct parser_state* s) {
    struct declaration_list res = {
        .len = 1,
        .decls = xmalloc(sizeof(struct declaration))};

    if (!parse_declaration_inplace(s, res.decls)) {
        free(res.decls);
        return (struct declaration_list){.len = 0, .decls = NULL};
    }

    size_t alloc_size = res.len;
    while (is_declaration(s)) {
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.decls,
                       &alloc_size,
                       sizeof(struct declaration));
        }

        if (!parse_declaration_inplace(s, &res.decls[res.len])) {
            free_declaration_list(&res);
            return (struct declaration_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    return res;
}

void free_declaration_list(struct declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

static bool current_is_type_qual(const struct parser_state* s) {
    if (is_type_qual(s->it->type)) {
        if (s->it->type == ATOMIC) {
            return s->it[1].type != LBRACKET;
        } else {
            return true;
        }
    } else {
        return false;
    }
}


enum parse_declaration_spec_res {
    DECL_SPEC_ERROR,
    DECL_SPEC_SUCCESS,
    DECL_SPEC_LAST, // if this is the last declaration spec
};

/**
 *
 * @param s The current parser_state
 * @param res The address where the result is to be written in
 * @param alloc_len_align_specs The length of the current allocation in
 * res->align_specs
 * @param alloc_len_type_specs The length of the current allocation in
 * res->type_specs
 *
 * @return 0 for an error, 1 for success and 2 if the next token is not a
 * declaration_spec
 */
static enum parse_declaration_spec_res parse_declaration_spec(struct parser_state* s,
                           struct declaration_specs* res,
                           size_t* alloc_len_align_specs) {
    assert(res);
    assert(alloc_len_align_specs);

    if (is_storage_class_spec(s->it->type)) {
        struct storage_class* sc = &res->storage_class;
        switch (s->it->type) {
            case TYPEDEF:
                sc->is_typedef = true;
                break;
            case EXTERN:
                sc->is_extern = true;
                break;
            case STATIC:
                sc->is_static = true;
                break;
            case THREAD_LOCAL:
                sc->is_thread_local = true;
                break;
            case AUTO:
                sc->is_auto = true;
                break;
            case REGISTER:
                sc->is_register = true;
                break;
            default:
                UNREACHABLE();
        }
        accept_it(s);
    } else if (current_is_type_qual(s)) {
        update_type_quals(s, &res->type_quals);
    } else if (is_type_spec(s)) {
        if (!update_type_specs(s, &res->type_specs)) {
            return DECL_SPEC_ERROR;
        }
    } else if (is_func_spec(s->it->type)) {
        struct func_specs* fs = &res->func_specs;
        switch (s->it->type) {
            case INLINE:
                fs->is_inline = true;
                break;
            case NORETURN:
                fs->is_noreturn = true;
                break;
            default:
                UNREACHABLE();
        }
        accept_it(s);
    } else if (s->it->type == ALIGNAS) {
        if (res->num_align_specs == *alloc_len_align_specs) {
            grow_alloc((void**)&res->align_specs,
                       alloc_len_align_specs,
                       sizeof(struct align_spec));
        }

        if (!parse_align_spec_inplace(
                s,
                &res->align_specs[res->num_align_specs])) {
            return DECL_SPEC_ERROR;
        }

        ++res->num_align_specs;
    } else {
        return DECL_SPEC_LAST;
    }

    return DECL_SPEC_SUCCESS;
}

struct declaration_specs* parse_declaration_specs(struct parser_state* s,
                                                  bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    struct declaration_specs* res = xmalloc(sizeof(struct declaration_specs));
    res->info = create_ast_node_info(s->it->loc);
    res->func_specs = (struct func_specs){
        .is_inline = false,
        .is_noreturn = false,
    };

    res->storage_class = (struct storage_class){
        .is_typedef = false,
        .is_extern = false,
        .is_static = false,
        .is_thread_local = false,
        .is_auto = false,
        .is_register = false,
    };

    res->type_quals = create_type_quals();

    res->align_specs = NULL;
    res->num_align_specs = 0;
    size_t alloc_len_align_specs = 0;

    res->type_specs = create_type_specs();

    while (true) {
        enum parse_declaration_spec_res success = parse_declaration_spec(s, res, &alloc_len_align_specs);

        if (success == DECL_SPEC_ERROR) {
            free_declaration_specs(res);
            return NULL;
        } else if (success == DECL_SPEC_LAST) {
            break;
        }
    }

    res->align_specs = xrealloc(res->align_specs,
                                sizeof(struct align_spec)
                                    * res->num_align_specs);

    *found_typedef = res->storage_class.is_typedef;

    return res;
}

static void free_declaration_specs_children(struct declaration_specs* s) {
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        free_align_spec_children(&s->align_specs[i]);
    }
    free(s->align_specs);
    free_type_specs_children(&s->type_specs);
}

void free_declaration_specs(struct declaration_specs* s) {
    free_declaration_specs_children(s);

    free(s);
}

static struct declarator* parse_declarator_base(
    struct parser_state* s,
    struct direct_declarator* (*parse_func)(struct parser_state* s)) {
    struct declarator* res = xmalloc(sizeof(struct declarator));
    if (s->it->type == ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    res->direct_decl = parse_func(s);
    if (!res->direct_decl) {
        if (res->ptr) {
            free_pointer(res->ptr);
        }
        free(res);
        return NULL;
    }

    return res;
}

struct declarator* parse_declarator_typedef(struct parser_state* s) {
    return parse_declarator_base(s, parse_direct_declarator_typedef);
}

struct declarator* parse_declarator(struct parser_state* s) {
    return parse_declarator_base(s, parse_direct_declarator);
}

static void free_declarator_children(struct declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(struct declarator* d) {
    free_declarator_children(d);
    free(d);
}

static void free_abs_arr_or_func_suffix(struct abs_arr_or_func_suffix* s);

static bool parse_abs_func_suffix(struct parser_state* s,
                                  struct abs_arr_or_func_suffix* res) {
    assert(s->it->type == LBRACKET);
    accept_it(s);
    res->type = ABS_ARR_OR_FUNC_SUFFIX_FUNC;
    if (s->it->type == RBRACKET) {
        res->func_types = (struct param_type_list){
            .is_variadic = false,
            .param_list = NULL,
        };
        accept_it(s);
    } else {
        res->func_types = parse_param_type_list(s);
        if (res->func_types.param_list == NULL) {
            return false;
        }

        if (!accept(s, RBRACKET)) {
            free_param_type_list(&res->func_types);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_suffix(struct parser_state* s,
                                 struct abs_arr_or_func_suffix* res) {
    assert(s->it->type == LINDEX);
    accept_it(s);
    if (s->it->type == RINDEX) {
        res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = false;
        accept_it(s);
        return true;
    } else if (s->it->type == ASTERISK) {
        res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = true;
        accept_it(s);
        res->assign = NULL;
        if (!accept(s, RINDEX)) {
            return false;
        }
        return true;
    }

    res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN;
    res->is_static = false;
    if (s->it->type == STATIC) {
        accept_it(s);
        res->is_static = true;
    }

    if (is_type_qual(s->it->type)) {
        res->type_quals = parse_type_qual_list(s);
        if (!is_valid_type_quals(&res->type_quals)) {
            return false;
        }

        if (s->it->type == STATIC) {
            if (res->is_static) {
                set_parser_err(s->err,
                               PARSER_ERR_ARR_DOUBLE_STATIC,
                               s->it->loc);
                free_abs_arr_or_func_suffix(res);
                return false;
            } else {
                accept_it(s);
                res->is_static = true;
            }
        }
    }

    if (s->it->type == RINDEX) {
        if (res->is_static) {
            set_parser_err(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it->loc);
            free_abs_arr_or_func_suffix(res);
            return false;
        }
        res->assign = NULL;
        accept_it(s);
    } else {
        res->assign = parse_assign_expr(s);
        if (!(res->assign && accept(s, RINDEX))) {
            free_abs_arr_or_func_suffix(res);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_or_func_suffix(struct parser_state* s,
                                         struct abs_arr_or_func_suffix* res) {
    assert(res);
    assert(s->it->type == LBRACKET || s->it->type == LINDEX);
    res->info = create_ast_node_info(s->it->loc);

    switch (s->it->type) {
        case LBRACKET:
            return parse_abs_func_suffix(s, res);
        case LINDEX:
            return parse_abs_arr_suffix(s, res);
        default:
            UNREACHABLE();
    }
}

struct direct_abs_declarator* parse_direct_abs_declarator(
    struct parser_state* s) {
    struct direct_abs_declarator* res = xmalloc(
        sizeof(struct direct_abs_declarator));
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == LBRACKET
        && (s->it[1].type == LBRACKET || s->it[1].type == LINDEX
            || s->it[1].type == ASTERISK)) {
        accept_it(s);
        res->bracket_decl = parse_abs_declarator(s);
        if (!res->bracket_decl) {
            free(res);
            return NULL;
        }

        if (!accept(s, RBRACKET)) {
            free_abs_declarator(res->bracket_decl);
            free(res);
            return NULL;
        }
    } else {
        res->bracket_decl = NULL;
    }

    res->following_suffixes = NULL;
    res->len = 0;
    size_t alloc_len = res->len;
    while (s->it->type == LBRACKET || s->it->type == LINDEX) {
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->following_suffixes,
                       &alloc_len,
                       sizeof(struct abs_arr_or_func_suffix));
        }

        if (!parse_abs_arr_or_func_suffix(s,
                                          &res->following_suffixes[res->len])) {
            free_direct_abs_declarator(res);
            return NULL;
        }

        ++res->len;
    }

    res->following_suffixes = xrealloc(res->following_suffixes,
                                       sizeof(struct abs_arr_or_func_suffix)
                                           * res->len);

    return res;
}

static void free_abs_arr_or_func_suffix(struct abs_arr_or_func_suffix* s) {
    switch (s->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            if (s->assign) {
                free_assign_expr(s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            free_param_type_list(&s->func_types);
            break;
        default:
            break;
    }
}

static void free_direct_abs_declarator_children(struct direct_abs_declarator* d) {
    if (d->bracket_decl) {
        free_abs_declarator(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        free_abs_arr_or_func_suffix(&d->following_suffixes[i]);
    }
    free(d->following_suffixes);
}

void free_direct_abs_declarator(struct direct_abs_declarator* d) {
    free_direct_abs_declarator_children(d);
    free(d);
}

static void free_arr_suffix(struct arr_suffix* s) {
    if (s->arr_len) {
        free_assign_expr(s->arr_len);
    }
}

static bool parse_arr_suffix(struct parser_state* s,
                             struct arr_or_func_suffix* res) {
    assert(s->it->type == LINDEX);

    res->type = ARR_OR_FUNC_ARRAY;
    struct arr_suffix* suffix = &res->arr_suffix;
    *suffix = (struct arr_suffix){
        .is_static = false,
        .type_quals = create_type_quals(),
        .is_asterisk = false,
        .arr_len = NULL,
    };
    accept_it(s);
    if (s->it->type == ASTERISK) {
        accept_it(s);
        suffix->is_asterisk = true;
        if (!accept(s, RINDEX)) {
            return false;
        }
        return true;
    } else if (s->it->type == RINDEX) {
        accept_it(s);
        return true;
    }

    if (s->it->type == STATIC) {
        accept_it(s);
        suffix->is_static = true;
    }

    if (is_type_qual(s->it->type)) {
        suffix->type_quals = parse_type_qual_list(s);
        if (!is_valid_type_quals(&suffix->type_quals)) {
            return false;
        }

        if (s->it->type == ASTERISK) {
            if (suffix->is_static) {
                set_parser_err(s->err,
                               PARSER_ERR_ARR_STATIC_ASTERISK,
                               s->it->loc);
                free_arr_suffix(suffix);
                return false;
            }
            accept_it(s);
            suffix->is_asterisk = true;
            if (!accept(s, RINDEX)) {
                free_arr_suffix(suffix);
                return false;
            }
            return true;
        }
    }

    if (s->it->type == STATIC) {
        if (suffix->is_static) { 
            set_parser_err(s->err,
                           PARSER_ERR_ARR_DOUBLE_STATIC,
                           s->it->loc);
            free_arr_suffix(suffix);
            return false;
        }
        suffix->is_static = true;
        accept_it(s);
    }

    if (s->it->type == RINDEX) {
        if (suffix->is_static) {
            set_parser_err(s->err,
                           PARSER_ERR_ARR_STATIC_NO_LEN,
                           s->it->loc);
            free_arr_suffix(suffix);
            return false;
        }
        accept_it(s);
    } else {
        suffix->arr_len = parse_assign_expr(s);
        if (!(suffix->arr_len && accept(s, RINDEX))) {
            free_arr_suffix(suffix);
            return false;
        }
    }

    return true;
}

static bool parse_func_suffix(struct parser_state* s,
                              struct arr_or_func_suffix* res) {
    assert(s->it->type == LBRACKET);

    accept_it(s);
    if (s->it->type == IDENTIFIER
        && !is_typedef_name(s, &s->it->spelling)) {
        res->type = ARR_OR_FUNC_FUN_OLD_PARAMS;
        res->fun_params = parse_identifier_list(s);
        if (res->fun_params.len == 0) {
            return false;
        }

        if (!accept(s, RBRACKET)) {
            free_identifier_list(&res->fun_params);
            return false;
        }
    } else if (s->it->type == RBRACKET) {
        accept_it(s);
        res->type = ARR_OR_FUNC_FUN_EMPTY;
    } else {
        res->type = ARR_OR_FUNC_FUN_PARAMS;
        res->fun_types = parse_param_type_list(s);
        if (res->fun_types.param_list == NULL) {
            return false;
        }

        if (!accept(s, RBRACKET)) {
            free_param_type_list(&res->fun_types);
            return false;
        }
    }
    return true;
}

static bool parse_arr_or_func_suffix(struct parser_state* s,
                                     struct arr_or_func_suffix* res) {
    assert(res);
    assert(s->it->type == LINDEX || s->it->type == LBRACKET);
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->type) {
        case LINDEX:
            return parse_arr_suffix(s, res);

        case LBRACKET:
            return parse_func_suffix(s, res);

        default:
            UNREACHABLE();
    }
}

static struct direct_declarator* parse_direct_declarator_base(
    struct parser_state* s,
    struct declarator* (*parse_func)(struct parser_state*),
    bool (*identifier_handler)(struct parser_state*, const struct token*)) {
    struct direct_declarator* res = xmalloc(sizeof(struct direct_declarator));
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == LBRACKET) {
        accept_it(s);
        res->is_id = false;
        res->decl = parse_func(s);
        if (!res->decl) {
            free(res);
            return NULL;
        }

        if (!accept(s, RBRACKET)) {
            free_declarator(res->decl);
            free(res);
            return NULL;
        }
    } else if (s->it->type == IDENTIFIER) {
        res->is_id = true;
        if (!identifier_handler(s, s->it)) {
            return NULL;
        }
        const struct str spelling = take_spelling(s->it);
        struct source_loc loc = s->it->loc;
        accept_it(s);
        res->id = create_identifier(&spelling, loc);
    } else {
        free(res);
        enum token_type expected[] = {LBRACKET, IDENTIFIER};
        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
        return NULL;
    }

    res->suffixes = NULL;
    res->len = 0;
    size_t alloc_len = res->len;
    while (s->it->type == LBRACKET || s->it->type == LINDEX) {
        if (alloc_len == res->len) {
            grow_alloc((void**)&res->suffixes,
                       &alloc_len,
                       sizeof(struct arr_or_func_suffix));
        }

        if (!parse_arr_or_func_suffix(s, &res->suffixes[res->len])) {
            free_direct_declarator(res);
            return NULL;
        }

        ++res->len;
    }

    res->suffixes = xrealloc(res->suffixes,
                             sizeof(struct arr_or_func_suffix) * res->len);

    return res;
}

static bool empty_id_handler(struct parser_state* s, const struct token* token) {
    return true;
}

struct direct_declarator* parse_direct_declarator(struct parser_state* s) {
    return parse_direct_declarator_base(s, parse_declarator, empty_id_handler);
}

struct direct_declarator* parse_direct_declarator_typedef(
    struct parser_state* s) {
    return parse_direct_declarator_base(s,
                                        parse_declarator_typedef,
                                        register_typedef_name);
}

static void free_direct_declarator_children(struct direct_declarator* d) {
    if (d->is_id) {
        free_identifier(d->id);
    } else {
        free_declarator(d->decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        struct arr_or_func_suffix* item = &d->suffixes[i];
        switch (item->type) {
            case ARR_OR_FUNC_ARRAY:
                free_arr_suffix(&item->arr_suffix);
                break;
            case ARR_OR_FUNC_FUN_PARAMS:
                free_param_type_list(&item->fun_types);
                break;
            case ARR_OR_FUNC_FUN_OLD_PARAMS:
                free_identifier_list(&item->fun_params);
                break;
            case ARR_OR_FUNC_FUN_EMPTY:
                break;
            default:
                UNREACHABLE();
        }
    }
    free(d->suffixes);
}

void free_direct_declarator(struct direct_declarator* d) {
    free_direct_declarator_children(d);
    free(d);
}

bool parse_enumerator_inplace(struct parser_state* s, struct enumerator* res) {
    assert(res);

    if (s->it->type != IDENTIFIER) {
        expected_token_error(s, IDENTIFIER);
        return false;
    }

    struct token* id_token = s->it;
    accept_it(s);

    if (!register_enum_constant(s, id_token)) {
        return false;
    }

    const struct str spell = take_spelling(id_token);
    struct source_loc loc = id_token->loc;

    struct const_expr* enum_val = NULL;
    if (s->it->type == ASSIGN) {
        accept_it(s);
        enum_val = parse_const_expr(s);
        if (!enum_val) {
            free_str(&spell);
            return false;
        }
    }
    
    res->identifier = create_identifier(&spell, loc);
    res->enum_val = enum_val;

    return true;
}

void free_enumerator_children(struct enumerator* e) {
    free_identifier(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}

struct enum_list parse_enum_list(struct parser_state* s) {
    struct enum_list res = {
        .len = 1,
        .enums = xmalloc(sizeof(struct enumerator)),
    };
    if (!parse_enumerator_inplace(s, &res.enums[0])) {
        free(res.enums);
        return (struct enum_list){.len = 0, .enums = NULL};
    }

    size_t alloc_len = 1;
    while (s->it->type == COMMA && s->it[1].type == IDENTIFIER) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.enums,
                       &alloc_len,
                       sizeof(struct enumerator));
        }

        if (!parse_enumerator_inplace(s, &res.enums[res.len])) {
            goto fail;
        }

        ++res.len;
    }

    res.enums = xrealloc(res.enums, res.len * sizeof(struct enumerator));

    return res;
fail:
    free_enum_list(&res);
    return (struct enum_list){.len = 0, .enums = NULL};
}

void free_enum_list(struct enum_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_enumerator_children(&l->enums[i]);
    }
    free(l->enums);
}

static struct enum_spec* create_enum_spec(struct source_loc loc,
                                          struct identifier* identifier,
                                          struct enum_list enum_list) {
    assert(identifier || enum_list.len > 0);
    struct enum_spec* res = xmalloc(sizeof(struct enum_spec));
    res->info = create_ast_node_info(loc);
    res->identifier = identifier;
    res->enum_list = enum_list;

    return res;
}

struct enum_spec* parse_enum_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, ENUM)) {
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->type == IDENTIFIER) {
        const struct str spell = take_spelling(s->it);
        const struct source_loc id_loc = s->it->loc;
        accept_it(s);
        id = create_identifier(&spell, id_loc);
    }

    struct enum_list enums = {.len = 0, .enums = NULL};
    if (s->it->type == LBRACE) {
        accept_it(s);
        enums = parse_enum_list(s);
        if (enums.len == 0) {
            goto fail;
        }
        if (s->it->type == COMMA) {
            accept_it(s);
        }
        if (!accept(s, RBRACE)) {
            free_enum_list(&enums);
            goto fail;
        }
    } else if (id == NULL) {
        expected_token_error(s, LBRACE);
        goto fail;
    }

    return create_enum_spec(loc, id, enums);
fail:
    if (id) {
        free_identifier(id);
    }
    return NULL;
}

static void free_enum_spec_children(struct enum_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_enum_list(&s->enum_list);
}

void free_enum_spec(struct enum_spec* s) {
    free_enum_spec_children(s);
    free(s);
}

static bool parse_external_decl_normal_decl(struct parser_state* s,
                                            struct external_declaration* res,
                                            struct declaration_specs* decl_specs,
                                            struct declarator* first_decl,
                                            bool found_typedef) {
    res->is_func_def = false;

    struct declaration* decl = &res->decl;

    decl->is_normal_decl = true;
    decl->decl_specs = decl_specs;

    struct initializer* init = NULL;
    if (s->it->type == ASSIGN) {
        if (found_typedef) {
            set_parser_err(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
        accept_it(s);
        init = parse_initializer(s);
        if (!init) {
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
    }

    struct init_declarator* init_decl = xmalloc(
        sizeof(struct init_declarator));
    init_decl->decl = first_decl;
    init_decl->init = init;

    if (found_typedef) {
        decl->init_decls = parse_init_declarator_list_typedef_first(
            s,
            init_decl);
    } else {
        decl->init_decls = parse_init_declarator_list_first(s, init_decl);
    }
    if (decl->init_decls.len == 0) {
        free_init_declarator_children(init_decl);
        free(init_decl);
        free_declaration_children(decl);
        return false;
    }

    if (!accept(s, SEMICOLON)) {
        free_declaration_children(decl);
        return false;
    }
    return true;
}

static bool parse_external_declaration_func_def(struct parser_state* s,
                                                struct external_declaration* res,
                                                struct declaration_specs* decl_specs,
                                                struct declarator* first_decl,
                                                bool found_typedef) {
    if (found_typedef) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_FUNC_DEF, s->it->loc);
        free_declaration_specs(decl_specs);
        free_declarator(first_decl);
        return false;
    }
    res->is_func_def = true;

    struct func_def* func_def = &res->func_def;

    func_def->specs = decl_specs;
    func_def->decl = first_decl;
    if (s->it->type != LBRACE) {
        func_def->decl_list = parse_declaration_list(s);
        if (func_def->decl_list.len == 0) {
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
    } else {
        func_def->decl_list = (struct declaration_list){
            .len = 0,
            .decls = NULL,
        };
    }

    func_def->comp = parse_compound_statement(s);
    if (!func_def->comp) {
        free_declaration_specs(decl_specs);
        free_declarator(first_decl);
        free_declaration_list(&func_def->decl_list);
        return false;
    }
    return true;
}

bool parse_external_declaration_inplace(struct parser_state* s,
                                        struct external_declaration* res) {
    assert(res);

    if (s->it->type == STATIC_ASSERT) {
        res->is_func_def = false;
        res->decl.is_normal_decl = false;
        res->decl.static_assert_decl = parse_static_assert_declaration(s);
        if (!res->decl.static_assert_decl) {
            return false;
        }
        return true;
    }
    bool found_typedef = false;
    struct declaration_specs* decl_specs = parse_declaration_specs(
        s,
        &found_typedef);
    if (!decl_specs) {
        return false;
    }

    if (s->it->type == SEMICOLON) {
        accept_it(s);
        res->is_func_def = false;
        res->decl.is_normal_decl = true;
        res->decl.decl_specs = decl_specs;
        res->decl.init_decls = (struct init_declarator_list){
            .len = 0,
            .decls = NULL,
        };

        return true;
    }

    struct declarator* first_decl;
    if (found_typedef) {
        first_decl = parse_declarator_typedef(s);
    } else {
        first_decl = parse_declarator(s);
    }

    if (!first_decl) {
        free_declaration_specs(decl_specs);
        return false;
    }

    if (s->it->type == ASSIGN || s->it->type == COMMA || s->it->type == SEMICOLON) {
        return parse_external_decl_normal_decl(s,
                                               res,
                                               decl_specs,
                                               first_decl,
                                               found_typedef);
    } else {
       return parse_external_declaration_func_def(s,
                                                  res,
                                                  decl_specs,
                                                  first_decl,
                                                  found_typedef);
    }
}

void free_external_declaration_children(struct external_declaration* d) {
    if (d->is_func_def) {
        free_func_def_children(&d->func_def);
    } else {
        free_declaration_children(&d->decl);
    }
}

void free_func_def_children(struct func_def* d) {
    free_declaration_specs(d->specs);
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}

struct identifier_list parse_identifier_list(struct parser_state* s) {
    if (s->it->type != IDENTIFIER) {
        return (struct identifier_list){.len = 0, .identifiers = NULL};
    }
    struct identifier_list res = {
        .len = 1,
        .identifiers = xmalloc(sizeof(struct identifier)),
    };
    struct str spell = take_spelling(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);
    init_identifier(res.identifiers, &spell, loc);

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.identifiers,
                       &alloc_len,
                       sizeof(struct identifier));
        }

        if (s->it->type != IDENTIFIER) {
            free_identifier_list(&res);
            return (struct identifier_list){.len = 0, .identifiers = NULL};
        }
        spell = take_spelling(s->it);
        loc = s->it->loc; 
        accept_it(s);
        init_identifier(&res.identifiers[res.len], &spell, loc);

        ++res.len;
    }

    res.identifiers = xrealloc(res.identifiers,
                               sizeof(struct identifier) * res.len);

    return res;
}

void free_identifier_list(struct identifier_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_identifier_children(&l->identifiers[i]);
    }
    free(l->identifiers);
}

bool parse_init_declarator_typedef_inplace(struct parser_state* s,
                                           struct init_declarator* res) {
    res->decl = parse_declarator_typedef(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->type == ASSIGN) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
        return false;
    }

    res->init = NULL;

    return true;
}

bool parse_init_declarator_inplace(struct parser_state* s,
                                   struct init_declarator* res) {
    res->decl = parse_declarator(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->type == ASSIGN) {
        accept_it(s);
        res->init = parse_initializer(s);
        if (!res->init) {
            free_declarator(res->decl);
            return false;
        }
    } else {
        res->init = NULL;
    }
    return true;
}

void free_init_declarator_children(struct init_declarator* d) {
    free_declarator(d->decl);
    if (d->init) {
        free_initializer(d->init);
    }
}

static struct init_declarator_list parse_init_declarator_list_first_base(
    struct parser_state* s,
    bool (*inplace_parse_func)(struct parser_state*, struct init_declarator*),
    struct init_declarator* first_decl) {
    assert(first_decl);
    struct init_declarator_list res = {
        .len = 1,
        .decls = first_decl,
    };

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls,
                       &alloc_len,
                       sizeof(struct init_declarator));
        }

        if (!inplace_parse_func(s, &res.decls[res.len])) {
            free_init_declarator_list(&res);
            return (struct init_declarator_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls, sizeof(struct init_declarator) * res.len);

    return res;
}

static struct init_declarator_list parse_init_declarator_list_base(
    struct parser_state* s,
    bool (*inplace_parse_func)(struct parser_state*, struct init_declarator*)) {
    struct init_declarator* first_decl = xmalloc(
        sizeof(struct init_declarator));

    if (!inplace_parse_func(s, first_decl)) {
        free(first_decl);
        return (struct init_declarator_list){.len = 0, .decls = NULL};
    }

    return parse_init_declarator_list_first_base(s,
                                                 inplace_parse_func,
                                                 first_decl);
}

struct init_declarator_list parse_init_declarator_list_first(
    struct parser_state* s,
    struct init_declarator* first_decl) {
    return parse_init_declarator_list_first_base(s,
                                                 parse_init_declarator_inplace,
                                                 first_decl);
}

struct init_declarator_list parse_init_declarator_list(struct parser_state* s) {
    return parse_init_declarator_list_base(s, parse_init_declarator_inplace);
}

struct init_declarator_list parse_init_declarator_list_typedef_first(
    struct parser_state* s,
    struct init_declarator* first_decl) {
    return parse_init_declarator_list_first_base(
        s,
        parse_init_declarator_typedef_inplace,
        first_decl);
}

struct init_declarator_list parse_init_declarator_list_typedef(
    struct parser_state* s) {
    return parse_init_declarator_list_base(
        s,
        parse_init_declarator_typedef_inplace);
}

void free_init_declarator_list(struct init_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

// There might be a better way to do this
static bool is_declarator(const struct token* current) {
    const struct token* it = current;
    while (it->type == ASTERISK) {
        ++it;

        while (is_type_qual(it->type)) {
            ++it;
        }
    }

    if (it->type == IDENTIFIER) {
        return true;
    } else if (it->type == LBRACKET) {
        ++it;
        return is_declarator(it);
    } else {
        return false;
    }
}

bool parse_param_declaration_inplace(struct parser_state* s,
                                     struct param_declaration* res) {
    assert(res);

    bool found_typedef = false;
    res->decl_specs = parse_declaration_specs(s, &found_typedef);
    if (!res->decl_specs) {
        return false;
    }

    if (found_typedef) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_PARAM_DECL, s->it->loc);
        free_declaration_specs(res->decl_specs);
        return false;
    }

    if (s->it->type == COMMA || s->it->type == RBRACKET) {
        res->type = PARAM_DECL_NONE;
        res->decl = NULL;
    } else if (is_declarator(s->it)) {
        res->type = PARAM_DECL_DECL;
        res->decl = parse_declarator(s);
        if (!res->decl) {
            free_declaration_specs(res->decl_specs);
            return false;
        }
    } else {
        res->type = PARAM_DECL_ABSTRACT_DECL;
        res->abstract_decl = parse_abs_declarator(s);
        if (!res->abstract_decl) {
            free_declaration_specs(res->decl_specs);
            return false;
        }
    }

    return true;
}

void free_param_declaration_children(struct param_declaration* d) {
    free_declaration_specs(d->decl_specs);
    switch (d->type) {
        case PARAM_DECL_DECL:
            free_declarator(d->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            free_abs_declarator(d->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            break;
    }
}

struct param_list* parse_param_list(struct parser_state* s) {
    struct param_list* res = xmalloc(sizeof(struct param_list));
    res->decls = xmalloc(sizeof(struct param_declaration));
    res->len = 1;

    if (!parse_param_declaration_inplace(s, &res->decls[0])) {
        free(res->decls);
        free(res);
        return NULL;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA && s->it[1].type != ELLIPSIS) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->decls,
                       &alloc_len,
                       sizeof(struct param_declaration));
        }

        if (!parse_param_declaration_inplace(s, &res->decls[res->len])) {
            free_param_list(res);
            return NULL;
        }

        ++res->len;
    }

    res->decls = xrealloc(res->decls,
                          sizeof(struct param_declaration) * res->len);

    return res;
}

static void free_param_list_children(struct param_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_param_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_param_list(struct param_list* l) {
    free_param_list_children(l);
    free(l);
}

static struct param_type_list create_param_type_list(
    bool is_variadic,
    struct param_list* param_list) {
    assert(param_list);
    return (struct param_type_list){.is_variadic = is_variadic,
                                    .param_list = param_list};
}

struct param_type_list parse_param_type_list(struct parser_state* s) {
    struct param_list* param_list = parse_param_list(s);
    if (!param_list) {
        return (struct param_type_list){.is_variadic = false,
                                        .param_list = NULL};
    }

    bool is_variadic = false;
    if (s->it->type == COMMA) {
        accept_it(s);
        if (!accept(s, ELLIPSIS)) {
            free_param_list(param_list);
            return (struct param_type_list){.is_variadic = false,
                                            .param_list = NULL};
        }
        is_variadic = true;
    }

    return create_param_type_list(is_variadic, param_list);
}

void free_param_type_list(struct param_type_list* l) {
    free_param_list(l->param_list);
}

struct pointer* parse_pointer(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, ASTERISK)) {
        return NULL;
    }

    struct pointer* res = xmalloc(sizeof(struct pointer));
    res->info = create_ast_node_info(loc);
    res->num_indirs = 1;
    res->quals_after_ptr = xmalloc(sizeof(struct type_quals));

    if (is_type_qual(s->it->type)) {
        res->quals_after_ptr[0] = parse_type_qual_list(s);
        if (!is_valid_type_quals(&res->quals_after_ptr[0])) {
            free(res->quals_after_ptr);
            free(res);
            return NULL;
        }
    } else {
        res->quals_after_ptr[0] = create_type_quals();
    }

    size_t alloc_size = res->num_indirs;
    while (s->it->type == ASTERISK) {
        accept_it(s);

        if (res->num_indirs == alloc_size) {
            grow_alloc((void**)&res->quals_after_ptr,
                       &alloc_size,
                       sizeof(struct type_quals));
        }

        if (is_type_qual(s->it->type)) {
            res->quals_after_ptr[res->num_indirs] = parse_type_qual_list(s);
            if (!is_valid_type_quals(&res->quals_after_ptr[res->num_indirs])) {
                free_pointer(res);
                return NULL;
            }
        } else {
            res->quals_after_ptr[res->num_indirs] = create_type_quals();
        }

        ++res->num_indirs;
    }

    res->quals_after_ptr = xrealloc(res->quals_after_ptr,
                                    sizeof(struct type_quals)
                                        * res->num_indirs);

    return res;
}

static void free_pointer_children(struct pointer* p) {
    free(p->quals_after_ptr);
}

void free_pointer(struct pointer* p) {
    free_pointer_children(p);
    free(p);
}

struct static_assert_declaration* parse_static_assert_declaration(
    struct parser_state* s) {
    if (!(accept(s, STATIC_ASSERT) && accept(s, LBRACKET))) {
        return NULL;
    }

    struct const_expr* const_expr = parse_const_expr(s);
    if (!const_expr) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        free_const_expr(const_expr);
        return NULL;
    }

    if (s->it->type != STRING_LITERAL) {
        expected_token_error(s, STRING_LITERAL);
        free_const_expr(const_expr);
        return NULL;
    }

    const struct str spell = take_spelling(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);

    if (!(accept(s, RBRACKET) && accept(s, SEMICOLON))) {
        free_const_expr(const_expr);
        free_str(&spell);
        return NULL;
    }

    struct static_assert_declaration* res = xmalloc(
        sizeof(struct static_assert_declaration));
    res->const_expr = const_expr;
    res->err_msg = create_string_literal(&spell, loc); 

    return res;
}

static void free_static_assert_declaration_children(struct static_assert_declaration* d) {
    free_const_expr(d->const_expr);
    free_string_literal(&d->err_msg);
}

void free_static_assert_declaration(struct static_assert_declaration* d) {
    free_static_assert_declaration_children(d);

    free(d);
}

bool parse_struct_declaration_inplace(struct parser_state* s,
                                      struct struct_declaration* res) {
    if (s->it->type == STATIC_ASSERT) {
        res->is_static_assert = true;
        res->assert = parse_static_assert_declaration(s);
        if (!res->assert) {
            return false;
        }
    } else {
        res->is_static_assert = false;
        bool found_typedef = false;
        res->decl_specs = parse_declaration_specs(s, &found_typedef);
        if (!res->decl_specs) {
            return false;
        }

        if (found_typedef) {
            set_parser_err(s->err, PARSER_ERR_TYPEDEF_STRUCT, s->it->loc);
        }

        if (s->it->type != SEMICOLON) {
            res->decls = parse_struct_declarator_list(s);
            if (res->decls.len == 0) {
                free_declaration_specs(res->decl_specs);
                return false;
            }
        } else {
            res->decls = (struct struct_declarator_list){
                .len = 0,
                .decls = NULL,
            };
        }

        if (!accept(s, SEMICOLON)) {
            free_struct_declaration_children(res);
            return false;
        }
    }

    return true;
}

void free_struct_declaration_children(struct struct_declaration* d) {
    if (d->is_static_assert) {
        free_static_assert_declaration(d->assert);
    } else {
        free_declaration_specs(d->decl_specs);
        free_struct_declarator_list(&d->decls);
    }
}

struct struct_declaration_list parse_struct_declaration_list(
    struct parser_state* s) {
    struct struct_declaration_list res = {
        .len = 1,
        .decls = xmalloc(sizeof(struct struct_declaration)),
    };

    if (!parse_struct_declaration_inplace(s, &res.decls[0])) {
        free(res.decls);
        return (struct struct_declaration_list){.len = 0, .decls = NULL};
    }

    size_t alloc_len = res.len;
    while (is_declaration(s) || s->it->type == STATIC_ASSERT) {
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls,
                       &alloc_len,
                       sizeof(struct struct_declaration));
        }

        if (!parse_struct_declaration_inplace(s, &res.decls[res.len])) {
            free_struct_declaration_list(&res);
            return (struct struct_declaration_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls,
                         sizeof(struct struct_declaration) * res.len);

    return res;
}

void free_struct_declaration_list(struct struct_declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

bool parse_struct_declarator_inplace(struct parser_state* s,
                                     struct struct_declarator* res) {
    assert(res);

    if (s->it->type != COLON) {
        res->decl = parse_declarator(s);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    if (s->it->type == COLON) {
        accept_it(s);
        res->bit_field = parse_const_expr(s);
        if (!res->bit_field) {
            free_struct_declarator_children(res);
            return false;
        }
    } else {
        res->bit_field = NULL;
        if (!res->decl) {
            set_parser_err(s->err,
                           PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
                           s->it->loc);
            return false;
        }
    }

    return true;
}

void free_struct_declarator_children(struct struct_declarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

struct struct_declarator_list parse_struct_declarator_list(
    struct parser_state* s) {
    struct struct_declarator_list res = {
        .len = 1,
        .decls = xmalloc(sizeof *res.decls),
    };

    if (!parse_struct_declarator_inplace(s, &res.decls[0])) {
        free(res.decls);
        return (struct struct_declarator_list){.len = 0, .decls = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls,
                       &alloc_len,
                       sizeof *res.decls);
        }

        if (!parse_struct_declarator_inplace(s, &res.decls[res.len])) {
            free_struct_declarator_list(&res);
            return (struct struct_declarator_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls, sizeof *res.decls * res.len);

    return res;
}

void free_struct_declarator_list(struct struct_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

static struct struct_union_spec* create_struct_union(
    struct source_loc loc,
    bool is_struct,
    struct identifier* identifier,
    struct struct_declaration_list decl_list) {
    struct struct_union_spec* res = xmalloc(sizeof(struct struct_union_spec));
    res->info = create_ast_node_info(loc);
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;

    return res;
}

struct struct_union_spec* parse_struct_union_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    bool is_struct;
    if (s->it->type == STRUCT) {
        is_struct = true;
        accept_it(s);
    } else if (s->it->type == UNION) {
        is_struct = false;
        accept_it(s);
    } else {
        enum token_type expected[] = {STRUCT, UNION};
        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->type == IDENTIFIER) {
        const struct str spell = take_spelling(s->it);
        const struct source_loc id_loc = s->it->loc;
        accept_it(s);
        id = create_identifier(&spell, id_loc);
    }

    struct struct_declaration_list list = {.len = 0, .decls = NULL};
    if (s->it->type == LBRACE) {
        accept_it(s);
        list = parse_struct_declaration_list(s);
        if (list.len == 0) {
            goto fail;
        }

        if (!accept(s, RBRACE)) {
            free_struct_declaration_list(&list);
            goto fail;
        }
    }
    return create_struct_union(loc, is_struct, id, list);

fail:
    if (id) {
        free_identifier(id);
    }
    return NULL;
}

static void free_struct_union_spec_children(struct struct_union_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_struct_declaration_list(&s->decl_list);
}

void free_struct_union_spec(struct struct_union_spec* s) {
    free_struct_union_spec_children(s);
    free(s);
}

struct type_quals create_type_quals(void) {
    return (struct type_quals){
        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_atomic = false,
    };
}

void update_type_quals(struct parser_state* s, struct type_quals* quals) {
    assert(is_type_qual(s->it->type));

    switch (s->it->type) {
        case CONST:
            quals->is_const = true;
            break;
        case RESTRICT:
            quals->is_restrict = true;
            break;
        case VOLATILE:
            quals->is_volatile = true;
            break;
        case ATOMIC:
            quals->is_atomic = true;
            break;
        default:
            UNREACHABLE();
    }
    accept_it(s);
}

struct type_quals parse_type_qual_list(struct parser_state* s) {
    struct type_quals res = create_type_quals();

    if (!is_type_qual(s->it->type)) {
        enum token_type expected[] = {CONST, RESTRICT, VOLATILE, ATOMIC};

        expected_tokens_error(s,
                              expected,
                              sizeof expected / sizeof(enum token_type));
        return create_type_quals();
    }

    while (is_type_qual(s->it->type)) {
        update_type_quals(s, &res);
    }

    return res;
}

bool is_valid_type_quals(const struct type_quals* q) {
    return q->is_const || q->is_volatile || q->is_restrict || q->is_atomic;
}

static inline bool is_standalone_type_spec(enum token_type t) {
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
            return true;
        default:
            return false;
    }
}

struct type_specs create_type_specs(void) {
    return (struct type_specs){
        .mods =
            {
                .is_unsigned = false,
                .is_signed = false,
                .is_short = false,
                .num_long = 0,
                .is_complex = false,
                .is_imaginary = false,
            },
        .type = TYPE_SPEC_NONE,
    };
}

static void cannot_combine_with_spec_err(const struct parser_state* s,
                                         enum token_type prev_spec) {
    set_parser_err(s->err, PARSER_ERR_INCOMPATIBLE_TYPE_SPECS, s->it->loc);
    s->err->type_spec = s->it->type;
    s->err->prev_type_spec = prev_spec;
}

static enum type_spec_type get_predef_type_spec(enum token_type t) {
    switch (t) {
        case VOID:
            return TYPE_SPEC_VOID;
        case CHAR:
            return TYPE_SPEC_CHAR;
        case INT:
            return TYPE_SPEC_INT;
        case FLOAT:
            return TYPE_SPEC_FLOAT;
        case DOUBLE:
            return TYPE_SPEC_DOUBLE;
        case BOOL:
            return TYPE_SPEC_BOOL;

        default:
            UNREACHABLE();
    }
}

static bool update_standalone_type_spec(struct parser_state* s,
                                        struct type_specs* res) {
    switch (s->it->type) {
        case VOID:
        case CHAR:
        case INT:
        case FLOAT:
        case DOUBLE:
        case BOOL:
            if (res->type != TYPE_SPEC_NONE) {
                set_parser_err(s->err,
                               PARSER_ERR_DISALLOWED_TYPE_QUALS,
                               s->it->loc);
                s->err->incompatible_type = s->it->type;
                free_type_specs_children(res);
                return false;
            }
            res->type = get_predef_type_spec(s->it->type);
            break;
        case SHORT:
            if (res->mods.num_long != 0) {
                cannot_combine_with_spec_err(s, LONG);
                free_type_specs_children(res);
                return false;
            }

            res->mods.is_short = true;
            break;
        case LONG:
            if (res->mods.is_short) {
                cannot_combine_with_spec_err(s, SHORT);
                free_type_specs_children(res);
                return false;
            }
            res->mods.num_long += 1;
            break;
        case SIGNED:
            if (res->mods.is_unsigned) {
                cannot_combine_with_spec_err(s, UNSIGNED);
                free_type_specs_children(res);
                return false;
            }
            res->mods.is_signed = true;
            break;
        case UNSIGNED:
            if (res->mods.is_signed) {
                cannot_combine_with_spec_err(s, SIGNED);
                free_type_specs_children(res);
                return false;
            }
            res->mods.is_unsigned = true;
            break;
        case COMPLEX:
            res->mods.is_complex = true;
            break;
        case IMAGINARY:
            res->mods.is_imaginary = true;
            break;

        default:
            UNREACHABLE();
    }
    accept_it(s);
    return true;
}

static bool update_non_standalone_type_spec(struct parser_state* s,
                                            struct type_specs* res) {
    switch (s->it->type) {
        case ATOMIC: {
            res->type = TYPE_SPEC_ATOMIC;
            res->atomic_spec = parse_atomic_type_spec(s);
            if (!res->atomic_spec) {
                return false;
            }
            break;
        }
        case STRUCT:
        case UNION: {
            res->type = TYPE_SPEC_STRUCT;
            res->struct_union_spec = parse_struct_union_spec(s);
            if (!res->struct_union_spec) {
                return false;
            }
            break;
        }
        case ENUM: {
            res->type = TYPE_SPEC_ENUM;
            res->enum_spec = parse_enum_spec(s);
            if (!res->enum_spec) {
                return false;
            }
            break;
        }
        case IDENTIFIER: {
            if (is_typedef_name(s, &s->it->spelling)) {
                res->type = TYPE_SPEC_TYPENAME;
                const struct str spell = take_spelling(s->it);
                res->typedef_name = create_identifier(&spell,
                                                      s->it->loc);
                accept_it(s);
                break;
            } else {
                set_parser_err(s->err,
                               PARSER_ERR_EXPECTED_TYPEDEF_NAME,
                               s->it->loc);
                s->err->non_typedef_spelling = take_spelling(s->it);
                return false;
            }
        }
        default: {
            enum token_type expected[] = {
                ATOMIC,
                STRUCT,
                UNION,
                ENUM,
                TYPEDEF_NAME,
            };
            expected_tokens_error(s,
                                  expected,
                                  sizeof(expected) / sizeof(enum token_type));
            return false;
        }
    }

    return true;
}

bool update_type_specs(struct parser_state* s, struct type_specs* res) {
    assert(res);

    if (is_standalone_type_spec(s->it->type)) {
        return update_standalone_type_spec(s, res);
    } else {
        return update_non_standalone_type_spec(s, res);
    }
}

void free_type_specs_children(struct type_specs* s) {
    switch (s->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            break;
        case TYPE_SPEC_ATOMIC:
            free_atomic_type_spec(s->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            free_struct_union_spec(s->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            free_enum_spec(s->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            free_identifier(s->typedef_name);
            break;
    }
}

bool is_valid_type_specs(const struct type_specs* s) {
    assert(s);

    if (s->type == TYPE_SPEC_NONE) {
        const struct type_modifiers* mods = &s->mods;
        return mods->is_unsigned || mods->is_signed || mods->is_short
               || mods->num_long != 0 || mods->is_complex || mods->is_imaginary;
    }

    return true;
}

static bool parse_add_expr_add_chain(struct parser_state* s,
                                     struct add_expr* res) {
    res->len = 0;
    res->add_chain = NULL;

    size_t alloc_len = res->len;
    while (is_add_op(s->it->type)) {
        const enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->add_chain,
                       &alloc_len,
                       sizeof(struct mul_expr_and_op));
        }

        struct mul_expr_and_op* curr = &res->add_chain[res->len];
        curr->rhs = parse_mul_expr(s);
        if (!curr->rhs) {
            free_add_expr(res);
            return false;
        }

        curr->op = op == ADD ? ADD_EXPR_ADD : ADD_EXPR_SUB;

        ++res->len;
    }

    res->add_chain = xrealloc(res->add_chain,
                              sizeof(struct mul_expr_and_op) * res->len);

    return true;
}

struct add_expr* parse_add_expr(struct parser_state* s) {
    struct mul_expr* lhs = parse_mul_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct add_expr* parse_add_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct mul_expr* lhs = parse_mul_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_add_expr_children(struct add_expr* e) {
    free_mul_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_mul_expr(e->add_chain[i].rhs);
    }
    free(e->add_chain);
}

void free_add_expr(struct add_expr* e) {
    free_add_expr_children(e);
    free(e);
}

static bool parse_and_expr_rest(struct parser_state* s, struct and_expr* res) {
    assert(res->eq_exprs);

    res->len = 1;
    size_t alloc_len = res->len;

    while (s->it->type == AND) {
        accept_it(s);
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->eq_exprs,
                       &alloc_len,
                       sizeof(struct eq_expr));
        }
        if (!parse_eq_expr_inplace(s, &res->eq_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->eq_exprs = xrealloc(res->eq_exprs, sizeof(struct eq_expr) * res->len);

    return true;
fail:
    free_and_expr_children(res);
    return false;
}

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res) {
    res->eq_exprs = xmalloc(sizeof(struct eq_expr));
    if (!parse_eq_expr_inplace(s, res->eq_exprs)) {
        free(res->eq_exprs);
        return false;
    }

    if (!parse_and_expr_rest(s, res)) {
        return false;
    }
    return true;
}

struct and_expr* parse_and_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct eq_expr* eq_exprs = parse_eq_expr_cast(s, start);
    if (!eq_exprs) {
        return NULL;
    }

    struct and_expr* res = xmalloc(sizeof(struct and_expr));
    res->eq_exprs = eq_exprs;

    if (!parse_and_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void free_and_expr_children(struct and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    free(e->eq_exprs);
}

struct arg_expr_list parse_arg_expr_list(struct parser_state* s) {
    struct arg_expr_list res = {
        .len = 1,
        .assign_exprs = xmalloc(sizeof(struct assign_expr)),
    };
    if (!parse_assign_expr_inplace(s, &res.assign_exprs[0])) {
        free(res.assign_exprs);
        return (struct arg_expr_list){.len = 0, .assign_exprs = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.assign_exprs,
                       &alloc_len,
                       sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res.assign_exprs[res.len])) {
            free_arg_expr_list(&res);
            return (struct arg_expr_list){.assign_exprs = NULL, .len = 0};
        }

        ++res.len;
    }

    res.assign_exprs = xrealloc(res.assign_exprs,
                                sizeof(struct assign_expr) * res.len);
    return res;
}

void free_arg_expr_list(struct arg_expr_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}

struct unary_or_cond {
    bool is_cond;
    union {
        struct unary_expr* unary;
        struct cond_expr* cond;
    };
};

static struct unary_or_cond parse_unary_or_cond(struct parser_state* s) {
    struct unary_or_cond res = {
        .is_cond = false,
        .cond = NULL,
    };
    if (s->it->type == LBRACKET && next_is_type_name(s)) {
        const struct source_loc start_bracket_loc = s->it->loc;
        accept_it(s);

        struct type_name* type_name = parse_type_name(s);
        if (!type_name) {
            return res;
        }

        if (!accept(s, RBRACKET)) {
            free_type_name(type_name);
            return res;
        }

        if (s->it->type == LBRACE) {
            res.is_cond = false;
            res.unary = parse_unary_expr_type_name(s, NULL, 0, type_name, start_bracket_loc);
        } else {
            struct cast_expr* cast_expr = parse_cast_expr_type_name(s, type_name, start_bracket_loc);
            if (!cast_expr) {
                return res;
            }

            res.is_cond = true;
            res.cond = parse_cond_expr_cast(s, cast_expr);
        }
    } else {
        res.is_cond = false;
        res.unary = parse_unary_expr(s);
    }

    return res;
}

static enum assign_expr_op token_type_to_assign_op(enum token_type t) {
    assert(is_assign_op(t));
    switch (t) {
        case ASSIGN:
            return ASSIGN_EXPR_ASSIGN;
        case MUL_ASSIGN:
            return ASSIGN_EXPR_MUL;
        case DIV_ASSIGN:
            return ASSIGN_EXPR_DIV;
        case MOD_ASSIGN:
            return ASSIGN_EXPR_MOD;
        case ADD_ASSIGN:
            return ASSIGN_EXPR_ADD;
        case SUB_ASSIGN:
            return ASSIGN_EXPR_SUB;
        case LEFT_ASSIGN:
            return ASSIGN_EXPR_LSHIFT;
        case RIGHT_ASSIGN:
            return ASSIGN_EXPR_RSHIFT;
        case AND_ASSIGN:
            return ASSIGN_EXPR_AND;
        case XOR_ASSIGN:
            return ASSIGN_EXPR_XOR;
        case OR_ASSIGN:
            return ASSIGN_EXPR_OR;

        default:
            UNREACHABLE();
    }
}

bool parse_assign_expr_inplace(struct parser_state* s,
                               struct assign_expr* res) {
    assert(res);

    res->len = 0;
    res->assign_chain = NULL;
    res->value = NULL;

    struct unary_or_cond opt = parse_unary_or_cond(s);
    if (opt.unary == NULL) {
        return false;
    } else if (opt.is_cond) {
        res->value = opt.cond;
        return true;
    }

    struct unary_expr* last_unary = opt.unary;

    size_t alloc_len = res->len;
    while (is_assign_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        opt = parse_unary_or_cond(s);
        if (opt.unary == NULL) {
            free_unary_expr(last_unary);
            goto fail;
        } else if (opt.is_cond) {
            res->value = opt.cond;
            ++res->len;
            res->assign_chain = xrealloc(res->assign_chain,
                                         sizeof(struct cond_expr) * res->len);
            res->assign_chain[res->len - 1] = (struct unary_and_op){
                .op = token_type_to_assign_op(op),
                .unary = last_unary,
            };
            return true;
        }

        struct unary_expr* new_last = opt.unary;

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->assign_chain,
                       &alloc_len,
                       sizeof(struct unary_and_op));
        }

        res->assign_chain[res->len] = (struct unary_and_op){
            .op = token_type_to_assign_op(op),
            .unary = last_unary,
        };
        last_unary = new_last;

        ++res->len;
    }

    res->assign_chain = xrealloc(res->assign_chain,
                                 sizeof(struct unary_and_op) * res->len);

    res->value = parse_cond_expr_cast(s, create_cast_expr_unary(last_unary));
    if (!res->value) {
        goto fail;
    }

    return true;
fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_unary_expr(res->assign_chain[i].unary);
    }
    free(res->assign_chain);

    return false;
}

struct assign_expr* parse_assign_expr(struct parser_state* s) {
    struct assign_expr* res = xmalloc(sizeof(struct assign_expr));
    if (!parse_assign_expr_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_assign_expr_children(struct assign_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_unary_expr(e->assign_chain[i].unary);
    }
    free(e->assign_chain);

    free_cond_expr(e->value);
}

void free_assign_expr(struct assign_expr* e) {
    free_assign_expr_children(e);
    free(e);
}

static bool parse_cast_expr_rest(struct parser_state* s,
                                 struct cast_expr* res) {
    size_t alloc_len = res->len;
    struct source_loc last_lbracket_loc = {
        .file_idx = (size_t)-1,
        .file_loc = {0, 0},
    };
    while (s->it->type == LBRACKET && next_is_type_name(s)) {
        last_lbracket_loc = s->it->loc;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->type_names,
                       &alloc_len,
                       sizeof(struct type_name));
        }

        if (!parse_type_name_inplace(s, &res->type_names[res->len])) {
            goto fail;
        }

        if (!accept(s, RBRACKET)) {
            goto fail;
        }
        ++res->len;
    }

    if (s->it->type == LBRACE) {
        assert(res->len > 0);
        struct type_name* type_name = xmalloc(sizeof(struct type_name));

        --res->len;
        *type_name = res->type_names[res->len];

        if (res->type_names) {
            res->type_names = xrealloc(res->type_names,
                                       res->len * sizeof(struct type_name));
        }

        res->rhs = parse_unary_expr_type_name(s, NULL, 0, type_name, last_lbracket_loc);
    } else {
        if (res->type_names) {
            res->type_names = xrealloc(res->type_names,
                                       res->len * sizeof(struct type_name));
        }
        res->rhs = parse_unary_expr(s);
    }
    if (!res->rhs) {
        goto fail;
    }

    return true;

fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_type_name_children(&res->type_names[i]);
    }
    free(res->type_names);
    return false;
}

struct cast_expr* parse_cast_expr(struct parser_state* s) {
    struct cast_expr* res = xmalloc(sizeof(struct cast_expr));
    res->info = create_ast_node_info(s->it->loc);
    res->type_names = NULL;
    res->len = 0;

    if (!parse_cast_expr_rest(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

struct cast_expr* parse_cast_expr_type_name(struct parser_state* s,
                                            struct type_name* type_name,
                                            struct source_loc start_bracket_loc) {
    assert(type_name);

    struct cast_expr* res = xmalloc(sizeof(struct cast_expr));
    res->info = create_ast_node_info(start_bracket_loc);
    res->type_names = type_name;
    res->len = 1;

    if (!parse_cast_expr_rest(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

struct cast_expr* create_cast_expr_unary(struct unary_expr* start) {
    assert(start);

    struct cast_expr* res = xmalloc(sizeof(struct cast_expr));
    res->info = create_ast_node_info(start->info.loc);
    res->type_names = NULL;
    res->len = 0;
    res->rhs = start;
    return res;
}

static void free_cast_expr_children(struct cast_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_type_name_children(&e->type_names[i]);
    }
    free(e->type_names);
    free_unary_expr(e->rhs);
}

void free_cast_expr(struct cast_expr* e) {
    free_cast_expr_children(e);
    free(e);
}

static bool parse_cond_expr_conditionals(struct parser_state* s,
                                         struct cond_expr* res) {
    res->len = 0;
    res->conditionals = NULL;

    size_t alloc_len = res->len;
    while (s->it->type == QMARK) {
        accept_it(s);

        struct expr* expr = parse_expr(s);
        if (!expr) {
            goto fail;
        }

        if (!accept(s, COLON)) {
            free_expr(expr);
            goto fail;
        }

        struct log_or_expr* new_last = parse_log_or_expr(s);
        if (!new_last) {
            free_expr(expr);
            goto fail;
        }

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->conditionals,
                       &alloc_len,
                       sizeof(struct log_or_and_expr));
        }

        res->conditionals[res->len] = (struct log_or_and_expr){
            .log_or = res->last_else,
            .expr = expr,
        };

        res->last_else = new_last;

        ++res->len;
    }

    res->conditionals = xrealloc(res->conditionals,
                                 sizeof(struct log_or_and_expr) * res->len);
    return true;

fail:
    free_cond_expr_children(res);
    return false;
}

bool parse_cond_expr_inplace(struct parser_state* s, struct cond_expr* res) {
    assert(res);

    res->last_else = parse_log_or_expr(s);
    if (!res->last_else) {
        return false;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        return false;
    }

    return true;
}

struct cond_expr* parse_cond_expr_cast(struct parser_state* s,
                                       struct cast_expr* start) {
    assert(start);

    struct cond_expr* res = xmalloc(sizeof(struct cond_expr));
    res->last_else = parse_log_or_expr_cast(s, start);
    if (!res->last_else) {
        free(res);
        return NULL;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_cond_expr_children(struct cond_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(e->conditionals);

    free_log_or_expr(e->last_else);
}

void free_cond_expr(struct cond_expr* e) {
    free_cond_expr_children(e);
    free(e);
}

struct constant create_constant(struct value val, struct source_loc loc) {
    enum constant_type t = value_is_float(val.type) ? CONSTANT_FLOAT
                                                    : CONSTANT_INT;
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = t,
        .val = val,
    };
}

struct constant create_enum_constant(const struct str* spelling,
                                     struct source_loc loc) {
    assert(spelling);
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = CONSTANT_ENUM,
        .spelling = *spelling,
    };
}

void free_constant(struct constant* c) {
    if (c->type == CONSTANT_ENUM) {
        free_str(&c->spelling);
    }
}

struct const_expr* parse_const_expr(struct parser_state* s) {
    struct const_expr* res = xmalloc(sizeof(struct const_expr));
    if (!parse_cond_expr_inplace(s, &res->expr)) {
        free(res);
        return NULL;
    }
    return res;
}

static void free_const_expr_children(struct const_expr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(struct const_expr* e) {
    free_const_expr_children(e);
    free(e);
}

static bool parse_eq_expr_eq_chain(struct parser_state* s,
                                   struct eq_expr* res) {
    assert(res->lhs);

    res->len = 0;
    res->eq_chain = NULL;

    size_t alloc_len = res->len;
    while (is_eq_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->eq_chain,
                       &alloc_len,
                       sizeof(struct rel_expr_and_op));
        }

        struct rel_expr_and_op* curr = &res->eq_chain[res->len];
        curr->rhs = parse_rel_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = op == EQ_OP ? EQ_EXPR_EQ : EQ_EXPR_NE;

        ++res->len;
    }

    res->eq_chain = xrealloc(res->eq_chain,
                             sizeof(struct rel_expr_and_op) * res->len);

    return true;
fail:
    free_eq_expr_children(res);
    return false;
}

bool parse_eq_expr_inplace(struct parser_state* s, struct eq_expr* res) {
    assert(res);

    res->lhs = parse_rel_expr(s);
    if (!res->lhs) {
        return false;
    }

    if (!parse_eq_expr_eq_chain(s, res)) {
        return false;
    }

    return true;
}

struct eq_expr* parse_eq_expr_cast(struct parser_state* s,
                                   struct cast_expr* start) {
    assert(start);

    struct rel_expr* lhs = parse_rel_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct eq_expr* res = xmalloc(sizeof(struct eq_expr));
    res->lhs = lhs;

    if (!parse_eq_expr_eq_chain(s, res)) {
        return NULL;
    }

    return res;
}

void free_eq_expr_children(struct eq_expr* e) {
    free_rel_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr(e->eq_chain[i].rhs);
    }
    free(e->eq_chain);
}

bool parse_expr_inplace(struct parser_state* s, struct expr* res) {
    assert(res);

    res->assign_exprs = xmalloc(sizeof(struct assign_expr));

    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        free(res->assign_exprs);
        return false;
    }
    res->len = 1;
    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (alloc_len == res->len) {
            grow_alloc((void**)&res->assign_exprs,
                       &alloc_len,
                       sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            free_expr_children(res);
            return false;
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs,
                                     sizeof(struct assign_expr) * res->len);
    }

    return true;
}

struct expr* parse_expr(struct parser_state* s) {
    struct expr* res = xmalloc(sizeof(struct expr));
    if (!parse_expr_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_expr_children(struct expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_assign_expr_children(&e->assign_exprs[i]);
    }
    free(e->assign_exprs);
}

void free_expr(struct expr* e) {
    free_expr_children(e);
    free(e);
}

bool parse_generic_assoc_inplace(struct parser_state* s,
                                 struct generic_assoc* res) {
    assert(res);
    
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == DEFAULT) {
        accept_it(s);
        res->type_name = NULL;
    } else {
        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            return false;
        }
    }

    if (!accept(s, COLON)) {
        goto fail;
    }

    res->assign = parse_assign_expr(s);
    if (!res->assign) {
        goto fail;
    }

    return true;
fail:
    if (res->type_name) {
        free_type_name(res->type_name);
    }
    return false;
}

void free_generic_assoc_children(struct generic_assoc* a) {
    if (a->type_name) {
        free_type_name(a->type_name);
    }
    free_assign_expr(a->assign);
}

struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s) {
    size_t alloc_len = 1;
    struct generic_assoc_list res = {
        .info = create_ast_node_info(s->it->loc),
        .len = 1,
        .assocs = xmalloc(sizeof(struct generic_assoc) * alloc_len),
    };
    
    if (!parse_generic_assoc_inplace(s, &res.assocs[0])) {
        free(res.assocs);
        return (struct generic_assoc_list){.len = 0, .assocs = NULL};
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.assocs,
                       &alloc_len,
                       sizeof(struct generic_assoc));
        }

        if (!parse_generic_assoc_inplace(s, &res.assocs[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.assocs = xrealloc(res.assocs, res.len * sizeof(struct generic_assoc));
    return res;

fail:
    free_generic_assoc_list(&res);
    return (struct generic_assoc_list){.len = 0, .assocs = NULL};
}

void free_generic_assoc_list(struct generic_assoc_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_generic_assoc_children(&l->assocs[i]);
    }
    free(l->assocs);
}

static struct generic_sel* create_generic_sel(
    struct assign_expr* assign,
    struct generic_assoc_list assocs,
    struct source_loc loc) {
    assert(assign);
    assert(assocs.len != 0);
    struct generic_sel* res = xmalloc(sizeof(struct generic_sel));
    
    res->info = create_ast_node_info(loc);
    res->assign = assign;
    res->assocs = assocs;
    return res;
}

struct generic_sel* parse_generic_sel(struct parser_state* s) {
    assert(s->it->type == GENERIC);
    struct source_loc loc = s->it->loc;
    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct assign_expr* assign = parse_assign_expr(s);
    if (!assign) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        goto fail;
    }

    struct generic_assoc_list assocs = parse_generic_assoc_list(s);
    if (assocs.len == 0) {
        goto fail;
    }

    if (!accept(s, RBRACKET)) {
        free_generic_assoc_list(&assocs);
        goto fail;
    }

    return create_generic_sel(assign, assocs, loc);

fail:
    free_assign_expr(assign);
    return NULL;
}

static void free_generic_sel_children(struct generic_sel* s) {
    free_assign_expr(s->assign);
    free_generic_assoc_list(&s->assocs);
}

void free_generic_sel(struct generic_sel* s) {
    free_generic_sel_children(s);
    free(s);
}

static bool parse_log_and_expr_rest(struct parser_state* s,
                                    struct log_and_expr* res) {
    assert(res);
    res->len = 1;
    size_t alloc_len = res->len;
    while (s->it->type == AND_OP) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->or_exprs,
                       &alloc_len,
                       sizeof(struct or_expr));
        }

        if (!parse_or_expr_inplace(s, &res->or_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->or_exprs = xrealloc(res->or_exprs, sizeof(struct or_expr) * res->len);
    return true;
fail:
    free_log_and_expr_children(res);
    return false;
}

bool parse_log_and_expr_inplace(struct parser_state* s,
                                struct log_and_expr* res) {
    assert(res);

    res->or_exprs = xmalloc(sizeof(struct or_expr));
    if (!parse_or_expr_inplace(s, res->or_exprs)) {
        free(res->or_exprs);
        return false;
    }

    if (!parse_log_and_expr_rest(s, res)) {
        return false;
    }

    return true;
}

struct log_and_expr* parse_log_and_expr_cast(struct parser_state* s,
                                             struct cast_expr* start) {
    assert(start);

    struct or_expr* or_exprs = parse_or_expr_cast(s, start);
    if (!or_exprs) {
        return NULL;
    }

    struct log_and_expr* res = xmalloc(sizeof(struct log_and_expr));
    res->or_exprs = or_exprs;

    if (!parse_log_and_expr_rest(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

void free_log_and_expr_children(struct log_and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_or_expr_children(&e->or_exprs[i]);
    }
    free(e->or_exprs);
}

static void free_log_or_expr_children(struct log_or_expr* e);

static bool parse_log_or_expr_ops(struct parser_state* s,
                                  struct log_or_expr* res) {
    assert(res);
    assert(res->len == 1);

    size_t alloc_len = res->len;
    while (s->it->type == OR_OP) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->log_ands,
                       &alloc_len,
                       sizeof(struct log_and_expr));
        }

        if (!parse_log_and_expr_inplace(s, &res->log_ands[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->log_ands = xrealloc(res->log_ands,
                             sizeof(struct log_and_expr) * res->len);

    return true;
fail:
    free_log_or_expr_children(res);
    return false;
}

struct log_or_expr* parse_log_or_expr(struct parser_state* s) {
    struct log_and_expr* and_exprs = xmalloc(sizeof(struct log_and_expr));
    if (!parse_log_and_expr_inplace(s, and_exprs)) {
        free(and_exprs);
        return NULL;
    }

    struct log_or_expr* res = xmalloc(sizeof(struct log_or_expr));
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

struct log_or_expr* parse_log_or_expr_cast(struct parser_state* s,
                                           struct cast_expr* start) {
    assert(start);

    struct log_and_expr* and_exprs = parse_log_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    struct log_or_expr* res = xmalloc(sizeof(struct log_or_expr));
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

static void free_log_or_expr_children(struct log_or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_log_and_expr_children(&e->log_ands[i]);
    }
    free(e->log_ands);
}

void free_log_or_expr(struct log_or_expr* e) {
    free_log_or_expr_children(e);
    free(e);
}

static enum mul_expr_op token_type_to_mul_op(enum token_type t) {
    assert(is_mul_op(t));
    switch (t) {
        case ASTERISK:
            return MUL_EXPR_MUL;
        case DIV:
            return MUL_EXPR_DIV;
        case MOD:
            return MUL_EXPR_MOD;

        default:
            UNREACHABLE();
    }
}

static bool parse_mul_expr_mul_chain(struct parser_state* s,
                                     struct mul_expr* res) {
    res->len = 0;
    res->mul_chain = NULL;

    size_t alloc_len = res->len;
    while (is_mul_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->mul_chain,
                       &alloc_len,
                       sizeof(struct cast_expr_and_op));
        }

        struct cast_expr_and_op* curr = &res->mul_chain[res->len];
        curr->rhs = parse_cast_expr(s);
        if (!curr->rhs) {
            free_mul_expr(res);
            return false;
        }
        curr->op = token_type_to_mul_op(op);

        ++res->len;
    }

    res->mul_chain = xrealloc(res->mul_chain,
                              sizeof(struct cast_expr_and_op) * res->len);

    return true;
}

struct mul_expr* parse_mul_expr(struct parser_state* s) {
    struct cast_expr* lhs = parse_cast_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));
    
    res->lhs = lhs;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct mul_expr* parse_mul_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));
    res->lhs = start;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_mul_expr_children(struct mul_expr* e) {
    free_cast_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_cast_expr(e->mul_chain[i].rhs);
    }
    free(e->mul_chain);
}

void free_mul_expr(struct mul_expr* e) {
    free_mul_expr_children(e);
    free(e);
}

static bool parse_or_expr_rest(struct parser_state* s, struct or_expr* res) {
    res->len = 1;

    size_t alloc_len = res->len;
    while (s->it->type == OR) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->xor_exprs,
                       &alloc_len,
                       sizeof(struct xor_expr));
        }

        if (!parse_xor_expr_inplace(s, &res->xor_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->xor_exprs = xrealloc(res->xor_exprs,
                              sizeof(struct xor_expr) * res->len);

    return true;

fail:
    free_or_expr_children(res);
    return false;
}

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res) {
    assert(res);

    res->xor_exprs = xmalloc(sizeof(struct xor_expr));
    if (!parse_xor_expr_inplace(s, res->xor_exprs)) {
        free(res->xor_exprs);
        return false;
    }

    if (!parse_or_expr_rest(s, res)) {
        return false;
    }

    return true;
}

struct or_expr* parse_or_expr_cast(struct parser_state* s,
                                   struct cast_expr* start) {
    assert(start);

    struct xor_expr* xor_exprs = parse_xor_expr_cast(s, start);
    if (!xor_exprs) {
        return NULL;
    }

    struct or_expr* res = xmalloc(sizeof(struct or_expr));
    res->xor_exprs = xor_exprs;

    if (!parse_or_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void free_or_expr_children(struct or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_xor_expr_children(&e->xor_exprs[i]);
    }
    free(e->xor_exprs);
}

static bool is_posfix_op(enum token_type t) {
    switch (t) {
        case LINDEX:
        case LBRACKET:
        case DOT:
        case PTR_OP:
        case INC_OP:
        case DEC_OP:
            return true;

        default:
            return false;
    }
}

static bool parse_postfix_arr_suffix(struct parser_state* s,
                                     struct postfix_suffix* res) {
    assert(res);
    assert(s->it->type == LINDEX);

    accept_it(s);
    struct expr* expr = parse_expr(s);
    if (!expr) {
        return false;
    }
    if (!accept(s, RINDEX)) {
        free_expr(expr);
        return false;
    }

    res->type = POSTFIX_INDEX;
    res->index_expr = expr;
    return true;
}

static bool parse_postfix_func_suffix(struct parser_state* s,
                                      struct postfix_suffix* res) {
    assert(res);
    assert(s->it->type == LBRACKET);

    accept_it(s);
    struct arg_expr_list arg_expr_list = {
        .assign_exprs = NULL,
        .len = 0,
    };
    if (s->it->type != RBRACKET) {
        arg_expr_list = parse_arg_expr_list(s);
        if (arg_expr_list.len == 0) {
            return false;
        }
    }
    if (!accept(s, RBRACKET)) {
        free_arg_expr_list(&arg_expr_list);
        return false;
    }
    res->type = POSTFIX_BRACKET;
    res->bracket_list = arg_expr_list;
    return true;
}

static bool parse_postfix_access_suffix(struct parser_state* s,
                                        struct postfix_suffix* res) {
    assert(res);
    assert(s->it->type == DOT || s->it->type == PTR_OP);
    enum postfix_suffix_type type = s->it->type == PTR_OP
                                        ? POSTFIX_PTR_ACCESS
                                        : POSTFIX_ACCESS;
    accept_it(s);
    if (s->it->type != IDENTIFIER) {
        return false;
    }
    const struct str spelling = take_spelling(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);
    struct identifier* identifier = create_identifier(&spelling, loc);
    res->type = type;
    res->identifier = identifier;
    return true;
}

struct postfix_suffix parse_postfix_inc_dec_suffix(struct parser_state* s) {
    assert(s->it->type == INC_OP || s->it->type == DEC_OP);
    const enum token_type op = s->it->type;
    accept_it(s);
    return (struct postfix_suffix){
        .type = POSTFIX_INC_DEC,
        .is_inc = op == INC_OP,
    };
}

static bool parse_postfix_suffixes(struct parser_state* s,
                                   struct postfix_expr* res) {
    size_t alloc_len = 0;
    while (is_posfix_op(s->it->type)) {
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->suffixes,
                       &alloc_len,
                       sizeof(struct postfix_suffix));
        }

        switch (s->it->type) {
            case LINDEX:
                if (!parse_postfix_arr_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case LBRACKET:
                if (!parse_postfix_func_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case DOT:
            case PTR_OP:
                if (!parse_postfix_access_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case INC_OP:
            case DEC_OP:
                res->suffixes[res->len] = parse_postfix_inc_dec_suffix(s);
                break;

            default:
                UNREACHABLE();
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->suffixes = xrealloc(res->suffixes,
                                 res->len * sizeof(struct postfix_suffix));
    }

    return true;
}

struct postfix_expr* parse_postfix_expr(struct parser_state* s) {
    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->type == LBRACKET && next_is_type_name(s)) {
        res->info = create_ast_node_info(s->it->loc);
        accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            free(res);
            return NULL;
        }

        if (!(accept(s, RBRACKET) && accept(s, LBRACE))) {
            free_type_name(res->type_name);
            free(res);
            return NULL;
        }

        res->init_list = parse_init_list(s);
        if (res->init_list.len == 0) {
            free_type_name(res->type_name);
            free(res);
            return NULL;
        }

        if (s->it->type == COMMA) {
            accept_it(s);
        }

        if (!accept(s, RBRACE)) {
            free_postfix_expr(res);
            return NULL;
        }
    } else {
        res->is_primary = true;
        res->primary = parse_primary_expr(s);
        if (!res->primary) {
            free(res);
            return NULL;
        }
    }

    if (!parse_postfix_suffixes(s, res)) {
        free_postfix_expr(res);
        return NULL;
    }

    return res;
}

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @return A postfix_expr that uses the given type_name
 */
struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s,
                                                  struct type_name* type_name,
                                                  struct source_loc start_bracket_loc) {
    assert(type_name);
    assert(s->it->type == LBRACE);

    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->info = create_ast_node_info(start_bracket_loc);
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    accept_it(s);

    res->init_list = parse_init_list(s);
    if (s->err->type != PARSER_ERR_NONE) {
        goto fail;
    }

    if (s->it->type == COMMA) {
        accept_it(s);
    }

    if (!accept(s, RBRACE)) {
        return NULL;
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }
    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}

static void free_postfix_expr_children(struct postfix_expr* p) {
    if (p->is_primary) {
        free_primary_expr(p->primary);
    } else {
        free_type_name(p->type_name);
        free_init_list_children(&p->init_list);
    }
    for (size_t i = 0; i < p->len; ++i) {
        struct postfix_suffix* s = &p->suffixes[i];
        switch (s->type) {
            case POSTFIX_INDEX:
                free_expr(s->index_expr);
                break;
            case POSTFIX_BRACKET:
                free_arg_expr_list(&s->bracket_list);
                break;
            case POSTFIX_ACCESS:
            case POSTFIX_PTR_ACCESS:
                free_identifier(s->identifier);
                break;
            case POSTFIX_INC_DEC:
                break;
        }
    }
    free(p->suffixes);
}

void free_postfix_expr(struct postfix_expr* p) {
    free_postfix_expr_children(p);
    free(p);
}

static struct primary_expr* create_primary_expr_constant(
    struct constant constant) {
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_CONSTANT;
    res->constant = constant;

    return res;
}

static struct primary_expr* create_primary_expr_string(
    struct string_constant string) {
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_STRING_LITERAL;
    res->string = string;

    return res;
}

static struct primary_expr* create_primary_expr_identifier(
    struct identifier* identifier) {
    assert(identifier);

    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

static struct primary_expr* create_primary_expr_bracket(
    struct expr* bracket_expr,
    struct source_loc loc) {
    assert(bracket_expr);
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_BRACKET;
    res->info = create_ast_node_info(loc);
    res->bracket_expr = bracket_expr;

    return res;
}

static struct primary_expr* create_primary_expr_generic(
    struct generic_sel* generic) {
    assert(generic);
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_GENERIC;
    res->generic = generic;

    return res;
}

struct primary_expr* parse_primary_expr(struct parser_state* s) {
    switch (s->it->type) {
        case IDENTIFIER: {
            const struct str spelling = take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            if (is_enum_constant(s, &spelling)) {
                return create_primary_expr_constant(
                    create_enum_constant(&spelling, loc));
            }
            return create_primary_expr_identifier(
                create_identifier(&spelling, loc));
        }
        case F_CONSTANT:
        case I_CONSTANT: {
            struct source_loc loc = s->it->loc;
            struct value val = s->it->val;
            accept_it(s);
            return create_primary_expr_constant(create_constant(val, loc));
        }
        case STRING_LITERAL: {
            const struct str spelling = take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            return create_primary_expr_string(
                create_string_constant(&spelling, loc));
        }
        case FUNC_NAME: {
            struct source_loc loc = s->it->loc;
            accept_it(s);
            return create_primary_expr_string(create_func_name(loc));
        }
        case GENERIC: {
            struct generic_sel* generic = parse_generic_sel(s);
            if (!generic) {
                return NULL;
            }
            return create_primary_expr_generic(generic);
        }

        default: {
            struct source_loc loc = s->it->loc;
            if (accept(s, LBRACKET)) {
                struct expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr, loc);
                } else {
                    free_expr(bracket_expr);
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

static void free_primary_expr_children(struct primary_expr* e) {
    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            free_identifier(e->identifier);
            break;

        case PRIMARY_EXPR_CONSTANT:
            free_constant(&e->constant);
            break;

        case PRIMARY_EXPR_STRING_LITERAL:
            free_string_constant(&e->string);
            break;

        case PRIMARY_EXPR_BRACKET:
            free_expr(e->bracket_expr);
            break;

        case PRIMARY_EXPR_GENERIC:
            free_generic_sel(e->generic);
            break;
    }
}

void free_primary_expr(struct primary_expr* e) {
    free_primary_expr_children(e);
    free(e);
}

static enum rel_expr_op token_type_to_rel_op(enum token_type t) {
    assert(is_rel_op(t));
    switch (t) {
        case LT:
            return REL_EXPR_LT;
        case GT:
            return REL_EXPR_GT;
        case LE_OP:
            return REL_EXPR_LE;
        case GE_OP:
            return REL_EXPR_GE;
        
        default:
            UNREACHABLE();
    }
}

static bool parse_rel_expr_rel_chain(struct parser_state* s,
                                     struct rel_expr* res) {
    assert(res->lhs);

    res->len = 0;
    res->rel_chain = NULL;

    size_t alloc_len = res->len;
    while (is_rel_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->rel_chain,
                       &alloc_len,
                       sizeof(struct shift_expr_and_op));
        }

        struct shift_expr_and_op* curr = &res->rel_chain[res->len];
        curr->rhs = parse_shift_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = token_type_to_rel_op(op);

        ++res->len;
    }

    res->rel_chain = xrealloc(res->rel_chain,
                              sizeof(struct shift_expr_and_op) * res->len);

    return true;
fail:
    free_rel_expr(res);
    return false;
}

struct rel_expr* parse_rel_expr(struct parser_state* s) {
    struct shift_expr* lhs = parse_shift_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct rel_expr* res = xmalloc(sizeof(struct rel_expr));
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct rel_expr* parse_rel_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct shift_expr* lhs = parse_shift_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct rel_expr* res = xmalloc(sizeof(struct rel_expr));
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

void free_rel_expr_children(struct rel_expr* e) {
    free_shift_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].rhs);
    }
    free(e->rel_chain);
}

void free_rel_expr(struct rel_expr* e) {
    free_rel_expr_children(e);
    free(e);
}

static bool parse_shift_expr_shift_chain(struct parser_state* s,
                                         struct shift_expr* res) {
    res->len = 0;
    res->shift_chain = NULL;

    size_t alloc_len = res->len;
    while (is_shift_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->shift_chain,
                       &alloc_len,
                       sizeof(struct add_expr_and_op));
        }

        struct add_expr_and_op* curr = &res->shift_chain[res->len];
        curr->rhs = parse_add_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = op == LEFT_OP ? SHIFT_EXPR_LEFT : SHIFT_EXPR_RIGHT;

        ++res->len;
    }

    res->shift_chain = xrealloc(res->shift_chain,
                                sizeof(struct add_expr_and_op) * res->len);

    return true;

fail:
    free_shift_expr(res);
    return false;
}
struct shift_expr* parse_shift_expr(struct parser_state* s) {
    struct add_expr* lhs = parse_add_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct shift_expr* res = xmalloc(sizeof(struct shift_expr));
    res->lhs = lhs;

    if (!parse_shift_expr_shift_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct shift_expr* parse_shift_expr_cast(struct parser_state* s,
                                         struct cast_expr* start) {
    assert(start);

    struct add_expr* lhs = parse_add_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct shift_expr* res = xmalloc(sizeof(struct shift_expr));
    res->lhs = lhs;

    if (!parse_shift_expr_shift_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_shift_expr_children(struct shift_expr* e) {
    free_add_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].rhs);
    }
    free(e->shift_chain);
}

void free_shift_expr(struct shift_expr* e) {
    free_shift_expr_children(e);
    free(e);
}

struct string_constant create_string_constant(const struct str* spelling, struct source_loc loc) {
    assert(spelling);
    return (struct string_constant){
        .is_func = false,
        .lit = create_string_literal(spelling, loc),
    };
}

struct string_constant create_func_name(struct source_loc loc) {
    return (struct string_constant){
        .is_func = true,
        .info = create_ast_node_info(loc),
    };
}

void free_string_constant(struct string_constant* c) {
    if (!c->is_func) {
        free_string_literal(&c->lit);
    }
}

static inline void assign_operators_before(struct unary_expr* res,
                                           enum unary_expr_op* ops_before,
                                           size_t len) {
    assert(res);
    if (len > 0) {
        assert(ops_before);
    } else {
        assert(ops_before == NULL);
    }
    res->len = len;
    res->ops_before = ops_before;
}

static struct unary_expr* create_unary_expr_postfix(
    enum unary_expr_op* ops_before,
    size_t len,
    struct postfix_expr* postfix,
    struct source_loc loc) {
    assert(postfix);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_POSTFIX;
    res->postfix = postfix;

    return res;
}

static enum unary_expr_type token_type_to_unary_expr_type(enum token_type t) {
    assert(is_unary_op(t));
    switch (t) {
        case AND:
            return UNARY_ADDRESSOF;
        case ASTERISK:
            return UNARY_DEREF;
        case ADD:
            return UNARY_PLUS;
        case SUB:
            return UNARY_MINUS;
        case BNOT:
            return UNARY_BNOT;
        case NOT:
            return UNARY_NOT;
        default:
            UNREACHABLE();
    }
}

static struct unary_expr* create_unary_expr_unary_op(
    enum unary_expr_op* ops_before,
    size_t len,
    enum token_type unary_op,
    struct cast_expr* cast_expr,
    struct source_loc loc) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = token_type_to_unary_expr_type(unary_op);
    res->cast_expr = cast_expr;

    return res;
}

static struct unary_expr* create_unary_expr_sizeof_type(
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc loc) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;

    return res;
}

static struct unary_expr* create_unary_expr_alignof(
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc loc) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_ALIGNOF;
    res->type_name = type_name;

    return res;
}

struct unary_expr* parse_unary_expr_type_name(
    struct parser_state* s,
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc start_bracket_loc) {
    assert(type_name);

    struct postfix_expr* postfix = parse_postfix_expr_type_name(
        s,
        type_name,
        start_bracket_loc);
    if (!postfix) {
        return NULL;
    }

    return create_unary_expr_postfix(ops_before,
                                     len,
                                     postfix,
                                     start_bracket_loc);
}

enum unary_expr_op token_type_to_unary_expr_op(enum token_type t) {
    assert(t == INC_OP || t == DEC_OP || t == SIZEOF);
    switch (t) {
        case INC_OP:
            return UNARY_OP_INC;
        case DEC_OP:
            return UNARY_OP_DEC;
        case SIZEOF:
            return UNARY_OP_SIZEOF;

        default:
            UNREACHABLE();
    }
}

struct unary_expr* parse_unary_expr(struct parser_state* s) {
    size_t alloc_len = 0;
    enum unary_expr_op* ops_before = NULL;

    const struct source_loc loc = s->it->loc;
    size_t len = 0;
    while (s->it->type == INC_OP || s->it->type == DEC_OP
           || (s->it->type == SIZEOF && (s->it + 1)->type != LBRACKET)) {
        if (len == alloc_len) {
            grow_alloc((void**)&ops_before,
                       &alloc_len,
                       sizeof(enum token_type));
        }

        ops_before[len] = token_type_to_unary_expr_op(s->it->type);

        ++len;
        accept_it(s);
    }

    if (ops_before) {
        ops_before = xrealloc(ops_before, len * sizeof(enum token_type));
    }

    if (is_unary_op(s->it->type)) {
        const enum token_type unary_op = s->it->type;
        accept_it(s);
        struct cast_expr* cast = parse_cast_expr(s);
        if (!cast) {
            goto fail;
        }
        return create_unary_expr_unary_op(ops_before, len, unary_op, cast, loc);
    } else {
        switch (s->it->type) {
            case SIZEOF: {
                accept_it(s);
                assert(s->it->type == LBRACKET);
                struct source_loc start_bracket_loc = s->it->loc;
                if (next_is_type_name(s)) {
                    accept_it(s);

                    struct type_name* type_name = parse_type_name(s);
                    if (!type_name) {
                        goto fail;
                    }

                    if (!accept(s, RBRACKET)) {
                        goto fail;
                    }
                    if (s->it->type == LBRACE) {
                        ++len;
                        ops_before = xrealloc(ops_before,
                                              len * sizeof(enum token_type));
                        ops_before[len - 1] = UNARY_OP_SIZEOF;

                        struct unary_expr* res = parse_unary_expr_type_name(
                            s,
                            ops_before,
                            len,
                            type_name,
                            start_bracket_loc);
                        if (!res) {
                            free_type_name(type_name);
                            goto fail;
                        }
                        return res;
                    } else {
                        return create_unary_expr_sizeof_type(ops_before,
                                                             len,
                                                             type_name,
                                                             loc);
                    }
                } else {
                    ++len;
                    ops_before = xrealloc(ops_before,
                                          len * sizeof(enum token_type));
                    ops_before[len - 1] = UNARY_OP_SIZEOF;

                    struct postfix_expr* postfix = parse_postfix_expr(s);
                    if (!postfix) {
                        goto fail;
                    }
                    return create_unary_expr_postfix(ops_before,
                                                     len,
                                                     postfix,
                                                     loc);
                }
            }
            case ALIGNOF: {
                accept_it(s);
                if (!accept(s, LBRACKET)) {
                    goto fail;
                }

                struct type_name* type_name = parse_type_name(s);
                if (!type_name) {
                    goto fail;
                }

                if (!accept(s, RBRACKET)) {
                    goto fail;
                }
                return create_unary_expr_alignof(ops_before, len, type_name, loc);
            }
            default: {
                struct postfix_expr* postfix = parse_postfix_expr(s);
                if (!postfix) {
                    goto fail;
                }
                return create_unary_expr_postfix(ops_before, len, postfix, loc);
            }
        }
    }
fail:
    free(ops_before);
    return NULL;
}

void free_unary_expr_children(struct unary_expr* u) {
    free(u->ops_before);
    switch (u->type) {
        case UNARY_POSTFIX:
            free_postfix_expr(u->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT: 
            free_cast_expr(u->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            free_type_name(u->type_name);
            break;
    }
}

void free_unary_expr(struct unary_expr* u) {
    free_unary_expr_children(u);
    free(u);
}

static bool parse_xor_expr_rest(struct parser_state* s, struct xor_expr* res) {
    assert(res->and_exprs);

    res->len = 1;

    size_t alloc_len = res->len;
    while (s->it->type == XOR) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->and_exprs,
                       &alloc_len,
                       sizeof(struct and_expr));
        }

        if (!parse_and_expr_inplace(s, &res->and_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->and_exprs = xrealloc(res->and_exprs,
                              sizeof(struct and_expr) * res->len);
    return true;

fail:
    free_xor_expr_children(res);
    return false;
}

bool parse_xor_expr_inplace(struct parser_state* s, struct xor_expr* res) {
    assert(res);

    res->and_exprs = xmalloc(sizeof(struct and_expr));
    if (!parse_and_expr_inplace(s, res->and_exprs)) {
        free(res->and_exprs);
        return false;
    }

    if (!parse_xor_expr_rest(s, res)) {
        return false;
    }

    return true;
}

struct xor_expr* parse_xor_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct and_expr* and_exprs = parse_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    struct xor_expr* res = xmalloc(sizeof(struct xor_expr));
    res->and_exprs = and_exprs;

    if (!parse_xor_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void free_xor_expr_children(struct xor_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_and_expr_children(&e->and_exprs[i]);
    }
    free(e->and_exprs);
}

struct designation* parse_designation(struct parser_state* s) {
    struct designator_list designators = parse_designator_list(s);
    if (designators.len == 0) {
        return NULL;
    }

    if (!accept(s, ASSIGN)) {
        free_designator_list(&designators);
        return NULL;
    }

    struct designation* res = xmalloc(sizeof(struct designation));
    res->designators = designators;
    return res;
}

static void free_designation_children(struct designation* d) {
    free_designator_list(&d->designators);
}

void free_designation(struct designation* d) {
    free_designation_children(d);
    free(d);
}

bool parse_designator_inplace(struct parser_state* s, struct designator* res) {
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->type) {
        case LINDEX: {
            accept_it(s);
            struct const_expr* index = parse_const_expr(s);
            if (!index) {
                return false;
            }
            if (!accept(s, RINDEX)) {
                free_const_expr(index);
                return false;
            }

            res->is_index = true;
            res->arr_index = index;
            return true;
        }
        case DOT: {
            accept_it(s);
            if (s->it->type == IDENTIFIER) {
                const struct str spell = take_spelling(s->it);
                struct source_loc loc = s->it->loc;
                accept_it(s);
                res->is_index = false;
                res->identifier = create_identifier(&spell, loc);
                return true;
            } else {
                expected_token_error(s, IDENTIFIER);
                return false;
            }
        }
        default: {
            enum token_type expected[] = {LINDEX, DOT};
            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return false;
        }
    }
}

void free_designator_children(struct designator* d) {
    if (d->is_index) {
        free_const_expr(d->arr_index);
    } else {
        free_identifier(d->identifier);
    }
}

struct designator_list parse_designator_list(struct parser_state* s) {
    struct designator_list res = {
        .len = 1,
        .designators = xmalloc(sizeof(struct designator)),
    };

    if (!parse_designator_inplace(s, &res.designators[0])) {
        free(res.designators);
        return (struct designator_list){.len = 0, .designators = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == LINDEX || s->it->type == DOT) {
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.designators,
                       &alloc_len,
                       sizeof(struct designator));
        }

        if (!parse_designator_inplace(s, &res.designators[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.designators = xrealloc(res.designators,
                               res.len * sizeof(struct designator));

    return res;

fail:
    free_designator_list(&res);
    return (struct designator_list){.len = 0, .designators = NULL};
}

void free_designator_list(struct designator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_designator_children(&l->designators[i]);
    }
    free(l->designators);
}

static struct initializer* create_initializer_assign(
    struct source_loc loc,    
    struct assign_expr* assign) {
    assert(assign);
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->info = create_ast_node_info(loc);
    res->is_assign = true;
    res->assign = assign;

    return res;
}

static struct initializer* create_initializer_init_list(
    struct source_loc loc,
    struct init_list init_list) {
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->info = create_ast_node_info(loc);
    res->is_assign = false;
    res->init_list = init_list;

    return res;
}

struct initializer* parse_initializer(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (s->it->type == LBRACE) {
        accept_it(s);
        struct init_list init_list = parse_init_list(s);
        if (init_list.len == 0) {
            return NULL;
        }

        if (s->it->type == COMMA) {
            accept_it(s);
        }

        if (!accept(s, RBRACE)) {
            free_init_list_children(&init_list);
        }
        return create_initializer_init_list(loc, init_list);
    } else {
        struct assign_expr* assign = parse_assign_expr(s);
        if (!assign) {
            return NULL;
        }
        return create_initializer_assign(loc, assign);
    }
}

void free_initializer_children(struct initializer* i) {
    if (i->is_assign) {
        free_assign_expr(i->assign);
    } else {
        free_init_list_children(&i->init_list);
    }
}

void free_initializer(struct initializer* i) {
    free_initializer_children(i);
    free(i);
}

static bool parse_designation_init(struct parser_state* s,
                                   struct designation_init* res) {
    if (s->it->type == LINDEX || s->it->type == DOT) {
        res->designation = parse_designation(s);
        if (!res->designation) {
            return false;
        }
    } else {
        res->designation = NULL;
    }

    res->init = parse_initializer(s);
    if (!res->init) {
        if (res->designation) {
            free_designation(res->designation);
        }
        return false;
    }
    return true;
}

struct init_list parse_init_list(struct parser_state* s) {
    struct init_list res = {
        .len = 1,
        .inits = xmalloc(sizeof(struct designation_init)),
    };
    if (!parse_designation_init(s, &res.inits[0])) {
        free(res.inits);
        return (struct init_list){.len = 0, .inits = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == COMMA && s->it[1].type != RBRACE) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.inits,
                       &alloc_len,
                       sizeof(struct designation_init));
        }

        if (!parse_designation_init(s, &res.inits[res.len])) {
            goto fail;
        }

        ++res.len;
    }

    res.inits = xrealloc(res.inits, res.len * sizeof(struct designation_init));

    return res;
fail:
    free_init_list_children(&res);
    return (struct init_list){.len = 0, .inits = NULL};
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct designation_init* item = &l->inits[i];
        if (item->designation) {
            free_designation(item->designation);
        }
        free_initializer(item->init);
    }
    free(l->inits);
}

bool parse_block_item_inplace(struct parser_state* s, struct block_item* res) {
    if (is_declaration(s)) {
        res->is_decl = true;
        if (!parse_declaration_inplace(s, &res->decl)) {
            return false;
        }
    } else {
        res->is_decl = false;
        if (!parse_statement_inplace(s, &res->stat)) {
            return false;
        }
    }

    return true;
}

void free_block_item_children(struct block_item* i) {
    if (i->is_decl) {
        free_declaration_children(&i->decl);
    } else {
        free_statement_children(&i->stat);
    }
}

struct compound_statement* parse_compound_statement(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, LBRACE)) {
        return NULL;
    }

    parser_push_scope(s);

    struct compound_statement* res = xmalloc(sizeof(struct compound_statement));
    res->info = create_ast_node_info(loc);
    res->items = NULL;
    res->len = 0;

    size_t alloc_len = res->len;
    while (s->it->type != RBRACE) {
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->items,
                       &alloc_len,
                       sizeof(struct block_item));
        }

        if (!parse_block_item_inplace(s, &res->items[res->len])) {
            free_compound_statement(res);
            return NULL;
        }

        ++res->len;
    }

    assert(s->it->type == RBRACE);
    accept_it(s);

    parser_pop_scope(s);

    return res;
}

void free_compound_statement_children(struct compound_statement* s) {
    for (size_t i = 0; i < s->len; ++i) {
        free_block_item_children(&s->items[i]);
    }
    free(s->items);
}

void free_compound_statement(struct compound_statement* s) {
    free_compound_statement_children(s);
    free(s);
}

struct expr_statement* parse_expr_statement(struct parser_state* s) {
    struct expr_statement* res = xmalloc(sizeof(struct expr_statement));
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == SEMICOLON) {
        accept_it(s);
        res->expr.len = 0;
        res->expr.assign_exprs = NULL;
        return res;
    } else {
        if (!parse_expr_inplace(s, &res->expr)) {
            free(res);
            return NULL;
        }

        if (!accept(s, SEMICOLON)) {
            free_expr_statement(res);
            return NULL;
        }

        return res;
    }
}

static void free_expr_statement_children(struct expr_statement* s) {
    free_expr_children(&s->expr);
}

void free_expr_statement(struct expr_statement* s) {
    free_expr_statement_children(s);
    free(s);
}

static inline void assign_do_or_while(struct source_loc loc,
                                      struct iteration_statement* res,
                                      struct expr* while_cond,
                                      struct statement* loop_body) {
    assert(res);
    assert(while_cond);
    assert(loop_body);
    res->info = create_ast_node_info(loc);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

static struct iteration_statement* create_while_loop(
    struct source_loc loc,
    struct expr* while_cond,
    struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->type = ITERATION_STATEMENT_WHILE;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_do_loop(struct source_loc loc,
                                                  struct expr* while_cond,
                                                  struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->type = ITERATION_STATEMENT_DO;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_for_loop(
    struct source_loc loc,
    struct for_loop for_loop,
    struct statement* loop_body) {
    if (for_loop.is_decl) {
        assert(for_loop.init_decl);
    } else {
        assert(for_loop.init_expr);
    }
    assert(for_loop.cond);
    assert(loop_body);
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->info = create_ast_node_info(loc);
    res->type = ITERATION_STATEMENT_FOR;
    res->loop_body = loop_body;
    res->for_loop = for_loop;

    return res;
}

static struct iteration_statement* parse_while_statement(
    struct parser_state* s,
    struct source_loc loc) {
    assert(s->it->type == WHILE);

    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!while_cond) {
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_expr(while_cond);
        return NULL;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        free_expr(while_cond);
        return NULL;
    }

    return create_while_loop(loc, while_cond, loop_body);
}

static struct iteration_statement* parse_do_loop(struct parser_state* s,
                                                 struct source_loc loc) {
    assert(s->it->type == DO);

    accept_it(s);

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        return NULL;
    }

    if (!accept(s, WHILE) || !accept(s, LBRACKET)) {
        free_statement(loop_body);
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!(while_cond && accept(s, RBRACKET) && accept(s, SEMICOLON))) {
        free_statement(loop_body);
        return NULL;
    }

    return create_do_loop(loc, while_cond, loop_body);
}

static struct iteration_statement* parse_for_loop(struct parser_state* s,
                                                  struct source_loc loc) {
    assert(s->it->type == FOR);

    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct for_loop loop;
    if (is_declaration(s)) {
        loop.is_decl = true;
        loop.init_decl = parse_declaration(s);
        if (!loop.init_decl) {
            return NULL;
        }
    } else {
        loop.is_decl = false;
        loop.init_expr = parse_expr_statement(s);
        if (!loop.init_expr) {
            return NULL;
        }
    }

    loop.cond = parse_expr_statement(s);
    if (!loop.cond) {
        if (loop.is_decl) {
            free_declaration(loop.init_decl);
        } else {
            free_expr_statement(loop.init_expr);
        }
        return NULL;
    }

    if (s->it->type != RBRACKET) {
        loop.incr_expr = parse_expr(s);
        if (!loop.incr_expr) {
            goto fail;
        }
    } else {
        loop.incr_expr = NULL;
    }

    if (!accept(s, RBRACKET)) {
        goto fail;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        goto fail;
    }

    return create_for_loop(loc, loop, loop_body);
fail:
    if (loop.is_decl) {
        free_declaration(loop.init_decl);
    } else {
        free_expr_statement(loop.init_expr);
    }
    free_expr_statement(loop.cond);
    return NULL;
}

struct iteration_statement* parse_iteration_statement(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    switch (s->it->type) {
        case WHILE:
            return parse_while_statement(s, loc);
        case DO:
            return parse_do_loop(s, loc);

        case FOR: {
            return parse_for_loop(s, loc);
        }

        default: {
            enum token_type expected[] = {WHILE, DO, FOR};

            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
    }
}

static void free_iteration_statement_children(struct iteration_statement* s) {
    switch (s->type) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            free_expr(s->while_cond);
            break;
        case ITERATION_STATEMENT_FOR: {
            if (s->for_loop.is_decl) {
                free_declaration(s->for_loop.init_decl);
            } else {
                free_expr_statement(s->for_loop.init_expr);
            }
            free_expr_statement(s->for_loop.cond);
            if (s->for_loop.incr_expr) {
                free_expr(s->for_loop.incr_expr);
            }
            break;
        }
        default:
            UNREACHABLE();
    }
    free_statement(s->loop_body);
}

void free_iteration_statement(struct iteration_statement* s) {
    free_iteration_statement_children(s);
    free(s);
}

static struct jump_statement* create(struct source_loc loc,
                                     enum jump_statement_type type) {
    struct jump_statement* res = xmalloc(sizeof(struct jump_statement));
    res->info = create_ast_node_info(loc);
    res->type = type;

    return res;
}

static struct jump_statement* create_goto_statement(
    struct source_loc loc,
    struct identifier* identifier) {
    assert(identifier);
    struct jump_statement* res = create(loc, JUMP_STATEMENT_GOTO);
    res->goto_label = identifier;

    return res;
}

static struct jump_statement* create_return_statement(struct source_loc loc,
                                                      struct expr* ret_val) {
    struct jump_statement* res = create(loc, JUMP_STATEMENT_RETURN);
    res->ret_val = ret_val;
    return res;
}

struct jump_statement* parse_jump_statement(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    struct jump_statement* res = NULL;
    switch (s->it->type) {
        case GOTO: {
            accept_it(s);
            if (s->it->type == IDENTIFIER) {
                const struct str spell = take_spelling(s->it);
                const struct source_loc id_loc = s->it->loc;
                accept_it(s);
                res = create_goto_statement(loc, create_identifier(&spell, id_loc));
                break;
            } else {
                expected_token_error(s, IDENTIFIER);
                return NULL;
            }
        }
        case CONTINUE:
        case BREAK: {
            const enum token_type t = s->it->type;
            accept_it(s);
            res = create(loc, t == CONTINUE ? JUMP_STATEMENT_CONTINUE : JUMP_STATEMENT_BREAK);
            res->ret_val = NULL;
            break;
        }
        case RETURN: {
            accept_it(s);
            struct expr* ret_val;
            if (s->it->type == SEMICOLON) {
                ret_val = NULL;
            } else {
                ret_val = parse_expr(s);
                if (!ret_val) {
                    return NULL;
                }
            }

            res = create_return_statement(loc, ret_val);
            break;
        }

        default: {
            enum token_type expected[] = {GOTO, CONTINUE, BREAK, RETURN};
            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
    }

    if (!accept(s, SEMICOLON)) {
        free_jump_statement(res);
        return NULL;
    }

    return res;
}

static void free_jump_statement_children(struct jump_statement* s) {
    switch (s->type) {
        case JUMP_STATEMENT_GOTO:
            free_identifier(s->goto_label);
            break;
        case JUMP_STATEMENT_RETURN:
            if (s->ret_val) {
                free_expr(s->ret_val);
            }
            break;
        default:
            break;
    }
}

void free_jump_statement(struct jump_statement* s) {
    free_jump_statement_children(s);
    free(s);
}

struct labeled_statement* parse_labeled_statement(struct parser_state* s) {
    struct labeled_statement* res = xmalloc(sizeof(struct labeled_statement));
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->type) {
        case CASE: {
            res->type = LABELED_STATEMENT_CASE;
            accept_it(s);
            struct const_expr* case_expr = parse_const_expr(s);
            if (!case_expr) {
                free(res);
                return NULL;
            }
            res->case_expr = case_expr;
            break;
        }

        case IDENTIFIER: {
            res->type = LABELED_STATEMENT_LABEL;
            const struct str spelling = take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            res->label = create_identifier(&spelling, loc);
            break;
        }

        case DEFAULT: {
            res->type = LABELED_STATEMENT_DEFAULT;
            accept_it(s);
            res->case_expr = NULL;
            break;
        }

        default: {
            free(res);
            enum token_type expected[] = {IDENTIFIER, CASE, DEFAULT};

            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
    }

    if (!accept(s, COLON)) {
        goto fail;
    }

    res->stat = parse_statement(s);
    if (!res->stat) {
        goto fail;
    }

    return res;
fail:
    if (res->type == LABELED_STATEMENT_CASE) {
        free_const_expr(res->case_expr);
    }
    free(res);
    return NULL;
}

static void free_labeled_statement_children(struct labeled_statement* s) {
    switch (s->type) {
        case LABELED_STATEMENT_LABEL:
            free_identifier(s->label);
            break;
        case LABELED_STATEMENT_CASE:
            free_const_expr(s->case_expr);
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
        default:
            UNREACHABLE();
    }
    free_statement(s->stat);
}

void free_labeled_statement(struct labeled_statement* s) {
    free_labeled_statement_children(s);
    free(s);
}

struct selection_statement* parse_selection_statement(struct parser_state* s) {
    struct selection_statement* res = xmalloc(
        sizeof(struct selection_statement));
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == IF) {
        res->is_if = true;
        accept_it(s);
    } else {
        if (!accept(s, SWITCH)) {
            free(res);
            enum token_type expected[] = {IF, SWITCH};

            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
        res->is_if = false;
    }

    if (!accept(s, LBRACKET)) {
        free(res);
        return NULL;
    }

    res->sel_expr = parse_expr(s);
    if (!res->sel_expr) {
        free(res);
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_expr(res->sel_expr);
        free(res);
        return NULL;
    }

    res->sel_stat = parse_statement(s);
    if (!res->sel_stat) {
        free_expr(res->sel_expr);
        free(res);
        return NULL;
    }

    if (res->is_if && s->it->type == ELSE) {
        accept_it(s);
        res->else_stat = parse_statement(s);
        if (!res->else_stat) {
            free_statement(res->sel_stat);
            free_expr(res->sel_expr);
            free(res);
            return NULL;
        }
    } else {
        res->else_stat = NULL;
    }

    return res;
}

static void free_selection_statement_children(struct selection_statement* s) {
    free_expr(s->sel_expr);
    free_statement(s->sel_stat);
    if (s->else_stat) {
        free_statement(s->else_stat);
    }
}

void free_selection_statement(struct selection_statement* s) {
    free_selection_statement_children(s);
    free(s);
}

bool parse_statement_inplace(struct parser_state* s, struct statement* res) {
    assert(res);

    switch (s->it->type) {
        case LBRACE: {
            res->type = STATEMENT_COMPOUND;
            res->comp = parse_compound_statement(s);
            if (!res->comp) {
                return false;
            }
            break;
        }

        case FOR:
        case WHILE:
        case DO: {
            res->type = STATEMENT_ITERATION;
            res->it = parse_iteration_statement(s);
            if (!res->it) {
                return false;
            }
            break;
        }

        case GOTO:
        case CONTINUE:
        case BREAK:
        case RETURN: {
            res->type = STATEMENT_JUMP;
            res->jmp = parse_jump_statement(s);
            if (!res->jmp) {
                return false;
            }
            break;
        }

        case IF:
        case SWITCH: {
            res->type = STATEMENT_SELECTION;
            res->sel = parse_selection_statement(s);
            if (!res->sel) {
                return false;
            }
            break;
        }

        case CASE:
        case DEFAULT: {
            res->type = STATEMENT_LABELED;
            res->labeled = parse_labeled_statement(s);
            if (!res->labeled) {
                return false;
            }
            break;
        }

        case IDENTIFIER: {
            if (s->it[1].type == COLON) {
                res->type = STATEMENT_LABELED;
                res->labeled = parse_labeled_statement(s);
                if (!res->labeled) {
                    return false;
                }
                break;
            }
            goto is_stat_expr; // to avoid fallthrough warning
        }

is_stat_expr:
        default: {
            res->type = STATEMENT_EXPRESSION;
            res->expr = parse_expr_statement(s);
            if (!res->expr) {
                return false;
            }
            break;
        }
    }
    return true;
}

struct statement* parse_statement(struct parser_state* s) {
    struct statement* res = xmalloc(sizeof(struct statement));
    if (!parse_statement_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_statement_children(struct statement* s) {
    switch (s->type) {
        case STATEMENT_LABELED:
            free_labeled_statement(s->labeled);
            break;
        case STATEMENT_COMPOUND:
            free_compound_statement(s->comp);
            break;
        case STATEMENT_EXPRESSION:
            free_expr_statement(s->expr);
            break;
        case STATEMENT_SELECTION:
            free_selection_statement(s->sel);
            break;
        case STATEMENT_ITERATION:
            free_iteration_statement(s->it);
            break;
        case STATEMENT_JUMP:
            free_jump_statement(s->jmp);
            break;
    }
}

void free_statement(struct statement* s) {
    free_statement_children(s);
    free(s);
}

struct translation_unit parse_tokens(struct token* tokens,
                                     struct parser_err* err) {
    assert(tokens);

    struct parser_state state = create_parser_state(tokens, err);
    struct translation_unit res = parse_translation_unit(&state);
    free_parser_state(&state);
    return res;
}

struct parser_err create_parser_err(void) {
    return (struct parser_err){
        .type = PARSER_ERR_NONE,
    };
}

void set_parser_err(struct parser_err* err,
                    enum parser_err_type type,
                    struct source_loc loc) {
    assert(err);
    assert(type != PARSER_ERR_NONE);
    assert(err->type == PARSER_ERR_NONE);

    err->type = type;
    err->base = create_err_base(loc);
}

void print_parser_err(FILE* out, const struct file_info* file_info, const struct parser_err* err) {
    assert(err->type != PARSER_ERR_NONE);

    print_err_base(out, file_info, &err->base);
    switch (err->type) {
        case PARSER_ERR_NONE:
            UNREACHABLE();
        case PARSER_ERR_EXPECTED_TOKENS: {
            fprintf(out,
                    "Expected token of type %s",
                    get_type_str(err->expected[0]));
            for (size_t i = 1; i < err->num_expected; ++i) {
                printf(", %s", get_type_str(err->expected[i]));
            }

            if (err->got == INVALID) {
                fprintf(out, " but got to enf of file");
            } else {
                fprintf(out,
                        " but got token of type %s",
                        get_type_str(err->got));
            }
            break;
        }
        case PARSER_ERR_REDEFINED_SYMBOL: {
            assert(err->prev_def_file < file_info->len);
            const char* path = str_get_data(&file_info->paths[err->prev_def_file]);
            const char* type_str = err->was_typedef_name ? "typedef name"
                                                         : "enum constant";
            fprintf(out,
                    "Redefined symbol %s that was already defined as %s in %s(%zu, %zu)",
                    str_get_data(&err->redefined_symbol),
                    type_str,
                    path,
                    err->prev_def_loc.line,
                    err->prev_def_loc.index);
            break;
        }
        case PARSER_ERR_ARR_DOUBLE_STATIC:
            fprintf(out, "Expected only one use of static");
            break;
        case PARSER_ERR_ARR_STATIC_NO_LEN:
            fprintf(out, "Expected array length after use of static");
            break;
        case PARSER_ERR_ARR_STATIC_ASTERISK:
            fprintf(out, "Asterisk cannot be used with static");
            break;
        case PARSER_ERR_TYPEDEF_INIT:
            fprintf(out, "Initializer not allowed in typedef");
            break;
        case PARSER_ERR_TYPEDEF_FUNC_DEF:
            fprintf(out, "Function definition declared typedef");
            break;
        case PARSER_ERR_TYPEDEF_PARAM_DECL:
            fprintf(out, "typedef is not allowed in function declarator");
            break;
        case PARSER_ERR_TYPEDEF_STRUCT:
            fprintf(out, "typedef is not allowed in struct declaration");
            break;
        case PARSER_ERR_EMPTY_STRUCT_DECLARATOR:
            fprintf(out, "Expected a declarator or a bit field specifier");
            break;
        case PARSER_ERR_INCOMPATIBLE_TYPE_SPECS:
            fprintf(out,
                    "Cannot combine %s with previous %s type specifier",
                    get_type_str(err->type_spec),
                    get_type_str(err->prev_type_spec));
            break;
        case PARSER_ERR_DISALLOWED_TYPE_QUALS:
            fprintf(out,
                    "Cannot add qualifiers to type %s",
                    get_type_str(err->incompatible_type));
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            fprintf(
                out,
                "Expected a typedef name but got identifier with spelling %s",
                str_get_data(&err->non_typedef_spelling));
    }
    fprintf(out, "\n");
}

void free_parser_err(struct parser_err* err) {
    switch (err->type) {
        case PARSER_ERR_EXPECTED_TOKENS:
            free(err->expected);
            break;
        case PARSER_ERR_REDEFINED_SYMBOL:
            free_str(&err->redefined_symbol);
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            free_str(&err->non_typedef_spelling);
            break;
        default:
            break;
    }
}

enum identifier_type {
    ID_TYPE_NONE,
    ID_TYPE_TYPEDEF_NAME,
    ID_TYPE_ENUM_CONSTANT
};

/**
 * The pointers stored in this need to be valid at least as long
 * as the parser_state
 */
struct identifier_type_data {
    struct source_loc loc;
    enum identifier_type type;
};

enum {
    SCOPE_MAP_INIT_CAP = 100
};

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_type type);

static enum identifier_type get_item(const struct parser_state* s,
                                     const struct str* spell);

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err) {
    assert(tokens);

    struct parser_state res = {
        .it = tokens,
        ._len = 1,
        ._scope_maps = xmalloc(sizeof(struct string_hash_map)),
        .err = err,
    };
    res._scope_maps[0] = create_string_hash_map(
        sizeof(struct identifier_type_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
    return res;
}

void free_parser_state(struct parser_state* s) {
    for (size_t i = 0; i < s->_len; ++i) {
        free_string_hash_map(&s->_scope_maps[i]);
    }
    free(s->_scope_maps);
}

bool accept(struct parser_state* s, enum token_type expected) {
    if (s->it->type != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

void accept_it(struct parser_state* s) {
    assert(s->it->type != INVALID);
    ++s->it;
}

void parser_push_scope(struct parser_state* s) {
    ++s->_len;
    s->_scope_maps = xrealloc(s->_scope_maps,
                              sizeof(struct string_hash_map) * s->_len);
    s->_scope_maps[s->_len - 1] = create_string_hash_map(
        sizeof(struct identifier_type_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
}

void parser_pop_scope(struct parser_state* s) {
    assert(s->_len > 1);
    --s->_len;
    free_string_hash_map(&s->_scope_maps[s->_len]);
    s->_scope_maps = xrealloc(s->_scope_maps,
                              sizeof(struct string_hash_map) * s->_len);
}

bool register_enum_constant(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_ENUM_CONSTANT);
}

bool register_typedef_name(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_TYPEDEF_NAME);
}

bool is_enum_constant(const struct parser_state* s, const struct str* spell) {
    return get_item(s, spell) == ID_TYPE_ENUM_CONSTANT;
}

bool is_typedef_name(const struct parser_state* s, const struct str* spell) {
    return get_item(s, spell) == ID_TYPE_TYPEDEF_NAME;
}

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_type type) {
    assert(type != ID_TYPE_NONE);
    assert(token->type == IDENTIFIER);

    // TODO: Add a warning when an identifier from a previous scope is shadowed

    struct identifier_type_data to_insert = {
        .loc = token->loc,
        .type = type,
    };
    const struct identifier_type_data* item = string_hash_map_insert(
        &s->_scope_maps[s->_len - 1],
        &token->spelling,
        &to_insert);
    if (item != &to_insert) {
        set_parser_err(s->err, PARSER_ERR_REDEFINED_SYMBOL, token->loc);

        s->err->redefined_symbol = str_copy(&token->spelling);
        s->err->was_typedef_name = item->type == ID_TYPE_TYPEDEF_NAME;
        s->err->prev_def_file = item->loc.file_idx;
        s->err->prev_def_loc = item->loc.file_loc;
        return false;
    } else {
        return true;
    }
}

static enum identifier_type get_item(const struct parser_state* s,
                                     const struct str* spell) {
    for (size_t i = 0; i < s->_len; ++i) {
        const struct identifier_type_data* data = string_hash_map_get(
            &s->_scope_maps[i],
            spell);
        if (data != NULL) {
            return data->type;
        }
    }

    return ID_TYPE_NONE;
}

void expected_token_error(struct parser_state* s, enum token_type expected) {
    expected_tokens_error(s, &expected, 1);
}

void expected_tokens_error(struct parser_state* s,
                           const enum token_type* expected,
                           size_t num_expected) {
    assert(expected);
    assert(num_expected >= 1);

    set_parser_err(s->err, PARSER_ERR_EXPECTED_TOKENS, s->it->loc);

    const size_t bytes = sizeof(enum token_type) * num_expected;
    s->err->expected = xmalloc(bytes);
    memcpy(s->err->expected, expected, bytes);
    s->err->num_expected = num_expected;
    s->err->got = s->it->type;
}

static bool is_type_spec_token(const struct parser_state* s,
                               const struct token* token) {
    switch (token->type) {
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
        case IDENTIFIER:
            return is_typedef_name(s, &token->spelling);
        default:
            return false;
    }
}

bool next_is_type_name(const struct parser_state* s) {
    assert(s->it->type != INVALID);
    const struct token* next = s->it + 1;
    return is_type_spec_token(s, next) || is_type_qual(next->type)
           || (next->type == IDENTIFIER && is_typedef_name(s, &next->spelling));
}

bool is_type_spec(const struct parser_state* s) {
    return is_type_spec_token(s, s->it);
}

static bool is_declaration_spec(const struct parser_state* s) {
    return is_storage_class_spec(s->it->type) || is_type_spec(s)
           || is_type_qual(s->it->type) || is_func_spec(s->it->type)
           || s->it->type == ALIGNAS;
}

bool is_declaration(const struct parser_state* s) {
    return is_declaration_spec(s) || s->it->type == STATIC_ASSERT;
}

struct parse_float_const_res parse_float_const(const char* spell) {
    const char* end = spell; // so end is set
    assert(errno == 0);
    long double val = strtold(spell, (char**)&end);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (struct parse_float_const_res){
            .err.type = FLOAT_CONST_ERR_TOO_LARGE,
        };
    }
    enum value_type t = VALUE_DOUBLE;
    assert(spell <= end);
    if (*end != '\0') {
        if (*end == 'f' || *end == 'F') {
            t = VALUE_FLOAT;
        } else if (*end == 'l' || *end == 'L') {
            t = VALUE_LDOUBLE;
        } else {
            return (struct parse_float_const_res){
                .err =
                    {
                        .type = FLOAT_CONST_ERR_INVALID_CHAR,
                        .invalid_char = *end,
                    },
            };
        }
        ++end;
        if (*end != '\0') {
            return (struct parse_float_const_res){
                .err.type = FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
            };
        }
    }

    return (struct parse_float_const_res){
        .err.type = FLOAT_CONST_ERR_NONE,
        .res = create_float_value(t, val),
    };
}

void print_float_const_err(FILE* out, const struct float_const_err* err) {
    assert(err != FLOAT_CONST_ERR_NONE);

    switch (err->type) {
        case FLOAT_CONST_ERR_NONE:
            UNREACHABLE();
        case FLOAT_CONST_ERR_TOO_LARGE:
            fprintf(out, "floating constant too large to be represented");
            break;
        case FLOAT_CONST_ERR_SUFFIX_TOO_LONG:
            fprintf(out,
                    "floating constant suffix too long. Only one character is allowed in the suffix");
            break;
        case FLOAT_CONST_ERR_INVALID_CHAR:
            fprintf(out, "invalid character %c in suffix", err->invalid_char);
            break;
    }
    fprintf(out, "\n");
}

struct int_type_attrs {
    unsigned short num_long;
    bool is_unsigned;
};

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           struct int_const_err* err);

static enum value_type get_value_type_dec(struct int_type_attrs attrs,
                                          uintmax_t val,
                                          const struct arch_int_info* info,
                                          struct int_const_err* err);

static enum value_type get_value_type_other(struct int_type_attrs attrs,
                                            uintmax_t val,
                                            const struct arch_int_info* info,
                                            struct int_const_err* err);

struct parse_int_const_res parse_int_const(const char* spell,
                                           const struct arch_int_info* info) {
    const enum {
        DEC = 10,
        HEX = 16,
        OCT = 8
    } base = spell[0] == '0' ? ((tolower(spell[1]) == 'x') ? HEX : OCT) : DEC;
    const char* suffix = spell;
    assert(errno == 0);
    uintmax_t val = strtoull(spell, (char**)&suffix, base);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (struct parse_int_const_res){
            .err.type = INT_CONST_ERR_TOO_LARGE,
        };
    }

    assert(spell <= suffix);

    struct int_const_err err = {
        .type = INT_CONST_ERR_NONE,
    };
    struct int_type_attrs attrs = get_int_attrs(suffix, &err);
    if (err.type != INT_CONST_ERR_NONE) {
        return (struct parse_int_const_res){
            .err = err,
        };
    }
    const enum value_type
        type = base == DEC ? get_value_type_dec(attrs, val, info, &err)
                           : get_value_type_other(attrs, val, info, &err);
    if (err.type != INT_CONST_ERR_NONE) {
        return (struct parse_int_const_res){
            .err = err,
        };
    }
    assert(value_is_int(type) || value_is_uint(type));
    return (struct parse_int_const_res){
        .err.type = INT_CONST_ERR_NONE,
        .res = value_is_int(type) ? create_int_value(type, (intmax_t)val)
                                  : create_uint_value(type, val),
    };
}

void print_int_const_err(FILE* out, const struct int_const_err* err) {
    assert(out);
    assert(err);
    assert(err->type != INT_CONST_ERR_NONE);

    switch (err->type) {
        case INT_CONST_ERR_NONE:
            UNREACHABLE();
        case INT_CONST_ERR_TOO_LARGE:
            fprintf(out, "integer literal too large to be represented");
            break;
        case INT_CONST_ERR_SUFFIX_TOO_LONG:
            fprintf(out,
                    "integer literal suffix too long. The suffix may be a maximum of 3 characters");
            break;
        case INT_CONST_ERR_CASE_MIXING:
            fprintf(out, "ls in suffix must be the same case");
            break;
        case INT_CONST_ERR_U_BETWEEN_LS:
            fprintf(out, "u must not be between two ls in suffix");
            break;
        case INT_CONST_ERR_TRIPLE_LONG:
            fprintf(out, "l may only appear twice in suffix");
            break;
        case INT_CONST_ERR_DOUBLE_U:
            fprintf(out, "u may only appear once in suffix");
            break;
        case INT_CONST_ERR_INVALID_CHAR:
            fprintf(out,
                    "invalid character %c in integer literal",
                    err->invalid_char);
            break;
    }
    fprintf(out, "\n");
}

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           struct int_const_err* err) {
    struct int_type_attrs res = {
        .num_long = 0,
        .is_unsigned = false,
    };
    bool l_is_upper = false;
    bool last_was_u = false;
    for (size_t i = 0; suffix[i] != '\0'; ++i) {
        if (i == 3) {
            err->type = INT_CONST_ERR_SUFFIX_TOO_LONG;
            return (struct int_type_attrs){0};
        }
        switch (suffix[i]) {
            case 'l':
                if (res.num_long > 0 && l_is_upper) {
                    err->type = INT_CONST_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = INT_CONST_ERR_U_BETWEEN_LS;
                    return (struct int_type_attrs){0};
                } else if (res.num_long == 2) {
                    err->type = INT_CONST_ERR_TRIPLE_LONG;
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                }
                break;
            case 'L':
                if (res.num_long > 0 && !l_is_upper) {
                    err->type = INT_CONST_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = INT_CONST_ERR_U_BETWEEN_LS;
                    return (struct int_type_attrs){0};
                } else if (res.num_long == 2) {
                    err->type = INT_CONST_ERR_TRIPLE_LONG;
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                    l_is_upper = true;
                }
                break;
            case 'u':
            case 'U':
                if (res.is_unsigned) {
                    err->type = INT_CONST_ERR_DOUBLE_U;
                    return (struct int_type_attrs){0};
                } else {
                    res.is_unsigned = true;
                    last_was_u = true;
                }
                break;
            default:
                err->type = INT_CONST_ERR_INVALID_CHAR;
                err->invalid_char = suffix[i];
                return (struct int_type_attrs){0};
        }
    }
    return res;
}

enum {
    TARGET_CHAR_SIZE = CHAR_BIT,
};

static uintmax_t int_pow2(uintmax_t exp) {
    if (exp < sizeof(uintmax_t) * CHAR_BIT) {
        return 1ull << exp;
    }
    uintmax_t base = 2;
    uintmax_t res = 1;
    while (true) {
        if (exp & 1) {
            res *= base;
        }
        exp >>= 1;
        if (exp == 0) {
            break;
        }
        base *= base;
    }
    return res;
}

static uintmax_t max_uint(uintmax_t num_bits) {
    return int_pow2(num_bits) - 1;
}

static uintmax_t max_int(uintmax_t num_bits) {
    return int_pow2(num_bits - 1) - 1;
}

static uintmax_t get_max_int(const struct arch_int_info* info,
                             enum value_type type) {
    assert(type == VALUE_UINT || type == VALUE_ULINT || type == VALUE_ULLINT
           || type == VALUE_INT || type == VALUE_LINT || type == VALUE_LLINT);

    switch (type) {
        case VALUE_INT:
            return max_int(TARGET_CHAR_SIZE * info->int_size);
        case VALUE_UINT:
            return max_uint(TARGET_CHAR_SIZE * info->int_size);
        case VALUE_LINT:
            return max_int(TARGET_CHAR_SIZE * info->lint_size);
        case VALUE_ULINT:
            return max_uint(TARGET_CHAR_SIZE * info->lint_size);
        case VALUE_LLINT:
            return max_int(TARGET_CHAR_SIZE * info->llint_size);
        case VALUE_ULLINT:
            return max_uint(TARGET_CHAR_SIZE * info->llint_size);

        default:
            UNREACHABLE();
    }
}

static enum value_type get_value_type_unsigned(
    struct int_type_attrs attrs,
    uintmax_t val,
    const struct arch_int_info* info) {
    assert(attrs.is_unsigned);
    assert(attrs.num_long <= 2);

    switch (attrs.num_long) {
        case 0:
            if (val <= get_max_int(info, VALUE_UINT)) {
                return VALUE_UINT;
            }
            FALLTHROUGH();
        case 1:
            if (val <= get_max_int(info, VALUE_ULINT)) {
                return VALUE_ULINT;
            }
            FALLTHROUGH();
        case 2:
            if (val <= get_max_int(info, VALUE_ULLINT)) {
                return VALUE_ULLINT;
            } else {
                // unsigned will throw error in strotull
                UNREACHABLE();
            }
        default:
            UNREACHABLE();
    }
}

static enum value_type get_value_type_dec(struct int_type_attrs attrs,
                                          uintmax_t val,
                                          const struct arch_int_info* info,
                                          struct int_const_err* err) {
    assert(attrs.num_long <= 2);
    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(info, VALUE_INT)) {
                    return VALUE_INT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(info, VALUE_LINT)) {
                    return VALUE_LINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(info, VALUE_LLINT)) {
                    return VALUE_LLINT;
                } else {
                    err->type = INT_CONST_ERR_TOO_LARGE;
                    return VALUE_UINT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static enum value_type get_value_type_other(struct int_type_attrs attrs,
                                            uintmax_t val,
                                            const struct arch_int_info* info,
                                            struct int_const_err* err) {
    assert(attrs.num_long <= 2);

    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(info, VALUE_INT)) {
                    return VALUE_INT;
                } else if (val <= get_max_int(info, VALUE_UINT)) {
                    return VALUE_UINT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(info, VALUE_LINT)) {
                    return VALUE_LINT;
                } else if (val <= get_max_int(info, VALUE_ULINT)) {
                    return VALUE_ULINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(info, VALUE_LLINT)) {
                    return VALUE_LLINT;
                } else if (val <= get_max_int(info, VALUE_ULLINT)) {
                    return VALUE_ULLINT;
                } else {
                    err->type = INT_CONST_ERR_TOO_LARGE;
                    return VALUE_INT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static enum value_type get_uint_leastn_t_type(
    size_t n,
    const struct arch_int_info* info); 

struct parse_char_const_res parse_char_const(const char* spell,
                                             const struct arch_int_info* info) {
    assert(spell);
    assert(info);

    enum value_type type;
    switch (*spell) {
        case '\'':
            type = VALUE_INT;
            ++spell;
            break;
        case 'u':
            if (spell[1] == '8') {
                type = VALUE_UCHAR;
            } else if (spell[1] == '\'') {
                type = get_uint_leastn_t_type(16, info);
            } else {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 2,
                            .expected_chars = {'8', '\''},
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'U':
            type = get_uint_leastn_t_type(32, info);
            if (spell[1] != '\'') {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'L':
            // TODO: handle wchar_t stuff
            type = get_uint_leastn_t_type(32, info);
            if (spell[1] != '\'') {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        default:
            return (struct parse_char_const_res){
                .err =
                    {
                        .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 4,
                        .expected_chars = {'\'', 'u', 'U', 'L'},
                        .got_char = *spell,
                    },
            };
    }

    if (value_is_uint(type)) {
        uintmax_t val;
        switch (*spell) {
            case '\\':
                ++spell;
                switch (*spell) {
                    case 'a':
                        val = '\a';
                        break;
                    case 'b':
                        val = '\b';
                        break;
                    case 'f':
                        val = '\f';
                        break;
                    case 'n':
                        val = '\n';
                        break;
                    case 'r':
                        val = '\r';
                        break;
                    case 't':
                        val = '\t';
                        break;
                    case 'v':
                        val = '\v';
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                    case '\?':
                        val = *spell;
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (struct parse_char_const_res) {
                            .err = {
                                .type = CHAR_CONST_ERR_INVALID_ESCAPE,
                                .invalid_escape = *spell,
                            },
                        };
                }
                break;
            default:
                val = *spell;
                break;
        }

        ++spell;
        if (*spell != '\'') {
            return (struct parse_char_const_res) {
                .err = {
                    .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                    .num_expected = 1,
                    .expected_chars[0] = '\'',
                    .got_char = *spell,
                },
            };
        }
        assert(spell[1] == '\0');

        return (struct parse_char_const_res){
            .err.type = CHAR_CONST_ERR_NONE,
            .res = create_uint_value(type, val),
        };
    } else {
        assert(value_is_int(type));
        intmax_t val;
        switch (*spell) {
            case '\\':
                ++spell;
                switch (*spell) {
                    case 'a':
                        val = '\a';
                        break;
                    case 'b':
                        val = '\b';
                        break;
                    case 'f':
                        val = '\f';
                        break;
                    case 'n':
                        val = '\n';
                        break;
                    case 'r':
                        val = '\r';
                        break;
                       case 't':
                        val = '\t';
                        break;
                    case 'v':
                        val = '\v';
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                    case '\?':
                        val = *spell;
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (struct parse_char_const_res) {
                            .err = {
                                .type = CHAR_CONST_ERR_INVALID_ESCAPE,
                                .invalid_escape = *spell,
                            },
                        };
                }
                break;
            default:
                val = *spell;
                break;
        }

        ++spell;
        if (*spell != '\'') {
            return (struct parse_char_const_res) {
                .err = {
                    .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                    .num_expected = 1,
                    .expected_chars[0] = '\'',
                    .got_char = *spell,
                },
            };
        }
        assert(spell[1] == '\0');

        return (struct parse_char_const_res){
            .err.type = CHAR_CONST_ERR_NONE,
            .res = create_int_value(type, val),
        };
    }
}

void print_char_const_err(FILE* out, const struct char_const_err* err) {
    assert(err->type != CHAR_CONST_ERR_NONE);
    switch (err->type) {
        case CHAR_CONST_ERR_NONE:
            UNREACHABLE();
        case CHAR_CONST_ERR_EXPECTED_CHAR:
            fprintf(out, "Expected ");
            const uint8_t limit = err->num_expected - 1;
            for (uint8_t i = 0; i < limit; ++i) {
                fprintf(out, "%c, ", err->expected_chars[i]);
            }
            fprintf(out, " or %c but got %c", err->expected_chars[limit], err->got_char);
            break;
        case CHAR_CONST_ERR_INVALID_ESCAPE:
            fprintf(out, "Invalid escape character %c", err->invalid_escape);
            break;
    }
    fprintf(out, "\n");
}

static enum value_type get_uint_leastn_t_type(
    size_t n,
    const struct arch_int_info* info) {
    assert(n == 8 || n == 16 || n == 32 || n == 64);

    if (TARGET_CHAR_SIZE >= n) {
        return VALUE_UCHAR;
    } else if (TARGET_CHAR_SIZE * info->sint_size >= n) {
        return VALUE_USINT;
    } else if (TARGET_CHAR_SIZE * info->int_size >= n) {
        return VALUE_UINT;
    } else if (TARGET_CHAR_SIZE * info->lint_size >= n) {
        return VALUE_ULINT;
    } else if (TARGET_CHAR_SIZE * info->llint_size >= n) {
        return VALUE_ULLINT;
    }
    UNREACHABLE();
}

/**
 * Tokenizes a line of source code for the preprocessor, meaning that
 * keywords are treated as identifiers
 *
 * @param line_num The number of the current line in the file
 * @param comment_not_terminated A pointer to a boolean, that signifies whether
 *        a multi-line comment was started in a previous line
 */
bool tokenize_line(struct token_arr* res,
                   struct preproc_err* err,
                   const char* line,
                   size_t line_num,
                   size_t file_idx,
                   bool* comment_not_terminated);

void file_read_line(FILE* file,
                      char** res,
                      size_t* res_len,
                      char* static_buf,
                      size_t static_buf_len);


static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc include_loc);

static enum token_type keyword_type(const char* spelling);

static void append_terminator_token(struct token_arr* arr);

struct preproc_res preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state(path, err);

    if (!preproc_file(&state, path, (struct source_loc){(size_t)-1, {0, 0}})) {
        struct file_info info = state.file_info;
        state.file_info = (struct file_info){
            .len = 0,
            .paths = NULL,
        };
        free_preproc_state(&state);
        return (struct preproc_res){
            .toks = NULL,
            .file_info = info,
        };
    }

    struct preproc_res res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (struct token_arr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (struct file_info){
        .len = 0,
        .paths = NULL,
    };
    free_preproc_state(&state);
    return res;
}

enum {
    PREPROC_LINE_BUF_LEN = 200
};

struct code_source {
    bool is_file;
    union {
        FILE* file;
        const char* str;
    };
    const char* path;
    size_t current_line;
    bool comment_not_terminated;
};

static bool code_source_over(struct code_source* src) {
    if (src->is_file) {
        return feof(src->file);
    } else {
        return *src->str == '\0';
    }
}

static void string_read_line(const char** str,
                             char** res,
                             size_t* res_len,
                             char* static_buf,
                             size_t static_buf_len) {

    const char* start = *str;
    const char* it = *str;
    if (*res_len < static_buf_len) {
        *res = static_buf;
        bool use_dyn_buf = false;
        while (*it != '\n' && *it != '\0') {
            ++it;
            if (*res_len == static_buf_len - 1) {
                use_dyn_buf = true;
                break;
            }
        }

        if (!use_dyn_buf) {
            if (it == start && *res_len == 0) {
                *res = NULL;
            } else {
                const size_t len = it - start;
                *str = *it == '\0' ? it : it + 1;
                memcpy(static_buf + *res_len, start, sizeof(char) * len);
                *res_len += len;
                static_buf[len] = '\0';
            }
            return;
        }
    }

    while (*it != '\n' && *it != '\0') {
        ++it;
    }

    *str = *it == '\0' ? it : it + 1;
    const size_t len = it - start;
    const size_t prev_res_len = *res_len;
    *res_len += len;
    if (len == 0) {
        return;
    }
    *res = xrealloc(*res, sizeof(char) * (*res_len + 1));
    memcpy(*res + prev_res_len, start, len * sizeof(char));
    (*res)[*res_len] = '\0';
}

static char* code_source_read_line(struct code_source* src,
                                   char static_buf[PREPROC_LINE_BUF_LEN]) {
    char* res = NULL;
    bool escaped_newline = false;
    size_t len = 0;
    do {
        if (src->is_file) {
            file_read_line(src->file,
                           &res,
                           &len,
                           static_buf,
                           PREPROC_LINE_BUF_LEN);
        } else {
            string_read_line(&src->str,
                             &res,
                             &len,
                             static_buf,
                             PREPROC_LINE_BUF_LEN);
        }

        if (res != NULL && len > 0) {
            escaped_newline = res[len - 1] == '\\';
            // TODO: newlines not contained when escaped newline is found
        }
        ++src->current_line;
    } while (escaped_newline);

    return res;
}

static bool is_preproc_directive(const char* line) {
    const char* it = line;
    while (isspace(*it)) {
        ++it;
    }

    return *it == '#';
}

static bool preproc_statement(struct preproc_state* state,
                              struct code_source* src,
                              struct token_arr* arr);

// TODO: what to do if the line is a preprocessor directive
// Maybe just handle preprocessor directives until we reach an "actual" line
static bool read_and_tokenize_line(struct preproc_state* state,
                                   struct code_source* src) {
    assert(src);

    while (true) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src, static_buf);
        if (line == NULL) {
            return true;
        }

        if (is_preproc_directive(line)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };

            const bool res = tokenize_line(&arr,
                                           state->err,
                                           line,
                                           prev_curr_line,
                                           state->file_info.len - 1,
                                           &src->comment_not_terminated);
            if (line != static_buf) {
                free(line);
            }
            if (!res) {
                return false;
            }

            const bool stat_res = preproc_statement(state, src, &arr);
            free_token_arr(&arr);
            if (!stat_res) {
                return false;
            }
        } else {
            bool res = tokenize_line(&state->res,
                                     state->err,
                                     line,
                                     prev_curr_line,
                                     state->file_info.len - 1,
                                     &src->comment_not_terminated);
            if (line != static_buf) {
                free(line);
            }
            if (!res) {
                return false;
            }

            break;
        }
    }

    return true;
}

static size_t find_macro_end(struct preproc_state* state,
                             const struct token* macro_start,
                             struct code_source* src) {
    const struct token* it = macro_start;
    assert(it->type == IDENTIFIER);
    ++it;
    assert(it->type == LBRACKET);
    ++it;

    size_t open_bracket_count = 0;
    while (!code_source_over(src)
           && (open_bracket_count != 0 || it->type != RBRACKET)) {
        while (!code_source_over(src)
               && it == state->res.tokens + state->res.len) {
            if (!read_and_tokenize_line(state, src)) {
                return (size_t)-1;
            }
        }

        if (code_source_over(src) && it == state->res.tokens + state->res.len) {
            break;
        }

        if (it->type == LBRACKET) {
            ++open_bracket_count;
        } else if (it->type == RBRACKET) {
            --open_bracket_count;
        }
        ++it;
    }

    if (it->type != RBRACKET) {
        set_preproc_err(state->err,
                        PREPROC_ERR_UNTERMINATED_MACRO,
                        macro_start->loc);
        return (size_t)-1;
    }
    return it - state->res.tokens;
}

static bool expand_all_macros(struct preproc_state* state,
                              size_t start,
                              struct code_source* src) {
    bool no_incr = false;
    for (size_t i = start; i < state->res.len; ++i) {
        if (no_incr) {
            --i;
            no_incr = false;
        }
        const struct token* curr = &state->res.tokens[i];
        if (curr->type == IDENTIFIER) {
            const struct preproc_macro* macro = find_preproc_macro(
                state,
                &curr->spelling);
            if (macro != NULL) {
                size_t macro_end;
                if (macro->is_func_macro) {
                    const size_t next_idx = i + 1;
                    if (next_idx < state->res.len
                        && state->res.tokens[next_idx].type == LBRACKET) {
                        macro_end = find_macro_end(state, curr, src);
                        if (state->err == PREPROC_ERR_NONE) {
                            return false;
                        }
                    } else {
                        continue;
                    }
                } else {
                    macro_end = (size_t)-1;
                }
                if (!expand_preproc_macro(state,
                                          &state->res,
                                          macro,
                                          i,
                                          macro_end)) {
                    return false;
                }
                // need to continue at the start of the macro expansion
                no_incr = true;
            }
        }
    }

    return true;
}

static bool preproc_src(struct preproc_state* state, struct code_source* src) {
    while (!code_source_over(src)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, src)) {
            return false;
        }

        if (!expand_all_macros(state, prev_len, src)) {
            return false;
        }
    }

    append_terminator_token(&state->res);
    return true;
}

struct preproc_res preproc_string(const char* str,
                                  const char* path,
                                  struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state(path, err);

    struct code_source src = {
        .is_file = false,
        .str = str,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };

    if (!preproc_src(&state, &src)) {
        free_preproc_state(&state);
        return (struct preproc_res){
            .toks = NULL,
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
        };
    }

    struct preproc_res res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (struct token_arr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (struct file_info){
        .len = 0,
        .paths = NULL,
    };
    free_preproc_state(&state);

    return res;
}

static void file_err(struct preproc_err* err,
                     size_t fail_file,
                     struct source_loc include_loc,
                     bool open_fail);

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc include_loc) {
    FILE* file = fopen(path, "r");
    if (!file) {
        file_err(state->err, state->file_info.len - 1, include_loc, true);
        return false;
    }

    struct code_source src = {
        .is_file = true,
        .file = file,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };

    const bool res = preproc_src(state, &src);

    if (!res) {
        // TODO: what to do if closing fails
        fclose(file);
        return false;
    }

    if (fclose(file) != 0) {
        file_err(state->err, state->file_info.len - 1, include_loc, false);
        return false;
    }

    return true;
}

static void file_err(struct preproc_err* err,
                     size_t fail_file,
                     struct source_loc include_loc,
                     bool open_fail) {
    assert(fail_file != (size_t)-1);

    set_preproc_err(err, PREPROC_ERR_FILE_FAIL, include_loc);
    err->fail_file = fail_file;
    err->open_fail = open_fail;
}

static void free_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    free(tokens);
}

static void free_preproc_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_str(&it->spelling);
    }
    free(tokens);
}

void free_preproc_res_preproc_tokens(struct preproc_res* res) {
    free_preproc_tokens(res->toks);
    free_file_info(&res->file_info);
}

void free_preproc_res(struct preproc_res* res) {
    free_tokens(res->toks);
    free_file_info(&res->file_info);
}

bool convert_preproc_tokens(struct token* tokens,
                            const struct arch_int_info* info,
                            struct preproc_err* err) {
    assert(tokens);
    assert(info);
    for (struct token* t = tokens; t->type != INVALID; ++t) {
        switch (t->type) {
            case I_CONSTANT: {
                if (str_char_at(&t->spelling, 0) == '\'') {
                    struct parse_char_const_res res = parse_char_const(str_get_data(&t->spelling), info);
                    if (res.err.type != CHAR_CONST_ERR_NONE) {
                        set_preproc_err(err, PREPROC_ERR_CHAR_CONST, t->loc);
                        err->char_const_err = res.err;
                        err->constant_spell = t->spelling;
                        return false;
                    }
                    free_str(&t->spelling);
                    t->val = res.res;
                } else {
                    struct parse_int_const_res res = parse_int_const(
                        str_get_data(&t->spelling),
                        info);
                    if (res.err.type != INT_CONST_ERR_NONE) {
                        set_preproc_err(err, PREPROC_ERR_INT_CONST, t->loc);
                        err->int_const_err = res.err;
                        err->constant_spell = t->spelling;
                        return false;
                    }
                    free_str(&t->spelling);
                    t->val = res.res;
                }
                break;
            }
            case F_CONSTANT: {
                struct parse_float_const_res res = parse_float_const(
                    str_get_data(&t->spelling));
                if (res.err.type != FLOAT_CONST_ERR_NONE) {
                    set_preproc_err(err, PREPROC_ERR_FLOAT_CONST, t->loc);
                    err->float_const_err = res.err;
                    err->constant_spell = t->spelling;
                    return false;
                }
                free_str(&t->spelling);
                t->val = res.res;
                break;
            }
            case IDENTIFIER:
                t->type = keyword_type(str_get_data(&t->spelling));
                if (t->type != IDENTIFIER) {
                    free_str(&t->spelling);
                    t->spelling = create_null_str();
                }
                break;
            case STRINGIFY_OP:
            case CONCAT_OP:
                set_preproc_err(err,
                                PREPROC_ERR_MISPLACED_PREPROC_TOKEN,
                                t->loc);
                err->misplaced_preproc_tok = t->type;
                return false;
            default:
                break;
        }
    }
    return true;
}

static void append_terminator_token(struct token_arr* arr) {
    arr->tokens = xrealloc(arr->tokens, sizeof(struct token) * (arr->len + 1));
    arr->tokens[arr->len] = (struct token){
        .type = INVALID,
        .spelling = create_null_str(),
        .loc =
            {
                .file_idx = (size_t)-1,
                .file_loc = {(size_t)-1, (size_t)-1},
            },
    };
}

static bool is_cond_directive(const char* line) {
    const char* it = line;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    if (*it != '#') {
        return false;
    }

    ++it;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    const char else_dir[] = "else";
    const char elif_dir[] = "elif";
    const char endif_dir[] = "endif";

    if (*it == '\0') {
        return false;
    } else if (strncmp(it, else_dir, sizeof(else_dir)) == 0
               || strncmp(it, elif_dir, sizeof(elif_dir)) == 0
               || strncmp(it, endif_dir, sizeof(endif_dir))) {
        return true;
    }

    UNREACHABLE();
}

// TODO: could probably be optimized
static bool skip_until_next_cond(struct preproc_state* state,
                                 struct code_source* src) {
    while (!code_source_over(src)) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src, static_buf);
        if (is_cond_directive(line)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };
            const bool res = tokenize_line(&arr,
                                           state->err,
                                           line,
                                           prev_curr_line,
                                           state->file_info.len - 1,
                                           &src->comment_not_terminated);

            if (line != static_buf) {
                free(line);
            }

            if (!res) {
                return false;
            }

            bool stat_res = preproc_statement(state, src, &arr);
            free_token_arr(&arr);
            return stat_res;
        }

        if (line != static_buf) {
            free(line);
        }
    }

    return true;
}

static bool handle_preproc_if(struct preproc_state* state,
                              struct code_source* src,
                              bool cond) {
    struct source_loc loc = {
        .file_loc = {src->current_line, 0}, // TODO: might be current_line - 1
        .file_idx = state->file_info.len - 1,
    };
    push_preproc_cond(state, loc, cond);

    if (!cond) {
        return skip_until_next_cond(state, src);
    }

    return true;
}

static bool handle_ifdef_ifndef(struct preproc_state* state,
                                struct code_source* src,
                                struct token_arr* arr,
                                bool is_ifndef) {
    assert(arr);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    assert((!is_ifndef && strcmp(str_get_data(&arr->tokens[1].spelling), "ifdef") == 0)
           || (is_ifndef && strcmp(str_get_data(&arr->tokens[1].spelling), "ifndef") == 0));

    if (arr->len < 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[1].loc);
        state->err->count_empty = true;
        state->err->count_dir_type = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    } else if (arr->len > 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[3].loc);
        state->err->count_empty = false;
        state->err->count_dir_type = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    }
    if (arr->tokens[2].type != IDENTIFIER) {
        set_preproc_err(state->err,
                        PREPROC_ERR_NOT_IDENTIFIER,
                        arr->tokens[2].loc);
        state->err->not_identifier_got = arr->tokens[2].type;
        state->err->not_identifier_op = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                                  : SINGLE_MACRO_OP_IFDEF;
        return false;
    }

    const struct str* macro_spell = &arr->tokens[2].spelling;
    assert(macro_spell);
    assert(str_is_valid(macro_spell));
    const struct preproc_macro* macro = find_preproc_macro(state, macro_spell);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, src, cond);
}

static bool handle_else_elif(struct preproc_state* state,
                             struct code_source* src,
                             struct token_arr* arr,
                             bool is_else) {
    assert(arr->len > 1);
    if (state->conds_len == 0) {
        set_preproc_err(state->err, PREPROC_ERR_MISSING_IF, arr->tokens[1].loc);
        state->err->missing_if_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        return false;
    }

    struct preproc_cond* curr_if = peek_preproc_cond(state);
    if (curr_if->had_else) {
        set_preproc_err(state->err,
                        PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
                        arr->tokens[1].loc);
        state->err->elif_after_else_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        state->err->prev_else_loc = curr_if->loc;
        return false;
    } else if (curr_if->had_true_branch) {
        return skip_until_next_cond(state, src);
    } else if (is_else) {
        curr_if->had_else = true;
        // TODO: just continue
    } else {
        // TODO: evaluate condition
        (void)arr;
    }

    return false;
}

static bool preproc_statement(struct preproc_state* state,
                              struct code_source* src,
                              struct token_arr* arr) {
    assert(arr);
    assert(arr->tokens);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    if (arr->len == 1) {
        return true;
    } else if (arr->tokens[1].type != IDENTIFIER) {
        set_preproc_err(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    const char* directive = str_get_data(&arr->tokens[1].spelling);
    assert(directive);

    if (strcmp(directive, "if") == 0) {
        // TODO:
    } else if (strcmp(directive, "ifdef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, false);
    } else if (strcmp(directive, "ifndef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, true);
    } else if (strcmp(directive, "define") == 0) {
        const struct str spell = take_spelling(&arr->tokens[2]);
        struct preproc_macro macro = parse_preproc_macro(arr,
                                                         str_get_data(&spell),
                                                         state->err);
        if (state->err->type != PREPROC_ERR_NONE) {
            return false;
        }
        register_preproc_macro(state, &spell, &macro);
    } else if (strcmp(directive, "undef") == 0) {
        if (arr->len < 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[1].loc);
            state->err->count_empty = true;
            state->err->count_dir_type = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->len > 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[3].loc);
            state->err->count_empty = false;
            state->err->count_dir_type = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->tokens[2].type != IDENTIFIER) {
            set_preproc_err(state->err,
                            PREPROC_ERR_NOT_IDENTIFIER,
                            arr->tokens[2].loc);
            state->err->not_identifier_got = arr->tokens[2].type;
            state->err->not_identifier_op = SINGLE_MACRO_OP_UNDEF;
            return false;
        }

        remove_preproc_macro(state, &arr->tokens[2].spelling);
    } else if (strcmp(directive, "include") == 0) {
        // TODO:
    } else if (strcmp(directive, "pragma") == 0) {
        // TODO:
    } else if (strcmp(directive, "elif") == 0) {
        return handle_else_elif(state, src, arr, false);
    } else if (strcmp(directive, "else") == 0) {
        return handle_else_elif(state, src, arr, true);
    } else if (strcmp(directive, "endif") == 0) {
        if (state->conds_len == 0) {
            set_preproc_err(state->err,
                            PREPROC_ERR_MISSING_IF,
                            arr->tokens[1].loc);
            state->err->missing_if_op = ELSE_OP_ENDIF;
            return false;
        }

        pop_preproc_cond(state);
    } else {
        set_preproc_err(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    return true;
}

static inline bool is_spelling(const char* spelling, enum token_type type) {
    const char* expected_spell = get_spelling(type);
    assert(expected_spell != NULL);
    return strcmp(spelling, expected_spell) == 0;
}

static enum token_type keyword_type(const char* spell) {
    for (enum token_type e = KEYWORDS_START; e < KEYWORDS_END; ++e) {
        if (is_spelling(spell, e)) {
            return e;
        }
    }

    return IDENTIFIER;
}

struct preproc_err create_preproc_err(void) {
    return (struct preproc_err){
        .type = PREPROC_ERR_NONE,
    };
}

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE);

    err->type = type;
    err->base = create_err_base(loc);
}

static const char* get_single_macro_op_str(enum single_macro_op_type type);
static const char* get_else_op_str(enum else_op_type type);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err) {
    assert(err->type != PREPROC_ERR_NONE);

    switch (err->type) {
        case PREPROC_ERR_NONE:
            UNREACHABLE();
            break;
        case PREPROC_ERR_FILE_FAIL:
            if (err->base.loc.file_idx != (size_t)-1) {
                print_err_base(out, file_info, &err->base);
            }

            assert(err->fail_file < file_info->len);
            const char* fail_path = str_get_data(&file_info->paths[err->fail_file]);
            if (err->open_fail) {
                fprintf(out, "Failed to open file %s", fail_path);
            } else {
                fprintf(out, "Failed to close file %s", fail_path);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_LIT:
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "%s literal not properly terminated",
                    err->is_char_lit ? "Char" : "String");
            break;
        case PREPROC_ERR_INVALID_ID:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Invalid identifier: %s", str_get_data(&err->invalid_id));
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Invalid number: %s", str_get_data(&err->invalid_num));
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
            print_err_base(out, file_info, &err->base);
            if (err->too_few_args) {
                fprintf(out,
                        "Too few arguments in function-like macro invocation: Expected%s %zu arguments",
                        err->is_variadic ? " at least" : "",
                        err->expected_arg_count);
            } else {
                fprintf(out,
                        "Too many arguments in function like macro invocation: Expected only %zu arguments",
                        err->expected_arg_count);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_MACRO:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Unterminated macro invocation");
            break;
        case PREPROC_ERR_ARG_COUNT: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_single_macro_op_str(err->count_dir_type);
            if (err->count_empty) {
                fprintf(out,
                        "Expected an identifier after %s directive",
                        dir_str);
            } else {
                fprintf(out, "Excess tokens after %s directive", dir_str);
            }
            break;
        }
        case PREPROC_ERR_NOT_IDENTIFIER: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_single_macro_op_str(
                err->not_identifier_op);
            fprintf(out,
                    "Expected an identifier after %s directive, but got %s",
                    dir_str,
                    get_type_str(err->not_identifier_got));
            break;
        }
        case PREPROC_ERR_MISSING_IF: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_else_op_str(err->missing_if_op);
            fprintf(out, "%s directive without if", dir_str);
            break;
        }
        case PREPROC_ERR_INVALID_PREPROC_DIR:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Invalid preprocessor directive");
            break;
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE: {
            assert(err->elif_after_else_op != ELSE_OP_ENDIF);
            print_err_base(out, file_info, &err->base);
            const char*
                prev_else_file = str_get_data(&file_info->paths[err->prev_else_loc.file_idx]);
            const struct file_loc loc = err->prev_else_loc.file_loc;
            switch (err->elif_after_else_op) {
                case ELSE_OP_ELIF:
                    fprintf(
                        out,
                        "elif directive after else directive in %s:(%zu,%zu)",
                        prev_else_file,
                        loc.line,
                        loc.index);
                    break;
                case ELSE_OP_ELSE:
                    fprintf(out,
                            "Second else directive after else in %s:(%zu,%zu)",
                            prev_else_file,
                            loc.line,
                            loc.index);
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case PREPROC_ERR_MISPLACED_PREPROC_TOKEN:
            assert(err->misplaced_preproc_tok == STRINGIFY_OP
                   || err->misplaced_preproc_tok == CONCAT_OP);
            fprintf(
                out,
                "preprocessor token \"%s\" outside of preprocessor directive",
                get_spelling(err->misplaced_preproc_tok));
            break;
        case PREPROC_ERR_INT_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Integer constant %s is not a valid integer constant", str_get_data(&err->constant_spell));
            print_int_const_err(out, &err->int_const_err);
            break;
        case PREPROC_ERR_FLOAT_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Floating constant %s is not a valid integer constant", str_get_data(&err->constant_spell));
            print_float_const_err(out, &err->float_const_err);
            break;
        case PREPROC_ERR_CHAR_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Character constant %s is not a valid character constant", str_get_data(&err->constant_spell));
            print_char_const_err(out, &err->char_const_err);
            break;
    }
    fprintf(out, "\n");
}

static const char* get_single_macro_op_str(enum single_macro_op_type type) {
    switch (type) {
        case SINGLE_MACRO_OP_IFDEF:
            return "ifdef";
        case SINGLE_MACRO_OP_IFNDEF:
            return "ifndef";
        case SINGLE_MACRO_OP_UNDEF:
            return "undef";
    }

    UNREACHABLE();
}

static const char* get_else_op_str(enum else_op_type type) {
    switch (type) {
        case ELSE_OP_ELIF:
            return "elif";
        case ELSE_OP_ELSE:
            return "else";
        case ELSE_OP_ENDIF:
            return "endif";
    }

    UNREACHABLE();
}

void free_preproc_err(struct preproc_err* err) {
    assert(err);

    switch (err->type) {
        case PREPROC_ERR_INVALID_ID:
            free_str(&err->invalid_id);
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            free_str(&err->invalid_num);
            break;
        case PREPROC_ERR_INT_CONST:
        case PREPROC_ERR_FLOAT_CONST:
        case PREPROC_ERR_CHAR_CONST:
            free_str(&err->constant_spell);
            break;
        case PREPROC_ERR_FILE_FAIL:
        case PREPROC_ERR_MACRO_ARG_COUNT:
        case PREPROC_ERR_NONE:
        case PREPROC_ERR_UNTERMINATED_LIT:
        case PREPROC_ERR_UNTERMINATED_MACRO:
        case PREPROC_ERR_ARG_COUNT:
        case PREPROC_ERR_NOT_IDENTIFIER:
        case PREPROC_ERR_MISSING_IF:
        case PREPROC_ERR_INVALID_PREPROC_DIR:
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE:
        case PREPROC_ERR_MISPLACED_PREPROC_TOKEN:
            break;
    }
}

static bool expand_func_macro(struct preproc_state* state,
                              struct token_arr* res,
                              const struct preproc_macro* macro,
                              size_t macro_idx,
                              size_t macro_end);

static void expand_obj_macro(struct token_arr* res,
                             const struct preproc_macro* macro,
                             size_t macro_idx);

bool expand_preproc_macro(struct preproc_state* state,
                          struct token_arr* res,
                          const struct preproc_macro* macro,
                          size_t macro_idx,
                          size_t macro_end) {
    assert(state);
    assert(macro);

    if (macro->is_func_macro) {
        assert(macro_end < res->len);
        assert(res->tokens[macro_end].type == RBRACKET);
        return expand_func_macro(state, res, macro, macro_idx, macro_end);
    } else {
        expand_obj_macro(res, macro, macro_idx);
        return true;
    }
}

static struct token move_token(struct token* t);

struct preproc_macro parse_preproc_macro(struct token_arr* arr,
                                         const char* spell,
                                         struct preproc_err* err) {
    assert(arr);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    assert(arr->tokens[1].type == IDENTIFIER);
    assert(strcmp(str_get_data(&arr->tokens[1].spelling), "define") == 0);

    (void)err; // TODO: remove

    if (arr->len < 3) {
        // TODO: missing macro name
        return (struct preproc_macro){0};
    } else if (arr->tokens[2].type != IDENTIFIER) {
        // TODO: expected macro name
        return (struct preproc_macro){0};
    }

    struct preproc_macro res;
    res.spell = spell;

    if (arr->len > 3 && arr->tokens[3].type == LBRACKET) {
        res.is_func_macro = true;

        size_t it = 4;
        res.num_args = 0;

        const char** arg_spells = NULL;
        size_t arg_spells_cap = 0;
        while (it < arr->len && arr->tokens[it].type != RBRACKET
               && arr->tokens[it].type != ELLIPSIS) {
            if (arr->tokens[it].type != IDENTIFIER) {
                // TODO: unexpected token
                return (struct preproc_macro){0};
            }

            if (arg_spells_cap == res.num_args) {
                grow_alloc((void**)&arg_spells, &arg_spells_cap, sizeof(char*));
            }
            arg_spells[res.num_args] = str_get_data(&arr->tokens[it].spelling);
            assert(arg_spells[res.num_args]);

            ++res.num_args;
            ++it;

            if (arr->tokens[it].type != RBRACKET) {
                if (arr->tokens[it].type != COMMA) {
                    // TODO: expected comma or rbracket
                    return (struct preproc_macro){0};
                }
                ++it;
            }
        }

        if (it == arr->len) {
            // TODO: missing closing bracket
            return (struct preproc_macro){0};
        }

        if (arr->tokens[it].type == ELLIPSIS) {
            ++it;
            res.is_variadic = true;
            if (arr->tokens[it].type != RBRACKET) {
                // TODO: error
                return (struct preproc_macro){0};
            }
        } else {
            res.is_variadic = false;
        }

        assert(arr->tokens[it].type == RBRACKET);

        res.expansion_len = arr->len - it - 1; // TODO: not sure about - 1
        res.expansion = res.expansion_len == 0
                            ? NULL
                            : xmalloc(sizeof(struct token_or_arg)
                                      * res.expansion_len);

        for (size_t i = it + 1; i < arr->len; ++i) {
            const size_t res_idx = i - it - 1;

            struct token* curr_tok = &arr->tokens[i];
            struct token_or_arg* res_curr = &res.expansion[res_idx];
            if (curr_tok->type == IDENTIFIER) {
                if (res.is_variadic
                    && strcmp(str_get_data(&curr_tok->spelling), "__VA_ARGS__") == 0) {
                    res_curr->is_arg = true;
                    res_curr->arg_num = res.num_args;
                    continue;
                }

                size_t idx = (size_t)-1;
                for (size_t j = 0; j < res.num_args; ++j) {
                    if (strcmp(str_get_data(&curr_tok->spelling), arg_spells[j]) == 0) {
                        idx = j;
                        break;
                    }
                }

                if (idx != (size_t)-1) {
                    res_curr->is_arg = true;
                    res_curr->arg_num = idx;
                    continue;
                }
            }

            res_curr->is_arg = false;
            res_curr->token = *curr_tok;
            curr_tok->spelling = create_null_str();
        }

        // cast to make msvc happy (though it shouldn't be like this)
        free((void*)arg_spells);
    } else {
        res.is_func_macro = false;
        res.num_args = 0;
        res.is_variadic = false;

        res.expansion_len = arr->len - 3;
        res.expansion = res.expansion_len == 0
                            ? NULL
                            : xmalloc(sizeof(struct token_or_arg)
                                      * res.expansion_len);
        for (size_t i = 3; i < arr->len; ++i) {
            const size_t res_idx = i - 3;
            struct token_or_arg* res_curr = &res.expansion[res_idx];
            res_curr->is_arg = false;
            res_curr->token = move_token(&arr->tokens[i]);
        }
    }

    return res;
}

void free_preproc_macro(struct preproc_macro* m) {
    for (size_t i = 0; i < m->expansion_len; ++i) {
        if (!m->expansion[i].is_arg) {
            free_token(&m->expansion[i].token);
        }
    }
    free(m->expansion);
}

static struct token copy_token(const struct token* t);

static struct token move_token(struct token* t) {
    struct token res = *t;
    t->spelling = create_null_str();
    return res;
}

static struct token_arr collect_until(struct token* start,
                                      const struct token* end) {
    const size_t len = end - start;
    struct token_arr res = {
        .len = len,
        .tokens = len == 0 ? NULL : xmalloc(sizeof(struct token) * len),
    };

    for (size_t i = 0; i < len; ++i) {
        res.tokens[i] = move_token(&start[i]);
    }
    return res;
}

/**
 *
 * @param it Token pointer to the start of a macro argument
 * @param limit_ptr Pointer to this macros closing bracket
 *
 * @return The macro argument given at the start of this pointer
 */
static struct token_arr collect_macro_arg(struct token* it,
                                          const struct token* limit_ptr) {
    size_t num_open_brackets = 0;
    struct token* arg_start = it;
    while (it != limit_ptr && it->type != COMMA
           && (it->type != RBRACKET || num_open_brackets != 0)) {
        if (it->type == LBRACKET) {
            ++num_open_brackets;
        } else if (it->type == RBRACKET) {
            --num_open_brackets;
        }

        ++it;
    }

    return collect_until(arg_start, it);
}

struct macro_args {
    size_t len;
    struct token_arr* arrs;
};

static void free_macro_args(struct macro_args* args) {
    for (size_t i = 0; i < args->len; ++i) {
        free_token_arr(&args->arrs[i]);
    }
    free(args->arrs);
}

/**
 * @brief Collects arguments for the given macro, assuming the number of
 *        arguments was already checked in the course of finding the closing
 *        bracket
 *
 * @param args_start Pointer to start of macro arguments (opening bracket)
 * @param limit_ptr Pointer to closing bracket of this macro
 * @param expected_args Number of arguments this macro expects
 *
 * @return Pointer to #expected_args token_arrs representing the arguments of
 *         this macro call
 */
struct macro_args collect_macro_args(struct token* args_start,
                                     const struct token* limit_ptr,
                                     size_t expected_args,
                                     bool is_variadic,
                                     struct preproc_err* err) {
    assert(args_start->type == LBRACKET);

    size_t cap = is_variadic ? expected_args + 1 : expected_args;
    struct macro_args res = {
        .len = 0,
        .arrs = cap == 0 ? NULL : xmalloc(sizeof(struct token_arr) * cap),
    };

    struct token* it = args_start + 1;
    while (res.len < expected_args && it != limit_ptr) {
        res.arrs[res.len] = collect_macro_arg(it, limit_ptr);
        it += res.arrs[res.len].len;

        ++res.len;
        if (it->type == COMMA) {
            if ((it + 1 == limit_ptr && res.len < expected_args)
                || (is_variadic && res.len == expected_args)) {
                break;
            } else {
                free_token(it);
                ++it;
            }
        }
    }

    if (it->type == COMMA
        && ((it + 1 == limit_ptr && res.len < expected_args) || is_variadic)) {
        free_token(it);
        ++it;
        res.arrs[res.len] = collect_until(it, limit_ptr);
        it += res.arrs[res.len].len;
        assert(it == limit_ptr);
        ++res.len;
    } else if (is_variadic && res.len == expected_args) {
        assert(it == limit_ptr || it == args_start + 1);
        res.arrs[res.len] = collect_until(it, limit_ptr);
        it += res.arrs[res.len].len;
        ++res.len;
    }

    if (res.len < expected_args) {
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = true;
        goto fail;
    } else if (it != limit_ptr) {
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = false;
        goto fail;
    }

    return res;
fail:
    free_macro_args(&res);
    return (struct macro_args){
        .len = 0,
        .arrs = NULL,
    };
}

static size_t get_expansion_len(const struct preproc_macro* macro,
                                const struct macro_args* args) {
    size_t len = 0;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const struct token_or_arg* item = &macro->expansion[i];
        if (item->is_arg) {
            len += args->arrs[item->arg_num].len;
        } else {
            len += 1;
        }
    }

    return len;
}

static void shift_back(struct token* tokens,
                       size_t num,
                       size_t from,
                       size_t to) {
    memmove(tokens + from + num,
            tokens + from,
            (to - from) * sizeof(struct token));
}

static void shift_forward(struct token* tokens,
                          size_t num,
                          size_t from,
                          size_t to) {
    memmove(tokens + from,
            tokens + from + num,
            (to - from - num) * sizeof(struct token));
}

static void copy_into_tokens(struct token* tokens,
                             size_t* token_idx,
                             const struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        tokens[*token_idx] = copy_token(&arr->tokens[i]);
        ++*token_idx;
    }
}

// TODO: stringification and concatenation
static bool expand_func_macro(struct preproc_state* state,
                              struct token_arr* res,
                              const struct preproc_macro* macro,
                              size_t macro_idx,
                              size_t macro_end) {
    assert(macro->is_func_macro);
    assert(res->tokens[macro_idx + 1].type == LBRACKET);
    struct macro_args args = collect_macro_args(res->tokens + macro_idx + 1,
                                                &res->tokens[macro_end],
                                                macro->num_args,
                                                macro->is_variadic,
                                                state->err);
    if (args.len == 0 && macro->num_args != 0) {
        assert(state->err->type != PREPROC_ERR_NONE);
        return false;
    }

    assert((macro->is_variadic && args.len == macro->num_args + 1)
           || (!macro->is_variadic && args.len == macro->num_args));

    const size_t exp_len = get_expansion_len(macro, &args);
    const size_t macro_call_len = macro_end - macro_idx + 1;

    const bool alloc_grows = exp_len > macro_call_len;
    const size_t alloc_change = alloc_grows ? exp_len - macro_call_len
                                            : macro_call_len - exp_len;

    const size_t old_len = res->len;

    free_token(&res->tokens[macro_idx]);                      // identifier
    free_token(&res->tokens[macro_idx + 1]);                  // opening bracket
    free_token(&res->tokens[macro_idx + macro_call_len - 1]); // closing bracket

    if (alloc_grows) {
        res->len += alloc_change;
        res->cap += alloc_change;
        res->tokens = xrealloc(res->tokens, sizeof(struct token) * res->cap);

        shift_back(res->tokens,
                   alloc_change,
                   macro_idx + macro_call_len,
                   old_len);
    } else if (alloc_change != 0) {
        res->len -= alloc_change;

        shift_forward(res->tokens, alloc_change, macro_idx + exp_len, old_len);
    }

    size_t token_idx = macro_idx;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        if (curr->is_arg) {
            copy_into_tokens(res->tokens,
                             &token_idx,
                             &args.arrs[curr->arg_num]);
        } else {
            res->tokens[token_idx] = copy_token(&curr->token);
            ++token_idx;
        }
    }

    free_macro_args(&args);
    return true;
}

static void expand_obj_macro(struct token_arr* res,
                             const struct preproc_macro* macro,
                             size_t macro_idx) {
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const size_t exp_len = macro->expansion_len;
    const size_t old_len = res->len;

    free_token(&res->tokens[macro_idx]);

    if (exp_len > 0) {
        res->cap += exp_len - 1;
        res->len += exp_len - 1;
        res->tokens = xrealloc(res->tokens, sizeof(struct token) * res->cap);

        shift_back(res->tokens, exp_len - 1, macro_idx, old_len);
    } else {
        res->len -= 1;

        shift_forward(res->tokens, 1, macro_idx, old_len);
    }

    for (size_t i = 0; i < exp_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        assert(!curr->is_arg);

        res->tokens[macro_idx + i] = copy_token(&curr->token);
    }
}

static struct token copy_token(const struct token* t) {
    assert(t);
    return (struct token){
        .type = t->type,
        .spelling = str_copy(&t->spelling),
        // TODO: identify as token from macro expansion
        .loc =
            {
                .file_idx = t->loc.file_idx,
                .file_loc = t->loc.file_loc,
            },
    };
}

struct preproc_state create_preproc_state(const char* start_file, struct preproc_err* err) {
    struct str file_name = create_str(strlen(start_file), start_file);
    return (struct preproc_state){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = create_string_hash_map(
            sizeof(struct preproc_macro),
            100,
            true,
            (void (*)(void*))free_preproc_macro),
        .file_info = create_file_info(&file_name),
    };
}

const struct preproc_macro* find_preproc_macro(const struct preproc_state* state,
                                               const struct str* spelling) {
    return string_hash_map_get(&state->_macro_map, spelling);
}

void register_preproc_macro(struct preproc_state* state,
                            const struct str* spelling,
                            const struct preproc_macro* macro) {
    bool overwritten = string_hash_map_insert_overwrite(&state->_macro_map, spelling, macro);
    (void)overwritten; // TODO: warning if redefined
}

void remove_preproc_macro(struct preproc_state* state, const struct str* spelling) {
    string_hash_map_remove(&state->_macro_map, spelling);
}

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true) {
    if (state->conds_len == state->conds_cap) {
        grow_alloc((void**)&state->conds,
                   &state->conds_cap,
                   sizeof(struct preproc_cond));
    }

    struct preproc_cond c = {
        .had_true_branch = was_true,
        .had_else = false,
        .loc = loc,
    };
    state->conds[state->conds_len] = c;
    ++state->conds_len;
}

void pop_preproc_cond(struct preproc_state* state) {
    --state->conds_len;
}

struct preproc_cond* peek_preproc_cond(struct preproc_state* state) {
    return &state->conds[state->conds_len - 1];
}

void free_token_arr(struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        free_str(&arr->tokens[i].spelling);
    }
    free(arr->tokens);
}

void free_preproc_state(struct preproc_state* state) {
    free_token_arr(&state->res);
    free(state->conds);
    free_string_hash_map(&state->_macro_map);
    free_file_info(&state->file_info);
}

static bool is_dec_const(const char* str, size_t len);
static bool is_hex_const(const char* str, size_t len);
static bool is_oct_const(const char* str, size_t len);

static bool is_oct_or_hex_const_start(const char* str, size_t len);

bool is_int_const(const char* str, size_t len) {
    if (is_oct_or_hex_const_start(str, len)) {
        if (len >= 2 && tolower((unsigned char)str[1]) == 'x') {
            return is_hex_const(str, len);
        } else {
            return is_oct_const(str, len);
        }
    } else {
        return is_dec_const(str, len);
    }
}

static bool is_hex_digit(char c) {
    unsigned char uc = (unsigned char)c;
    return isdigit(c) || (tolower(uc) >= 'a' && tolower(uc) <= 'f');
}

static bool is_int_suffix(const char* str, size_t len) {
    for (size_t i = 0; i != len; ++i) {
        unsigned char uc = (unsigned char)str[i];
        if (tolower(uc) != 'u' && tolower(uc) != 'l') {
            return false;
        }
    }
    return true;
}

static bool is_hex_const(const char* str, size_t len) {
    assert(len > 2 && str[0] == '0' && tolower((unsigned char)str[1]) == 'x');

    size_t i = 2;
    if (!is_hex_digit(str[i])) {
        return false;
    }
    ++i;

    while (i != len && is_hex_digit(str[i])) {
        ++i;
    }

    if (i == len || is_int_suffix(str + i, len - i)) {
        return true;
    } else {
        return false;
    }
}

static bool is_oct_or_hex_const_start(const char* str, size_t len) {
    return len >= 2 && str[0] == '0';
}

static bool is_oct_const(const char* str, size_t len) {
    assert(is_oct_or_hex_const_start(str, len)
           && tolower((unsigned char)str[1]) != 'x');
    size_t i = 1;

    while (i != len && isdigit(str[i])) {
        ++i;
    }

    if (i == len || is_int_suffix(str + i, len - i)) {
        return true;
    } else {
        return false;
    }
}

static bool is_dec_const(const char* str, size_t len) {
    assert(len > 0 && !is_oct_or_hex_const_start(str, len));
    if (!isdigit(str[0])) {
        return false;
    }
    size_t i = 1;

    while (i != len && isdigit(str[i])) {
        ++i;
    }

    if (i == len || is_int_suffix(str + i, len - i)) {
        return true;
    } else {
        return false;
    }
}

bool is_char_const(const char* str, size_t len) {
    size_t last = len - 1;
    size_t i = 0;
    if (str[i] == 'L') {
        ++i;
    }

    if (str[i] != '\'' || str[last] != '\'') {
        return false;
    }

    ++i;

    char prev = str[i - 1];
    for (; i != last; ++i) {
        if (str[i] == '\'' && prev != '\\') {
            return false;
        }

        prev = str[i];
    }

    return true;
}

static bool is_dec_float_const(const char* str, size_t len);
static bool is_hex_float_const(const char* str, size_t len);

bool is_float_const(const char* str, size_t len) {
    assert(len > 0);
    if (str[0] == '0' && len >= 2 && tolower(str[1]) == 'x') {
        return is_hex_float_const(str, len);
    } else {
        return is_dec_float_const(str, len);
    }
}

static bool is_float_suffix(char c) {
    unsigned char uc = (unsigned char)c;
    return tolower(uc) == 'l' || tolower(uc) == 'f';
}

static bool is_exp_suffix(const char* str, size_t len, bool is_hex) {
    const char exp_char = is_hex ? 'p' : 'e';
    size_t i = 0;
    if (len < 2 || tolower((unsigned char)str[0]) != exp_char) {
        return false;
    }
    ++i;

    if ((str[i] != '+' && str[i] != '-' && !isdigit(str[i]))
        || (!isdigit(str[i]) && i + 1 == len)) {
        return false;
    }

    ++i;

    while (i != len && tolower(str[i]) != 'f' && tolower(str[i]) != 'l') {
        if (!isdigit(str[i])) {
            return false;
        }
        ++i;
    }

    if (i != len) {
        if (i != len - 1) {
            return false;
        }
        return is_float_suffix(str[i]);
    } else {
        return true;
    }
}

static bool is_dec_float_const(const char* str, size_t len) {

    size_t i = 0;
    while (i != len && isdigit(str[i])) {
        ++i;
    }

    if (i == len) {
        return false;
    } else if (str[i] == '.') {
        ++i;
        while (i != len && isdigit(str[i])) {
            ++i;
        }
        if (i == len || (i == len - 1 && is_float_suffix(str[len - 1]))
            || is_exp_suffix(str + i, len - i, false)) {
            return true;
        } else {
            return false;
        }
    } else if (str[i] == 'e') {
        return is_exp_suffix(str + i, len - i, false);
    } else {
        return false;
    }
}

static bool is_hex_float_const(const char* str, size_t len) {
    assert(len >= 2);
    assert(str[0] == '0' && tolower(str[1]) == 'x');

    size_t i = 2;
    while (i < len && tolower(str[i]) != 'p') {
        if (!is_hex_digit(str[i]) && str[i] != '.') {
            return false;
        }
        ++i;
    }

    if (i != len) {
        return is_exp_suffix(str + i, len - i, true);
    } else {
        return true;
    }
}

bool is_string_literal(const char* str, size_t len) {
    size_t last = len - 1;
    size_t i = 0;
    if (str[i] == 'L') {
        ++i;
    }

    if (str[i] != '\"' || str[last] != '\"') {
        return false;
    }
    ++i;
    char prev = str[i - 1];
    for (; i != last; ++i) {
        if ((str[i] == '\"' || str[i] == '\n') && prev != '\\') {
            return false;
        }
        prev = str[i];
    }

    return true;
}

static bool is_id_char(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

bool is_valid_identifier(const char* str, size_t len) {
    if (!isalpha(str[0]) && str[0] != '_') {
        return false;
    }
    for (size_t i = 1; i != len; ++i) {
        if (!is_id_char(str[i])) {
            return false;
        }
    }

    return true;
}

struct tokenizer_state {
    const char* it;
    char prev;
    char prev_prev;
    struct file_loc file_loc;
    size_t current_file_idx;
};

static enum token_type singlec_token_type(char c);
static enum token_type check_next(enum token_type type, const char* next);
static bool is_singlec_token(const struct tokenizer_state* s,
                             enum token_type t);

static void advance(struct tokenizer_state* s, size_t num);
static void advance_one(struct tokenizer_state* s);
static void advance_newline(struct tokenizer_state* s);

static void add_token(struct token_arr* res,
                      enum token_type type,
                      struct str* spell,
                      struct file_loc file_loc,
                      size_t file_idx);

static void handle_comments(struct tokenizer_state* s,
                            bool* comment_not_terminated);
static bool handle_character_literal(struct tokenizer_state* s,
                                     struct token_arr* res,
                                     struct preproc_err* err);
static bool handle_other(struct tokenizer_state* s,
                         struct token_arr* res,
                         struct preproc_err* err);
static void handle_ongoing_comment(struct tokenizer_state* s,
                                   bool* comment_not_terminated);

bool tokenize_line(struct token_arr* res,
                   struct preproc_err* err,
                   const char* line,
                   size_t line_num,
                   size_t file_idx,
                   bool* comment_not_terminated) {
    assert(res);
    assert(line);
    assert(file_idx != (size_t)-1);
    assert(comment_not_terminated);

    struct tokenizer_state s = {
        .it = line,
        .prev = '\0',
        .prev_prev = '\0',
        .file_loc =
            {
                .line = line_num,
                .index = 1,
            },
        .current_file_idx = file_idx,
    };

    while (*s.it != '\0') {
        if (*comment_not_terminated) {
            handle_ongoing_comment(&s, comment_not_terminated);
            continue;
        }
        while (isspace(*s.it)) {
            // TODO: how to handle escaped newlines
            advance_newline(&s);
        }
        if (*s.it == '\0') {
            break;
        }

        enum token_type type = singlec_token_type(*s.it);
        if (type != INVALID && is_singlec_token(&s, type)) {
            if (type == DIV && (s.it[1] == '/' || s.it[1] == '*')) {
                handle_comments(&s, comment_not_terminated);
                continue;
            }
            if (s.it[1] != '\0') {
                type = check_next(type, s.it + 1);
            }
            
            struct str null_str = create_null_str();
            add_token(res, type, &null_str, s.file_loc, s.current_file_idx);

            size_t len = strlen(get_spelling(type));
            advance(&s, len);
        } else if (*s.it == '\"' || *s.it == '\''
                   || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) {
            if (!handle_character_literal(&s, res, err)) {
                return false;
            }
        } else {
            if (!handle_other(&s, res, err)) {
                return false;
            }
        }
    }

    return true;
}

static enum token_type singlec_token_type(char c) {
    switch (c) {
        case ';':
            return SEMICOLON;
        case '(':
            return LBRACKET;
        case ')':
            return RBRACKET;
        case '{':
            return LBRACE;
        case '}':
            return RBRACE;
        case '[':
            return LINDEX;
        case ']':
            return RINDEX;
        case '.':
            return DOT;
        case '&':
            return AND;
        case '|':
            return OR;
        case '^':
            return XOR;
        case '!':
            return NOT;
        case '~':
            return BNOT;
        case '-':
            return SUB;
        case '+':
            return ADD;
        case '*':
            return ASTERISK;
        case '/':
            return DIV;
        case '%':
            return MOD;
        case '<':
            return LT;
        case '>':
            return GT;
        case '?':
            return QMARK;
        case ':':
            return COLON;
        case '=':
            return ASSIGN;
        case ',':
            return COMMA;
        case '#':
            return STRINGIFY_OP;
        default:
            return INVALID;
    }
}

static bool check_type(enum token_type type, const char* next_chars) {
    const char* spelling = get_spelling(type);
    size_t len = strlen(spelling);
    assert(len != 0);
    assert(len >= 2);
    return strncmp(spelling + 1, next_chars, len - 1) == 0;
}

static enum token_type check_next(enum token_type type, const char* next) {
    assert(next[0] != '\0');
    switch (type) {
        case ADD:
            if (check_type(ADD_ASSIGN, next)) {
                return ADD_ASSIGN;
            } else if (check_type(INC_OP, next)) {
                return INC_OP;
            } else {
                break;
            }
        case SUB:
            if (check_type(PTR_OP, next)) {
                return PTR_OP;
            } else if (check_type(DEC_OP, next)) {
                return DEC_OP;
            } else if (check_type(SUB_ASSIGN, next)) {
                return SUB_ASSIGN;
            } else {
                break;
            }
        case ASTERISK:
            if (check_type(MUL_ASSIGN, next)) {
                return MUL_ASSIGN;
            } else {
                break;
            }
        case DIV:
            if (check_type(DIV_ASSIGN, next)) {
                return DIV_ASSIGN;
            } else {
                break;
            }
        case LT:
            if (check_type(LEFT_ASSIGN, next)) {
                return LEFT_ASSIGN;
            } else if (check_type(LEFT_OP, next)) {
                return LEFT_OP;
            } else if (check_type(LE_OP, next)) {
                return LE_OP;
            } else {
                break;
            }
        case GT:
            if (check_type(RIGHT_ASSIGN, next)) {
                return RIGHT_ASSIGN;
            } else if (check_type(RIGHT_OP, next)) {
                return RIGHT_OP;
            } else if (check_type(GE_OP, next)) {
                return GE_OP;
            } else {
                break;
            }
        case AND:
            if (check_type(AND_OP, next)) {
                return AND_OP;
            } else if (check_type(AND_ASSIGN, next)) {
                return AND_ASSIGN;
            } else {
                break;
            }
        case OR:
            if (check_type(OR_OP, next)) {
                return OR_OP;
            } else if (check_type(OR_ASSIGN, next)) {
                return OR_ASSIGN;
            } else {
                break;
            }
        case XOR:
            if (check_type(XOR_ASSIGN, next)) {
                return XOR_ASSIGN;
            } else {
                break;
            }
        case MOD:
            if (check_type(MOD_ASSIGN, next)) {
                return MOD_ASSIGN;
            } else {
                break;
            }
        case DOT:
            if (check_type(ELLIPSIS, next)) {
                return ELLIPSIS;
            } else {
                break;
            }
        case ASSIGN:
            if (check_type(EQ_OP, next)) {
                return EQ_OP;
            } else {
                break;
            }
        case NOT:
            if (check_type(NE_OP, next)) {
                return NE_OP;
            } else {
                break;
            }
        case STRINGIFY_OP:
            if (check_type(CONCAT_OP, next)) {
                return CONCAT_OP;
            } else {
                break;
            }
        default:
            break;
    }

    return type;
}

static bool is_singlec_token(const struct tokenizer_state* s,
                             enum token_type t) {
    switch (t) {
        case DOT:
            return !isdigit(s->it[1]);
        default:
            return true;
    }
}

static void advance(struct tokenizer_state* s, size_t num) {
    assert(num > 0);
    s->it += num;
    s->file_loc.index += num;

    if (num > 1 || s->prev != '\0') {
        s->prev = *(s->it - 1);
        s->prev_prev = *(s->it - 2);
    }
}

static void advance_one(struct tokenizer_state* s) {
    ++s->file_loc.index;
    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static void advance_newline(struct tokenizer_state* s) {
    if (*s->it == '\n') {
        assert(s->it[-1] == '\\');
        s->file_loc.line += 1;
        s->file_loc.index = 1;
    } else {
        ++s->file_loc.index;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static void realloc_tokens_if_needed(struct token_arr* res) {
    if (res->len == res->cap) {
        grow_alloc((void**)&res->tokens, &res->cap, sizeof(struct token));
    }
}

static void add_token(struct token_arr* res,
                      enum token_type type,
                      struct str* spell,
                      struct file_loc file_loc,
                      size_t file_idx) {
    realloc_tokens_if_needed(res);
    res->tokens[res->len] = create_token(type, spell, file_loc, file_idx);
    ++res->len;
}

static void handle_comments(struct tokenizer_state* s,
                            bool* comment_not_terminated) {
    assert(*s->it == '/');
    assert(s->it[1] == '*' || s->it[1] == '/');

    assert(s->it[1] == '*' || s->it[1] == '/');
    if (s->it[1] == '*') {
        advance(s, 2);

        handle_ongoing_comment(s, comment_not_terminated);
    } else {
        while (*s->it != '\0' && *s->it != '\n') {
            advance_one(s);
        }
    }
}

static void handle_ongoing_comment(struct tokenizer_state* s,
                                   bool* comment_not_terminated) {
    assert(comment_not_terminated);

    while (*s->it != '\0' && (*s->it != '*' || s->it[1] != '/')) {
        advance_newline(s);
    }

    if (*s->it == '\0') {
        *comment_not_terminated = true;
    } else {
        assert(*s->it == '*');
        assert(s->it[1] == '/');
        advance(s, 2);
        *comment_not_terminated = false;
    }
}

static enum token_type get_char_lit_type(const char* buf,
                                         size_t len,
                                         char terminator) {
    if (terminator == '\"' && is_string_literal(buf, len)) {
        return STRING_LITERAL;
    } else if (terminator == '\'' && is_char_const(buf, len)) {
        return I_CONSTANT;
    } else {
        return INVALID;
    }
}

static void unterminated_literal_err(struct preproc_err* err,
                                     char terminator,
                                     struct file_loc start_loc,
                                     size_t file_idx) {
    const bool is_char_lit = terminator == '\'';

    struct source_loc loc = {
        .file_idx = file_idx,
        .file_loc = start_loc,
    };
    set_preproc_err(err, PREPROC_ERR_UNTERMINATED_LIT, loc);

    err->is_char_lit = is_char_lit;
}

static bool handle_character_literal(struct tokenizer_state* s,
                                     struct token_arr* res,
                                     struct preproc_err* err) {
    assert(*s->it == '\'' || *s->it == '\"' || *s->it == 'L');
    enum {
        BUF_STRLEN = 512
    };
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    const struct file_loc start_loc = s->file_loc;

    char terminator;
    if (*s->it == 'L') {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
        assert(*s->it == '\"' || *s->it == '\'');
    }

    terminator = *s->it;
    spell_buf[buf_idx] = *s->it;
    ++buf_idx;

    advance_one(s);
    while (
        *s->it != '\0'
        && ((s->prev_prev != '\\' && s->prev == '\\') || *s->it != terminator)
        && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_newline(s);
    }

    struct str spell = create_str(buf_idx, spell_buf);
    if (buf_idx == BUF_STRLEN) {
        while (*s->it != '\0'
               && ((s->prev_prev != '\\' && s->prev == '\\')
                   || *s->it != terminator)) {
            
            str_push_back(&spell, *s->it);

            advance_newline(s);
        }
        
        str_shrink_to_fit(&spell);
    }

    if (*s->it == '\0') {
        free_str(&spell);
        unterminated_literal_err(err,
                                 terminator,
                                 start_loc,
                                 s->current_file_idx);
        return false;
    } else {
        str_push_back(&spell, *s->it);

        advance_one(s);

        enum token_type type = get_char_lit_type(str_get_data(&spell),
                                                 str_len(&spell),
                                                 terminator);
        assert(type != INVALID);

        add_token(res, type, &spell, start_loc, s->current_file_idx);
    }

    return true;
}

static bool token_is_over(const struct tokenizer_state* s, bool is_num) {
    if (*s->it == '\0' || isspace(*s->it)) {
        return true;
    }
    enum token_type type = singlec_token_type(*s->it);
    if (type == INVALID) {
        return false;
    } else if (is_num) {
        switch (type) {
            case SUB:
            case ADD: {
                char prev = (char)tolower((unsigned char)s->prev);
                return (prev != 'e' && prev != 'p');
            }
            case DOT:
                return false;
            default:
                return true;
        }
    } else {
        return true;
    }
}

static bool handle_other(struct tokenizer_state* s,
                         struct token_arr* res,
                         struct preproc_err* err) {
    enum {
        BUF_STRLEN = 512
    };

    const bool is_num = isdigit(*s->it) || *s->it == '.';
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    struct file_loc start_loc = s->file_loc;
    while (!token_is_over(s, is_num) && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
    }

    struct str spell = create_str(buf_idx, spell_buf);
    if (!token_is_over(s, is_num)) {

        while (!token_is_over(s, is_num)) {
            
            str_push_back(&spell, *s->it);

            advance_one(s);
        }
        
        str_shrink_to_fit(&spell);
    }

    enum token_type type;
    const char* spell_data = str_get_data(&spell);
    const size_t spell_len = str_len(&spell);
    if (is_num) {
        if (is_int_const(spell_data, spell_len)) {
            type = I_CONSTANT;
        } else if (is_float_const(spell_data, spell_len)) {
            type = F_CONSTANT;
        } else {
            struct source_loc loc = {
                .file_idx = s->current_file_idx,
                .file_loc = start_loc,
            };
            set_preproc_err(err, PREPROC_ERR_INVALID_NUMBER, loc);
            err->invalid_num = spell;
            return false;
        }
    } else if (is_valid_identifier(spell_data, spell_len)) {
        type = IDENTIFIER;
    } else {
        struct source_loc loc = {
            .file_idx = s->current_file_idx,
            .file_loc = start_loc,
        };
        set_preproc_err(err, PREPROC_ERR_INVALID_ID, loc);

        err->invalid_id = spell;
        return false;
    }

    add_token(res, type, &spell, start_loc, s->current_file_idx);

    return true;
}

void file_read_line(FILE* file,
                    char** res,
                    size_t* res_len,
                    char* static_buf,
                    size_t static_buf_len) {
    assert(res);
    assert(res_len);

    int c;
    size_t cap;
    if (*res_len < static_buf_len) {
        *res = static_buf;
        bool copy_to_dyn_buf = false;
        while ((c = getc(file)) != '\n' && c != EOF) {
            static_buf[*res_len] = (char)c;
            ++*res_len;
            if (*res_len == static_buf_len - 1) {
                copy_to_dyn_buf = true;
                break;
            }
        }

        if (copy_to_dyn_buf) {
            cap = static_buf_len * 2;
            *res = xmalloc(sizeof(char) * cap);
            memcpy(*res, static_buf, sizeof(char) * (static_buf_len - 1));
        } else if (*res_len == 0 && c == EOF) {
            *res = NULL;
            return;
        } else {
            static_buf[*res_len] = '\0';
            *res = static_buf;
            return;
        }
    } else {
        cap = *res_len;
    }

    while ((c = getc(file)) != '\n' && c != EOF) {
        if (*res_len == cap) {
            grow_alloc((void**)res, &cap, sizeof(char));
        }
        (*res)[*res_len] = (char)c;

        ++*res_len;
    }

    if (*res_len == 0 && c == EOF) {
        assert(*res == NULL);
        return;
    }

    *res = xrealloc(*res, sizeof(char) * (*res_len + 1));
    (*res)[*res_len] = '\0';
}

void* xmalloc(size_t bytes) {
    assert(bytes != 0);

    void* res = malloc(bytes);
    if (!res) {
        fprintf(stderr, "xmalloc():\n\tFailed to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* xcalloc(size_t len, size_t elem_size) {
    assert(len != 0);
    assert(elem_size != 0);

    void* res = calloc(len, elem_size);
    if (!res) {
        fprintf(stderr,
                "xcalloc():\n\tFailed to allocate %zu elements of size %zu bytes each\n",
                len,
                elem_size);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* xrealloc(void* alloc, size_t bytes) {
    if (bytes == 0) {
        free(alloc);
        return NULL;
    }

    void* res = realloc(alloc, bytes);
    if (!res) {
        fprintf(stderr, "xrealloc():\n\tFailed to realloc %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size) {
    size_t new_num = *alloc_len + *alloc_len / 2 + 1;
    *alloc = xrealloc(*alloc, elem_size * new_num);
    *alloc_len = new_num;
}

char* alloc_string_copy(const char* str) {
    assert(str);
    char* res = xmalloc(sizeof(char) * (strlen(str) + 1));
    strcpy(res, str);
    return res;
}

struct str create_null_str(void) {
    return (struct str){
        ._is_static_buf = false,
        ._len = 0,
        ._cap = 0,
        ._data = NULL,
    };
}

struct str create_empty_str(void) {
    struct str res = {
        ._is_static_buf_dup = true,
        ._small_len = 0,
        ._static_buf = {0},
    };
    return res;
}

enum {
    STATIC_BUF_LEN = sizeof(struct str){0}._static_buf
                     / sizeof *(struct str){0}._static_buf
};

static struct str create_str_with_cap(size_t len, const char* str, size_t cap) {
    assert(cap >= len);
    assert(len == 0 || str);
    struct str res;
    if (cap < STATIC_BUF_LEN) {
        res._is_static_buf = true;
        res._small_len = (uint8_t)len;
        memcpy(res._static_buf, str, sizeof *res._static_buf * len);
        res._static_buf[len] = '\0';
    } else {
        res._is_static_buf = false;
        res._len = len;
        res._cap = cap + 1;
        res._data = xmalloc(sizeof *res._data * res._cap);
        memcpy(res._data, str, sizeof *res._static_buf * len);
        res._data[len] = '\0';
    }
    return res;
}

struct str create_str(size_t len, const char* str) {
    assert(len == 0 || str);
    return create_str_with_cap(len, str, len);
}

bool str_is_valid(const struct str* str) {
    assert(str);
    return str->_is_static_buf || str->_data != NULL;
}

size_t str_len(const struct str* str) {
    assert(str_is_valid(str));
    if (str->_is_static_buf) {
        return str->_small_len;
    } else {
        return str->_len;
    }
}

static char* str_get_mut_data(struct str* str) {
    assert(str);
    assert(str_is_valid(str));
    if (str->_is_static_buf) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

const char* str_get_data(const struct str* str) {
    assert(str);
    if (str->_is_static_buf) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

char str_char_at(const struct str* str, size_t i) {
    assert(str);
    assert(str_is_valid(str));
    if (str->_is_static_buf) {
        assert(i < str->_small_len);
        return str->_static_buf[i];
    } else {
        assert(i < str->_len);
        return str->_data[i];
    }
}

void str_push_back(struct str* str, char c) {
    assert(str);
    assert(str_is_valid(str));
    if (str->_is_static_buf) {
        if (str->_small_len == STATIC_BUF_LEN - 1) {
            size_t len = str->_small_len;
            char* data = xmalloc(sizeof *data * (STATIC_BUF_LEN + 1));
            memcpy(data, str->_static_buf, sizeof *data * (STATIC_BUF_LEN - 1));
            data[len] = c;
            ++len;
            data[len] = '\0';
            str->_cap = len + 1;
            str->_len = len;
            str->_data = data;
            str->_is_static_buf = false;
        } else {
            str->_static_buf[str->_small_len] = c;
            ++str->_small_len;
            str->_static_buf[str->_small_len] = '\0';
        }
    } else {
        if (str->_len == str->_cap - 1) {
            grow_alloc((void**)&str->_data, &str->_cap, sizeof *str->_data);
        }
        str->_data[str->_len] = c;
        ++str->_len;
        str->_data[str->_len] = '\0';
    }
}

void str_shrink_to_fit(struct str* str) {
    assert(str);
    assert(str_is_valid(str));
    if (!str->_is_static_buf && str->_len + 1 != str->_cap) {
        str->_cap = str->_len + 1;
        str->_data = xrealloc(str->_data, sizeof *str->_data * str->_cap);
    }
}

struct str str_concat(size_t len1,
                      const char* s1,
                      size_t len2,
                      const char* s2) {
    const size_t len = len1 + len2;
    struct str res = create_str_with_cap(len1, s1, len);
    char* res_data = str_get_mut_data(&res);
    memcpy(res_data + len1, s2, len2 * sizeof *res_data);
    res_data[len] = '\0';
    if (res._is_static_buf) {
        res._small_len = (uint8_t)len;
    } else {
        res._len = len;
    }
    return res;
}

struct str str_take(struct str* str) {
    assert(str);
    assert(str_is_valid(str));
    struct str res = *str;
    str->_is_static_buf = false;
    str->_len = 0;
    str->_cap = 0;
    str->_data = NULL;
    return res;
}

struct str str_copy(const struct str* str) {
    assert(str);
    if (str->_is_static_buf) {
        return *str;
    } else {
        if (str_is_valid(str)) {
            return create_str_with_cap(str->_len, str->_data, str->_cap - 1);
        } else {
            return create_null_str();
        }
    }
}

void free_str(const struct str* str) {
    assert(str);
    if (!str->_is_static_buf) {
        free(str->_data);
    }
}

struct string_hash_map_key {
    bool was_deleted;
    struct str str;
};

struct string_hash_map create_string_hash_map(size_t elem_size,
                                              size_t init_cap,
                                              bool free_keys,
                                              void (*item_free)(void*)) {
    return (struct string_hash_map){
        ._len = 0,
        ._cap = init_cap,
        ._item_size = elem_size,
        ._free_keys = free_keys,
        ._item_free = item_free,
        ._keys = xcalloc(init_cap, sizeof(struct string_hash_map_key)),
        ._items = xcalloc(init_cap, elem_size),
    };
}

void free_string_hash_map(struct string_hash_map* map) {
    if (map->_item_free) {
        char* items_char = map->_items;
        for (size_t i = 0; i < map->_cap; ++i) {
            if (str_is_valid(&map->_keys[i].str)) {
                void* item = items_char + i * map->_item_size;
                map->_item_free(item);
            }
        }
    }

    free(map->_items);

    if (map->_free_keys) {
        for (size_t i = 0; i < map->_cap; ++i) {
            free_str(&map->_keys[i].str);
        }
    }
    free(map->_keys);
}

static size_t hash_string(const struct str* str);
static void resize_map(struct string_hash_map* map);

static size_t find_item_index_insert(const struct string_hash_map* map,
                                     const struct str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    bool found_deleted = false;
    size_t deleted_idx = (size_t)-1;
    size_t it_count = 0;
    while (
        it_count != map->_cap
        && (map->_keys[i].was_deleted
            || (str_is_valid(&map->_keys[i].str)
                && strcmp(str_get_data(&map->_keys[i].str), str_get_data(key))
                       != 0))) {
        if (map->_keys[i].was_deleted && !found_deleted) {
            deleted_idx = i;
            found_deleted = true;
        }
        i = (i + 1) % map->_cap;
        ++it_count;
    }
    
    if (!str_is_valid(&map->_keys[i].str) && found_deleted) {
        return deleted_idx;
    } else {
        return i;
    }
}

static size_t find_item_index(const struct string_hash_map* map,
                              const struct str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    while (map->_keys[i].was_deleted
           || (str_is_valid(&map->_keys[i].str)
               && strcmp(str_get_data(&map->_keys[i].str), str_get_data(key))
                      != 0)) {
        i = (i + 1) % map->_cap;
    }

    return i;
}

static void rehash_if_necessary(struct string_hash_map* map) {
    if (map->_len == map->_cap) {
        resize_map(map);
    }
}

const void* string_hash_map_insert(struct string_hash_map* map,
                                   const struct str* key,
                                   const void* item) {
    assert(key);
    assert(item);
    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    void* found = (char*)map->_items + idx * map->_item_size;
    if (str_is_valid(&map->_keys[idx].str)) {
        return found;
    }

    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = false,
        .str = *key,
    };
    memcpy(found, item, map->_item_size);
    ++map->_len;

    return item;
}

bool string_hash_map_insert_overwrite(struct string_hash_map* map,
                                      const struct str* key,
                                      const void* item) {
    assert(key);
    assert(item);

    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    bool overwritten;
    void* curr_item = (char*)map->_items + idx * map->_item_size;
    if (str_is_valid(&map->_keys[idx].str)) {
        if (map->_free_keys) {
            free_str(key);
        }

        if (map->_item_free) {
            map->_item_free(curr_item);
        }
        overwritten = true;
    } else {
        map->_keys[idx] = (struct string_hash_map_key){
            .was_deleted = false,
            .str = *key,
        };
        overwritten = false;
    }

    memcpy(curr_item, item, map->_item_size);
    ++map->_len;
    return overwritten;
}

const void* string_hash_map_get(const struct string_hash_map* map,
                                const struct str* key) {
    assert(key);
    const size_t idx = find_item_index(map, key);

    if (!str_is_valid(&map->_keys[idx].str)) {
        return NULL;
    }

    return (char*)map->_items + idx * map->_item_size;
}

void string_hash_map_remove(struct string_hash_map* map,
                            const struct str* key) {
    const size_t idx = find_item_index(map, key);

    struct str* key_to_remove = &map->_keys[idx].str;
    if (key_to_remove == NULL) {
        return;
    }
    if (map->_free_keys) {
        free_str(key_to_remove);
    }
    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = true,
        .str = create_null_str(),
    };

    void* item_to_remove = (char*)map->_items + idx * map->_item_size;
    if (map->_item_free) {
        map->_item_free(item_to_remove);
    }
    --map->_len;
}

static void resize_map(struct string_hash_map* map) {
    const size_t prev_cap = map->_cap;
    const size_t prev_len = map->_len;
    struct string_hash_map_key* old_keys = map->_keys;
    void* old_items = map->_items;

    map->_len = 0;
    map->_cap += map->_cap / 2 + 1;
    map->_keys = xcalloc(map->_cap, sizeof(struct string_hash_map_key));
    map->_items = xcalloc(map->_cap, map->_item_size);

    for (size_t i = 0; i < prev_cap; ++i) {
        if (str_is_valid(&old_keys[i].str)) {
            const void* success = string_hash_map_insert(
                map,
                &old_keys[i].str,
                (char*)old_items + i * map->_item_size);
            assert(success != NULL);
        }
    }

    assert(map->_len == prev_len);
    free(old_items);
    free((void*)old_keys);
}

// Hash function taken from K&R version 2 (page 144)
static size_t hash_string(const struct str* str) {
    size_t hash = 0;

    const char* it = str_get_data(str);
    const char* limit = it + str_len(str);
    while (it != limit) {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}

