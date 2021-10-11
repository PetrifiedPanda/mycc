#include "tokenizer.h"

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "regex.h"
#include "error.h"

typedef struct {
    Token* tokens;
    size_t len;
} TokenArr;

typedef struct {
    const char* it;
    char prev;
    char prev_prev;
    size_t idx;
    size_t line;
} TokenizerState;

static TokenType multic_token_type(const char* spelling);
static TokenType singlec_token_type(char c);
static TokenType check_next(TokenType type, const char* next);
static bool token_is_over(const TokenizerState* s);
static bool is_valid_singlec_token(TokenType type, char prev, char prev_prev);

static inline void advance(TokenizerState* s, size_t num);
static inline void advance_one(TokenizerState* s);
static inline void advance_newline(TokenizerState* s);

static inline bool add_token(size_t* token_idx, TokenArr* res, TokenType type, const char* spell, size_t line, size_t start_idx);
static inline bool add_token_move(size_t* token_idx, TokenArr* res, TokenType type, char* spell, size_t line, size_t start_idx);

static TokenType get_lit_type(const char* buf, size_t len, char terminator);
static void unterminated_literal_err(char terminator, size_t line, size_t start_idx);

static bool handle_comments(TokenizerState* s);

static bool handle_character_literal(TokenizerState* s, TokenArr* res, size_t* token_idx);
static bool handle_other(TokenizerState* s, TokenArr* res, size_t* token_idx);

Token* tokenize(const char* str) {
    enum {NUM_START_TOKENS = 1};
    TokenizerState s = {str, '\0', '\0', 0, 0};

    TokenArr res = {malloc(sizeof(Token) * NUM_START_TOKENS), NUM_START_TOKENS};
    if (!res.tokens) {
        set_error(ERR_ALLOC_FAIL, "Failed to allocate token array");
        goto fail;
    }

    size_t token_idx = 0;

    while (*s.it != '\0') {
        while (isspace(*s.it)) {
            advance_newline(&s);
        }

        TokenType type = singlec_token_type(*s.it);
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
            const char* spelling = get_spelling(type);
            if (!add_token(&token_idx, &res, type, spelling, s.line, s.idx)) {
                goto fail;
            }

            size_t len = strlen(spelling);
            advance(&s, len);
        } else if (*s.it == '\"' || *s.it == '\'' || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) { // TODO: literals starting with L
            if (!handle_character_literal(&s, &res, &token_idx)) {
                goto fail;
            }
        } else {
            if (!handle_other(&s, &res, &token_idx)) {
                goto fail;
            }
        }
    }

    Token* new_tokens = realloc(res.tokens, sizeof(Token) * (token_idx + 1));
    if (!new_tokens) {
        set_error(ERR_ALLOC_FAIL, "Failed to resize token array");
        goto fail;
    }
    res.tokens = new_tokens;
    res.tokens[token_idx] = (Token){INVALID, NULL, (size_t)-1, (size_t)-1};
    res.len = token_idx;

    return res.tokens;

fail:
    free(res.tokens);
    res.tokens = NULL;
    res.len = 0;
    return res.tokens;
}

static TokenType multic_token_type(const char* spelling) {
    if (strcmp(spelling, "sizeof") == 0) {
        return SIZEOF;
    } else if (strcmp(spelling, "typedef") == 0) {
        return TYPEDEF;
    } else if (strcmp(spelling, "extern") == 0) {
        return EXTERN;
    } else if (strcmp(spelling, "static") == 0) {
        return STATIC;
    } else if (strcmp(spelling, "auto") == 0) {
        return AUTO;
    } else if (strcmp(spelling, "register") == 0) {
        return REGISTER;
    } else if (strcmp(spelling, "char") == 0) {
        return CHAR;
    } else if (strcmp(spelling, "short") == 0) {
        return SHORT;
    } else if (strcmp(spelling, "int") == 0) {
        return INT;
    } else if (strcmp(spelling, "long") == 0) {
        return LONG;
    } else if (strcmp(spelling, "unsigned") == 0) {
        return UNSIGNED;
    } else if (strcmp(spelling, "float") == 0) {
        return FLOAT;
    } else if (strcmp(spelling, "double") == 0) {
        return DOUBLE;
    } else if (strcmp(spelling, "const") == 0) {
        return CONST;
    } else if (strcmp(spelling, "volatile") == 0) {
        return VOLATILE;
    } else if (strcmp(spelling, "void") == 0) {
        return VOID;
    } else if (strcmp(spelling, "struct") == 0) {
        return STRUCT;
    } else if (strcmp(spelling, "union") == 0) {
        return UNION;
    } else if (strcmp(spelling, "case") == 0) {
        return CASE;
    } else if (strcmp(spelling, "default") == 0) {
        return DEFAULT;
    } else if (strcmp(spelling, "if") == 0) {
        return IF;
    } else if (strcmp(spelling, "else") == 0) {
        return ELSE;
    } else if (strcmp(spelling, "switch") == 0) {
        return SWITCH;
    } else if (strcmp(spelling, "while") == 0) {
        return WHILE;
    } else if (strcmp(spelling, "do") == 0) {
        return DO;
    } else if (strcmp(spelling, "for") == 0) {
        return FOR;
    } else if (strcmp(spelling, "goto") == 0) {
        return GOTO;
    } else if (strcmp(spelling, "continue") == 0) {
        return CONTINUE;
    } else if (strcmp(spelling, "break") == 0) {
        return BREAK;
    } else if (strcmp(spelling, "return") == 0) {
        return RETURN;
    } else {
        return INVALID;
    }
}

static TokenType singlec_token_type(char c) {
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

static bool check_type(TokenType type, const char* next_chars) {
    const char* spelling = get_spelling(type);
    size_t len = strlen(spelling);
    assert(len != 0);
    assert(len >= 2);
    return strncmp(spelling + 1, next_chars, len - 1) == 0;
}

static TokenType check_next(TokenType type, const char* next) {
    assert(next[0] != '\0');
    switch (type) {
    case ADD: {
        if (check_type(ADD_ASSIGN, next)) {
            return ADD_ASSIGN;
        } else if (check_type(INC_OP, next)) {
            return INC_OP;
        } else {
            break;
        }
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
    }

    return type;
}

static bool token_is_over(const TokenizerState* s) {
    TokenType type = singlec_token_type(*s->it);
    return *s->it == '\0' || isspace(*s->it) || (type != INVALID && is_valid_singlec_token(type, s->prev, s->prev_prev));
}

static bool is_valid_singlec_token(TokenType type, char prev, char prev_prev) {
    assert(type != INVALID);
    if (type == DOT && isdigit(prev)) {
        return false;
    } else if ((type == SUB || type == ADD) && tolower(prev) == 'e' && isdigit(prev_prev)) {
        return false;
    } else {
        return true;
    }
}

static inline void advance(TokenizerState* s, size_t num) {
    assert(num > 0);
    s->it += num;
    s->idx += num;
    
    if (num > 1 || s->prev != '\0') {
        s->prev = *(s->it - 1);
        s->prev_prev = *(s->it - 2);
    }
}

static inline void advance_one(TokenizerState* s) {
    ++s->idx;
    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static inline void advance_newline(TokenizerState* s) {
    if (*s->it == '\n') {
        s->line += 1;
        s->idx = 0;
    } else {
        ++s->idx;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static bool realloc_tokens_if_needed(size_t token_idx, TokenArr* res) {
    if (token_idx == res->len) {
        res->len *= 2;
        Token* new_tokens = realloc(res->tokens, sizeof(Token) * res->len);
        if (!new_tokens) {
            set_error(ERR_ALLOC_FAIL, "Failed to reallocate Token Array");
            return false;
        }
        res->tokens = new_tokens;
    }

    return true;
}

static inline bool add_token(size_t* token_idx, TokenArr* res, TokenType type, const char* spell, size_t line, size_t start_idx) {
    if (!realloc_tokens_if_needed(*token_idx, res)) {
        return false;
    }
    if (!create_token(&res->tokens[*token_idx], type, spell, line, start_idx)) {
        return false;
    }
    ++*token_idx;

    return true;
}

static inline bool add_token_move(size_t* token_idx, TokenArr* res, TokenType type, char* spell, size_t line, size_t start_idx) {
    if (!realloc_tokens_if_needed(*token_idx, res)) {
        return false;
    }
    create_token_move(&res->tokens[*token_idx], type, spell, line, start_idx);
    ++*token_idx;
    return true;
}

static TokenType get_lit_type(const char* buf, size_t len, char terminator) {
    if (terminator == '\"' && is_string_literal(buf, len)) {
        return STRING_LITERAL;
    } else if (terminator == '\'' && is_char_const(buf, len)) {
        return CONSTANT;
    } else {
        return INVALID;
    }
}

static void unterminated_literal_err(char terminator, size_t line, size_t start_idx) {
    const char* literal_type_str;
    if (terminator == '\"') {
        literal_type_str = "String";
    } else {
        literal_type_str = "Char";
    }
    set_error(ERR_TOKENIZER, "%s literal at line %ul char %ul not properly terminated", literal_type_str, line, start_idx);
}

static bool handle_comments(TokenizerState* s) {
    if (s->it[1] == '*') {
        advance(s, 2);
        while (*s->it != '\0' && *s->it != '*' && s->it[1] != '/') {
            advance_newline(s);
        }

        if (*s->it == '\0') {
            set_error(ERR_TOKENIZER, "Comment was not properly terminated");
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

static bool handle_character_literal(TokenizerState* s, TokenArr* res, size_t* token_idx) {
    assert(*s->it == '\'' || *s->it == '\"' || *s->it == 'L');
    enum {BUF_STRLEN = 1024};
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    size_t start_idx = s->idx;

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

    while (*s->it != '\0' && *s->it != terminator && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_newline(s);
    }
    if (*s->it == '\0') {
        unterminated_literal_err(terminator, s->line, start_idx);
        return false;
    } else if (buf_idx == BUF_STRLEN) {
        size_t dyn_buf_len = BUF_STRLEN + BUF_STRLEN / 2;
        size_t dyn_buf_lim = dyn_buf_len - 1;
        char* dyn_buf = malloc(sizeof(char) * dyn_buf_len);
        if (!dyn_buf) {
            set_error(ERR_ALLOC_FAIL, "Could not allocate buffer for long literal");
            return false;
        }
        strcpy(dyn_buf, spell_buf);

        while (*s->it != '\0' && *s->it != terminator) {
            if (buf_idx == dyn_buf_lim) {
                dyn_buf_len += dyn_buf_len / 2;
                char* new_buf = realloc(dyn_buf, dyn_buf_len);
                if (!new_buf) {
                    set_error(ERR_ALLOC_FAIL, "Could not enlarge buffer for long string literal");
                    free(dyn_buf);
                    return false;
                } else {
                    dyn_buf = new_buf;
                    dyn_buf_lim = dyn_buf_len - 1;
                }
            }
            dyn_buf[buf_idx] = *s->it;
            dyn_buf[buf_idx + 1] = '\0';
            ++buf_idx;

            advance_newline(s);
        }
        char* new_buf = realloc(dyn_buf, buf_idx + 2);
        if (!new_buf) {
            set_error(ERR_ALLOC_FAIL, "Could not resize buffer for long string literal");
            free(dyn_buf);
            return false;
        }
        dyn_buf = new_buf;
        dyn_buf[buf_idx] = *s->it;
        dyn_buf[buf_idx + 1] = '\0';
        ++buf_idx;
        advance_one(s);

        TokenType type = get_lit_type(dyn_buf, buf_idx, terminator);

        if (type == INVALID || !add_token_move(token_idx, res, type, dyn_buf, s->line, start_idx)) {
            if (type == INVALID) {
                set_error(ERR_TOKENIZER, "Terminated literal is of unknown type");
            }
            free(dyn_buf);
            return false;
        }
    } else {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
                    
        TokenType type = get_lit_type(spell_buf, buf_idx, terminator);

        if (type == INVALID || !add_token(token_idx, res, type, spell_buf, s->line, start_idx)) {
            if (type == INVALID) {
                set_error(ERR_TOKENIZER, "Terminated literal is of unknown type");
            }
            return false;
        }
    }

    return true;
}

static bool handle_other(TokenizerState* s, TokenArr* res, size_t* token_idx) {
    enum {BUF_STRLEN = 100, MAX_IDENTIFIER_LEN = 31};
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    size_t start_idx = s->idx;
    TokenType type = INVALID;
    while (!token_is_over(s) && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
        type = multic_token_type(spell_buf);
    }

    if (type != INVALID && token_is_over(s)) {
        if (!add_token(token_idx, res, type, spell_buf, s->line, start_idx)) {
            return false;
        }
    } else if (token_is_over(s)) {
        TokenType type = INVALID;
        if (is_hex_const(spell_buf, buf_idx) || is_oct_const(spell_buf, buf_idx) || is_dec_const(spell_buf, buf_idx) || is_float_const(spell_buf, buf_idx)) {
            type = CONST;
        } else if (buf_idx <= MAX_IDENTIFIER_LEN && is_valid_identifier(spell_buf, buf_idx)) {
            type = IDENTIFIER;
        } else {
            set_error(ERR_TOKENIZER, "Invalid identifier");
            return false;
        }

        if (!add_token(token_idx, res, type, spell_buf, s->line, start_idx)) {
            return false;
        }
    } else {
        set_error(ERR_TOKENIZER, "Identifier too long");
        return false; // TOKEN too long 
    }

    return true;
}
