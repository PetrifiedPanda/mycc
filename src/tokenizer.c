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
    SourceLocation source_loc;
    const char* current_file;
} TokenizerState;

static TokenType multic_token_type(const char* spelling);
static TokenType singlec_token_type(char c);
static TokenType check_next(TokenType type, const char* next);
static bool token_is_over(const TokenizerState* s);
static bool is_valid_singlec_token(TokenType type, char prev, char prev_prev);

static inline void advance(TokenizerState* s, size_t num);
static inline void advance_one(TokenizerState* s);
static inline void advance_newline(TokenizerState* s);

static inline bool add_token(size_t* token_idx, TokenArr* res, TokenType type, const char* spell, SourceLocation loc, const char* filename);
static inline bool add_token_move(size_t* token_idx, TokenArr* res, TokenType type, char* spell, SourceLocation loc, const char* filename);

static bool handle_comments(TokenizerState* s);

static bool handle_character_literal(TokenizerState* s, TokenArr* res, size_t* token_idx);
static bool handle_other(TokenizerState* s, TokenArr* res, size_t* token_idx);

Token* tokenize(const char* str, const char* filename) {
    enum {NUM_START_TOKENS = 1};
    TokenizerState s = {str, '\0', '\0', (SourceLocation){1, 1}, filename};

    TokenArr res = {malloc(sizeof(Token) * NUM_START_TOKENS), NUM_START_TOKENS};
    if (!res.tokens) {
        set_error(ERR_ALLOC_FAIL, "Failed to allocate token array at the start");
        goto fail;
    }

    size_t token_idx = 0;

    while (*s.it != '\0') {
        while (isspace(*s.it)) {
            advance_newline(&s);
        }
        if (*s.it == '\0') {
            break;
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
            
            if (!add_token(&token_idx, &res, type, NULL, s.source_loc, s.current_file)) {
                goto fail;
            }

            size_t len = strlen(get_spelling(type));
            advance(&s, len);
        } else if (*s.it == '\"' || *s.it == '\'' || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) {
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
    res.tokens[token_idx] = (Token){INVALID, NULL, NULL, (SourceLocation){(size_t) -1, (size_t) -1}};
    res.len = token_idx;

    return res.tokens;

fail:
    for (size_t i = 0; i < token_idx; ++i) {
        free_token(&res.tokens[i]);
    }
    free(res.tokens);
    res.tokens = NULL;
    res.len = 0;
    return res.tokens;
}

void free_tokenizer_result(Token* tokens) {
    for (Token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    free(tokens);
}

static inline bool is_spelling(const char* spelling, TokenType type){
    return strcmp(spelling, get_spelling(type)) == 0;
}

static TokenType multic_token_type(const char* spell) {
    if (is_spelling(spell, SIZEOF)) {
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
    } else if (is_spelling(spell, CONST)) {
        return CONST;
    } else if (is_spelling(spell, VOLATILE)) {
        return VOLATILE;
    } else if (is_spelling(spell, VOID)) {
        return VOID;
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
    s->source_loc.index += num;
    
    if (num > 1 || s->prev != '\0') {
        s->prev = *(s->it - 1);
        s->prev_prev = *(s->it - 2);
    }
}

static inline void advance_one(TokenizerState* s) {
    ++s->source_loc.index;
    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static inline void advance_newline(TokenizerState* s) {
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

static inline bool add_token(size_t* token_idx, TokenArr* res, TokenType type, const char* spell, SourceLocation loc, const char* filename) {
    if (!realloc_tokens_if_needed(*token_idx, res)) {
        return false;
    }
    if (!init_token(&res->tokens[*token_idx], type, spell, loc, filename)) {
        return false;
    }
    ++*token_idx;

    return true;
}

static inline bool add_token_move(size_t* token_idx, TokenArr* res, TokenType type, char* spell, SourceLocation loc, const char* filename) {
    if (!realloc_tokens_if_needed(*token_idx, res)) {
        return false;
    }
    if (!init_token_move(&res->tokens[*token_idx], type, spell, loc, filename)) {
        return false;
    }
    ++*token_idx;
    return true;
}

static bool handle_comments(TokenizerState* s) {
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

static TokenType get_lit_type(const char* buf, size_t len, char terminator) {
    if (terminator == '\"' && is_string_literal(buf, len)) {
        return STRING_LITERAL;
    } else if (terminator == '\'' && is_char_const(buf, len)) {
        return CONSTANT;
    } else {
        return INVALID;
    }
}

static void unterminated_literal_err(char terminator, SourceLocation start_loc, const char* filename) {
    const char* literal_type_str;
    if (terminator == '\"') {
        literal_type_str = "String";
    } else {
        literal_type_str = "Char";
    }
    set_error_file(ERR_TOKENIZER, filename, start_loc, "%s literal not properly terminated", literal_type_str);
}

static bool handle_character_literal(TokenizerState* s, TokenArr* res, size_t* token_idx) {
    assert(*s->it == '\'' || *s->it == '\"' || *s->it == 'L');
    enum {BUF_STRLEN = 2048};
    char spell_buf[BUF_STRLEN + 1] = {0};
    size_t buf_idx = 0;
    SourceLocation start_loc = s->source_loc;

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
        unterminated_literal_err(terminator, start_loc, s->current_file);
        return false;
    } else if (buf_idx == BUF_STRLEN) {
        set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Character Literal too long");
        return false;
    } else {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
                    
        TokenType type = get_lit_type(spell_buf, buf_idx, terminator);

        if (type == INVALID || !add_token(token_idx, res, type, spell_buf, start_loc, s->current_file)) {
            if (type == INVALID) {
                set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Terminated literal is of unknown type");
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
    SourceLocation start_loc = s->source_loc;
    while (!token_is_over(s) && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_one(s);
    }
    
    TokenType type = multic_token_type(spell_buf);
    if (type != INVALID && token_is_over(s)) {
        if (!add_token(token_idx, res, type, NULL, start_loc, s->current_file)) {
            return false;
        }
    } else if (token_is_over(s)) {
        assert(type == INVALID);
        if (is_hex_const(spell_buf, buf_idx) || is_oct_const(spell_buf, buf_idx) || is_dec_const(spell_buf, buf_idx) || is_float_const(spell_buf, buf_idx)) {
            type = CONSTANT;
        } else if (buf_idx <= MAX_IDENTIFIER_LEN && is_valid_identifier(spell_buf, buf_idx)) {
            type = IDENTIFIER;
        } else {
            set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Invalid identifier");
            return false;
        }

        if (!add_token(token_idx, res, type, spell_buf, start_loc, s->current_file)) {
            return false;
        }
    } else {
        set_error_file(ERR_TOKENIZER, s->current_file, start_loc, "Identifier too long");
        return false;
    }

    return true;
}
