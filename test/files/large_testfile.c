typedef _Bool bool;
typedef unsigned long long size_t;

const bool true = 1;
const bool false = 0;

void* NULL = 0;

void free(void*);

int isdigit(char);
char tolower(unsigned char);

int isspace(int);
size_t strlen(const char*);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);
void strcpy(char*, const char*);

void assert(bool);

bool is_hex_const(const char* str, size_t num);
bool is_oct_const(const char* str, size_t num);
bool is_dec_const(const char* str, size_t num);
bool is_char_const(const char* str, size_t num);
bool is_float_const(const char* str, size_t num);
bool is_string_literal(const char* str, size_t num);
bool is_valid_identifier(const char* str, size_t num);

enum token_type {
    IDENTIFIER,
    I_CONSTANT,
    F_CONSTANT,
    STRING_LITERAL,
    FUNC_NAME, // __func__

    SIZEOF,
    PTR_OP, // ->
    INC_OP,
    DEC_OP,
    LEFT_OP, // <<
    RIGHT_OP, // >>
    LE_OP,
    GE_OP,
    EQ_OP,
    NE_OP,
    AND_OP, // &&
    OR_OP, // ||
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
    TYPEDEF_NAME,
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

    COMPLEX, // _Complex
    IMAGINARY, // _Imaginary

    CONST,
    VOLATILE,
    RESTRICT,
    ATOMIC,

    STRUCT,
    UNION,
    ENUM,
    ELLIPSIS, // ...
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

    ALIGNAS, // _Alignas
    ALIGNOF, // _Alignof
    GENERIC, // _Generic
    NORETURN, // _Noreturn
    STATIC_ASSERT, // _Static_assert
    THREAD_LOCAL, // _Thread_local

    SEMICOLON,
    LBRACKET, // (
    RBRACKET, // )
    LBRACE, // {
    RBRACE, // }
    LINDEX, // [
    RINDEX, // ]
    DOT,
    AND, // &
    OR, // |
    XOR, // ^
    NOT, // !
    BNOT, // ~
    SUB,
    ADD,
    ASTERISK,
    DIV,
    MOD,
    LT, // <
    GT, // >
    QMARK, // ?
    COLON, // :
    ASSIGN, // =
    COMMA,
    INVALID
};

/**
 * @brief Gets a spelling for the given token_type
 *
 * @param type Type to get the spelling for
 * @return const char* The spelling of the given token type, if it is unambiguous, otherwise NULL
 */
const char* get_spelling(enum token_type type);

/**
 * @brief Gets a string to identify the token_type
 *
 * @param type enum token_type value
 * @return const char* A string that is identical to the spelling of the enum value
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

struct source_location {
    size_t line, index;
};

struct token {
    enum token_type type;
    char* spelling;
    char* file;
    struct source_location source_loc;
};

/**
 * @brief Initialize the token given in the pointer
 *
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given type have only one spelling
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token(struct token* t, enum token_type type, char* spelling, struct source_location loc, const char* filename);

/**
 * @brief Initialize the token in the given pointer, copying the spelling
 *
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be NULL
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token_copy(struct token* t, enum token_type type, const char* spelling, struct source_location loc, const char* filename);

void free_token(struct token* t);

enum error_type {
    ERR_NONE,
    ERR_TOKENIZER,
    ERR_PARSER
};

/**
 * @brief Get the last error that was set
 *
 * @return enum error_type The last error that was set using set_error() or set_error_file()
 */
enum error_type get_last_error();

/**
 * @brief Clears the global error state, so get_last_error() returns ERR_NONE
 *
 */
void clear_last_error();

/**
 * @brief Get the message set by the last error. An error must have been set
 *
 * @return const char* The error set by the last call to set_error() or set_error_file()
 */
const char* get_error_string();

/**
 *
 * @param t The error type to be converted to a string
 * @return The enum constant as a string
 */
const char* get_error_type_str(enum error_type t);

/**
 * @brief Set the global error state
 *
 * @param type The error type to set
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void set_error(enum error_type type, const char* format, ...);

/**
 * @brief Sets the global error state, specifying the location of the error in the file
 *
 * @param type The error type to set
 * @param filename Filename of the file that was trying to be compiled
 * @param loc SourceLocation in the given file
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void set_error_file(enum error_type type, const char* filename, struct source_location loc, const char* format, ...);

/**
 * @brief Appends to the already existing error message. The error must have already been set
 *
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void append_error_msg(const char* format, ...);

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
 * @brief Grows an existing allocation, writing the new allocation and its size in the given pointers
 *
 * @param alloc Pointer to existing allocation, to which the resulting allocation will be written
 * @param alloc_len Pointer to number of allocated elements, to which the new number of elements
 *                  will be written
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

struct token_arr {
    struct token* tokens;
    size_t len;
    size_t alloc_len;
};

struct tokenizer_state {
    const char* it;
    char prev;
    char prev_prev;
    struct source_location source_loc;
    const char* current_file;
};

static enum token_type multic_token_type(const char* spelling);
static enum token_type singlec_token_type(char c);
static enum token_type check_next(enum token_type type, const char* next);
static bool token_is_over(const struct tokenizer_state* s);
static bool is_valid_singlec_token(enum token_type type, char prev, char prev_prev);

static void advance(struct tokenizer_state* s, size_t num);
static void advance_one(struct tokenizer_state* s);
static void advance_newline(struct tokenizer_state* s);

static void add_token_copy(struct token_arr* res, enum token_type type, const char* spell, struct source_location loc, const char* filename);
static void add_token(struct token_arr* res, enum token_type type, char* spell, struct source_location loc, const char* filename);

static bool handle_comments(struct tokenizer_state* s);

static bool handle_character_literal(struct tokenizer_state* s, struct token_arr* res);
static bool handle_other(struct tokenizer_state* s, struct token_arr* res);

struct token* tokenize(const char* str, const char* filename) {
    enum {NUM_START_TOKENS = 1};
    struct tokenizer_state s = {
        .it = str,
        .prev = '\0',
        .prev_prev = '\0',
        .source_loc = (struct source_location){1, 1},
        .current_file = filename
    };

    struct token_arr res = {
        .tokens = xmalloc(sizeof(struct token) * NUM_START_TOKENS),
        .len = 0,
        .alloc_len = NUM_START_TOKENS
    };

    while (*s.it != '\0') {
        while (isspace(*s.it)) {
            advance_newline(&s);
        }
        if (*s.it == '\0') {
            break;
        }

        enum token_type type = singlec_token_type(*s.it);
        if (type != INVALID && is_valid_singlec_token(type, s.prev, s.prev_prev)) {
            if (type == DIV) {
                if (handle_comments(&s)) {
                    continue;
                } else if (get_last_error() != ERR_NONE) {
                    goto fail;
                }
            }
            if (s.it[1] != '\0') {
                type = check_next(type, s.it + 1);
            }

            add_token(&res, type, NULL, s.source_loc, s.current_file);

            size_t len = strlen(get_spelling(type));
            advance(&s, len);
        } else if (*s.it == '\"' || *s.it == '\'' || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) {
            if (!handle_character_literal(&s, &res)) {
                goto fail;
            }
        } else {
            if (!handle_other(&s, &res)) {
                goto fail;
            }
        }
    }

    res.tokens = xrealloc(res.tokens, sizeof(struct token) * (res.len + 1));
    res.tokens[res.len] = (struct token){
        .type = INVALID,
        .spelling = NULL,
        .file = NULL,
        .source_loc = {(size_t) -1, (size_t) -1}
    };

    return res.tokens;

    fail:
    for (size_t i = 0; i < res.len; ++i) {
        free_token(&res.tokens[i]);
    }
    free(res.tokens);
    return NULL;
}

void free_tokenizer_result(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    free(tokens);
}

static inline bool is_spelling(const char* spelling, enum token_type type) {
    return strcmp(spelling, get_spelling(type)) == 0;
}

static enum token_type multic_token_type(const char* spell) {
    if (is_spelling(spell, FUNC_NAME)) {
        return FUNC_NAME;
    } else if (is_spelling(spell, SIZEOF)) {
        return SIZEOF;
    } else if (is_spelling(spell, TYPEDEF)) {
        return TYPEDEF;
    } else if (is_spelling(spell, EXTERN)) {
        return EXTERN;
    } else if (is_spelling(spell, STATIC)) {
        return STATIC;
    } else if (is_spelling(spell, AUTO)) {
        return AUTO;
    } else if (is_spelling(spell, REGISTER)) {
        return REGISTER;
    } else if (is_spelling(spell, INLINE)) {
        return INLINE;
    } else if (is_spelling(spell, BOOL)) {
        return BOOL;
    } else if (is_spelling(spell, CHAR)) {
        return CHAR;
    } else if (is_spelling(spell, SHORT)) {
        return SHORT;
    } else if (is_spelling(spell, INT)) {
        return INT;
    } else if (is_spelling(spell, LONG)) {
        return LONG;
    } else if (is_spelling(spell, SIGNED)) {
        return SIGNED;
    }else if (is_spelling(spell, UNSIGNED)) {
        return UNSIGNED;
    } else if (is_spelling(spell, FLOAT)) {
        return FLOAT;
    } else if (is_spelling(spell, DOUBLE)) {
        return DOUBLE;
    } else if (is_spelling(spell, VOID)) {
        return VOID;
    } else if (is_spelling(spell, COMPLEX)) {
        return COMPLEX;
    } else if (is_spelling(spell, IMAGINARY)) {
        return IMAGINARY;
    } else if (is_spelling(spell, CONST)) {
        return CONST;
    } else if (is_spelling(spell, VOLATILE)) {
        return VOLATILE;
    } else if (is_spelling(spell, RESTRICT)) {
        return RESTRICT;
    } else if (is_spelling(spell, ATOMIC)) {
        return ATOMIC;
    } else if (is_spelling(spell, STRUCT)) {
        return STRUCT;
    } else if (is_spelling(spell, UNION)) {
        return UNION;
    } else if (is_spelling(spell, ENUM)) {
        return ENUM;
    } else if (is_spelling(spell, CASE)) {
        return CASE;
    } else if (is_spelling(spell, DEFAULT)) {
        return DEFAULT;
    } else if (is_spelling(spell, IF)) {
        return IF;
    } else if (is_spelling(spell, ELSE)) {
        return ELSE;
    } else if (is_spelling(spell, SWITCH)) {
        return SWITCH;
    } else if (is_spelling(spell, WHILE)) {
        return WHILE;
    } else if (is_spelling(spell, DO)) {
        return DO;
    } else if (is_spelling(spell, FOR)) {
        return FOR;
    } else if (is_spelling(spell, GOTO)) {
        return GOTO;
    } else if (is_spelling(spell, CONTINUE)) {
        return CONTINUE;
    } else if (is_spelling(spell, BREAK)) {
        return BREAK;
    } else if (is_spelling(spell, RETURN)) {
        return RETURN;
    } else if (is_spelling(spell, ALIGNAS)) {
        return ALIGNAS;
    } else if (is_spelling(spell, ALIGNOF)) {
        return ALIGNOF;
    } else if (is_spelling(spell, GENERIC)) {
        return GENERIC;
    } else if (is_spelling(spell, NORETURN)) {
        return NORETURN;
    } else if (is_spelling(spell, STATIC_ASSERT)) {
        return STATIC_ASSERT;
    } else if (is_spelling(spell, THREAD_LOCAL)) {
        return THREAD_LOCAL;
    } else {
        return INVALID;
    }
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
        default:
            break;
    }

    return type;
}

static bool is_valid_singlec_token(enum token_type type, char prev, char prev_prev) {
    assert(type != INVALID);
    if ((type == DOT && isdigit(prev)) || ((type == SUB || type == ADD) && tolower((unsigned char)prev) == 'e' && isdigit(prev_prev))) {
        return false;
    } else {
        return true;
    }
}

static void advance(struct tokenizer_state* s, size_t num) {
    assert(num > 0);
    s->it += num;
    s->source_loc.index += num;

    if (num > 1 || s->prev != '\0') {
        s->prev = *(s->it - 1);
        s->prev_prev = *(s->it - 2);
    }
}

static void advance_one(struct tokenizer_state* s) {
    ++s->source_loc.index;
    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static void advance_newline(struct tokenizer_state* s) {
    if (*s->it == '\n') {
        s->source_loc.line += 1;
        s->source_loc.index = 1;
    } else {
        ++s->source_loc.index;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static void realloc_tokens_if_needed(struct token_arr* res) {
    if (res->len == res->alloc_len) {
        grow_alloc((void**)&res->tokens, &res->alloc_len, sizeof(struct token));
    }
}

static void add_token_copy(struct token_arr* res, enum token_type type, const char* spell, struct source_location loc, const char* filename) {
    realloc_tokens_if_needed(res);
    init_token_copy(&res->tokens[res->len], type, spell, loc, filename);
    ++res->len;
}

static void add_token(struct token_arr* res, enum token_type type, char* spell, struct source_location loc, const char* filename) {
    realloc_tokens_if_needed(res);
    init_token(&res->tokens[res->len], type, spell, loc, filename);
    ++res->len;
}

static bool handle_comments(struct tokenizer_state* s) {
    if (s->it[1] == '*') {
        advance(s, 2);
        while (*s->it != '\0' && *s->it != '*' && s->it[1] != '/') {
            advance_newline(s);
        }

        if (*s->it == '\0') {
            set_error_file(ERR_TOKENIZER, s->current_file, s->source_loc, "Comment was not properly terminated");
            return false;
        } else {
            assert(*s->it == '*');
            assert(s->it[1] == '/');
            advance(s, 2);
            return true;
        }
    } else if (s->it[1] == '/') {
        while (*s->it != '\0' && *s->it != '\n') {
            advance_one(s);
        }
        return true;
    } else {
        return false;
    }
}

static enum token_type get_char_lit_type(const char* buf, size_t len, char terminator) {
    if (terminator == '\"' && is_string_literal(buf, len)) {
        return STRING_LITERAL;
    } else if (terminator == '\'' && is_char_const(buf, len)) {
        return I_CONSTANT;
    } else {
        return INVALID;
    }
}

static void unterminated_literal_err(char terminator, struct source_location start_loc, const char* filename) {
    const char* literal_type_str;
    if (terminator == '\"') {
        literal_type_str = "String";
    } else {
        literal_type_str = "Char";
    }
    set_error_file(ERR_TOKENIZER, filename, start_loc, "%s literal not properly terminated", literal_type_str);
}

static bool handle_character_literal(struct tokenizer_state* s, struct token_arr* res) {
    assert(*s->it == '\'' || *s->it == '\"' || *s->it == 'L');
    enum {BUF_STRLEN = 512};
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    const struct source_location start_loc = s->source_loc;

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
    while (*s->it != '\0' && (s->prev == '\\' || *s->it != terminator) && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_newline(s);
    }

    char* dyn_buf = NULL;
    if (buf_idx == BUF_STRLEN) {
        size_t buf_len = BUF_STRLEN + BUF_STRLEN / 2;
        dyn_buf = xmalloc(buf_len * sizeof(char));
        strcpy(dyn_buf, spell_buf);

        while (*s->it != '\0' && *s->it != terminator) {
            if (buf_idx == buf_len - 1) {
                grow_alloc((void**)&dyn_buf, &buf_len, sizeof(char));
            }

            dyn_buf[buf_idx] = *s->it;
            ++buf_idx;

            advance_newline(s);
        }

        dyn_buf = xrealloc(dyn_buf, (buf_idx + 2) * sizeof(char));
    }

    if (*s->it == '\0') {
        if (dyn_buf != NULL) {
            free(dyn_buf);
        }
        unterminated_literal_err(terminator, start_loc, s->current_file);
        return false;
    } else {
        bool is_dyn = dyn_buf != NULL;
        if (is_dyn) {
            dyn_buf[buf_idx] = *s->it;
            dyn_buf[buf_idx + 1] = '\0';
        } else {
            spell_buf[buf_idx] = *s->it;
        }
        ++buf_idx;

        advance_one(s);

        enum token_type type = get_char_lit_type(is_dyn ? dyn_buf : spell_buf, buf_idx, terminator);

        if (type == INVALID) {
            if (is_dyn) {
                free(dyn_buf);
            }
            set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Terminated literal is of unknown type");
            return false;
        }

        if (is_dyn) {
            add_token(res, type, dyn_buf, start_loc, s->current_file);
        } else {
            add_token_copy(res, type, spell_buf, start_loc, s->current_file);
        }
    }

    return true;
}

static bool token_is_over(const struct tokenizer_state* s) {
    enum token_type type = singlec_token_type(*s->it);
    return *s->it == '\0' || isspace(*s->it) || (type != INVALID && is_valid_singlec_token(type, s->prev, s->prev_prev));
}

static bool handle_other(struct tokenizer_state* s, struct token_arr* res) {
    enum {BUF_STRLEN = 512};
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    struct source_location start_loc = s->source_loc;
    while (!token_is_over(s) && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
    }

    char* dyn_buf = NULL;
    if (!token_is_over(s)) {
        size_t buf_len = BUF_STRLEN + BUF_STRLEN / 2;
        dyn_buf = xmalloc(buf_len * sizeof(char));
        strcpy(dyn_buf, spell_buf);

        while (!token_is_over(s)) {
            if (buf_idx == buf_len - 1) {
                grow_alloc((void**)&dyn_buf, &buf_len, sizeof(char));
            }

            dyn_buf[buf_idx] = *s->it;
            ++buf_idx;

            advance_one(s);
        }

        dyn_buf = xrealloc(dyn_buf, (buf_idx + 1) * sizeof(char));
        dyn_buf[buf_idx] = '\0';
    }

    char* buf_to_check = dyn_buf != NULL ? dyn_buf : spell_buf;
    enum token_type type = multic_token_type(buf_to_check);
    if (type != INVALID) {
        add_token(res, type, NULL, start_loc, s->current_file);
    } else {
        if (is_dec_const(buf_to_check, buf_idx) || is_hex_const(buf_to_check, buf_idx) || is_oct_const(buf_to_check, buf_idx)) {
            type = I_CONSTANT;
        } else if (is_float_const(buf_to_check, buf_idx)) {
            type = F_CONSTANT;
        } else if (is_valid_identifier(buf_to_check, buf_idx)) {
            type = IDENTIFIER;
        } else {
            if (buf_to_check == dyn_buf) {
                free(dyn_buf);
            }
            set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Invalid identifier");
            return false;
        }

        if (buf_to_check == dyn_buf) {
            add_token(res, type, dyn_buf, start_loc, s->current_file);
        } else {
            add_token_copy(res, type, spell_buf, start_loc, s->current_file);
        }
    }

    return true;
}

