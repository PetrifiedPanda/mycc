#include "preproc/tokenizer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "token.h"
#include "error.h"

#include "util/mem.h"

#include "preproc/regex.h"

struct tokenizer_state {
    const char* it;
    char prev;
    char prev_prev;
    struct source_location source_loc;
    const char* current_file;
};

static enum token_type singlec_token_type(char c);
static enum token_type check_next(enum token_type type, const char* next);

static bool token_is_over(const struct tokenizer_state* s);
static bool is_valid_singlec_token(enum token_type type,
                                   char prev,
                                   char prev_prev);

static void advance(struct tokenizer_state* s, size_t num);
static void advance_one(struct tokenizer_state* s);
static void advance_newline(struct tokenizer_state* s);

static void add_token_copy(struct preproc_state* res,
                           enum token_type type,
                           const char* spell,
                           struct source_location loc,
                           const char* filename);
static void add_token(struct preproc_state* res,
                      enum token_type type,
                      char* spell,
                      struct source_location loc,
                      const char* filename);

static void handle_comments(struct tokenizer_state* s,
                            bool* comment_not_terminated);
static bool handle_character_literal(struct tokenizer_state* s,
                                     struct preproc_state* res);
static bool handle_other(struct tokenizer_state* s, struct preproc_state* res);
static void handle_ongoing_comment(struct tokenizer_state* s,
                                   bool* comment_not_terminated);

bool tokenize_line(struct preproc_state* res,
                   const char* line,
                   size_t line_num,
                   const char* file,
                   bool* comment_not_terminated) {
    assert(res);
    assert(line);
    assert(file);
    assert(comment_not_terminated);

    struct tokenizer_state s = {
        .it = line,
        .prev = '\0',
        .prev_prev = '\0',
        .source_loc =
            {
                .line = line_num,
                .index = 1,
            },
        .current_file = file,
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
        if (type != INVALID
            && is_valid_singlec_token(type, s.prev, s.prev_prev)) {
            if (type == DIV && (s.it[1] == '/' || s.it[1] == '*')) {
                handle_comments(&s, comment_not_terminated);
                continue;
            }
            if (s.it[1] != '\0') {
                type = check_next(type, s.it + 1);
            }

            add_token(res, type, NULL, s.source_loc, s.current_file);

            size_t len = strlen(get_spelling(type));
            advance(&s, len);
        } else if (*s.it == '\"' || *s.it == '\''
                   || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) {
            if (!handle_character_literal(&s, res)) {
                return false;
            }
        } else {
            if (!handle_other(&s, res)) {
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

static bool is_valid_singlec_token(enum token_type type,
                                   char prev,
                                   char prev_prev) {
    assert(type != INVALID);
    if ((type == DOT && isdigit(prev))
        || ((type == SUB || type == ADD) && tolower((unsigned char)prev) == 'e'
            && isdigit(prev_prev))) {
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
        assert(s->it[-1] == '\\');
        s->source_loc.line += 1;
        s->source_loc.index = 1;
    } else {
        ++s->source_loc.index;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
}

static void realloc_tokens_if_needed(struct preproc_state* res) {
    if (res->len == res->cap) {
        grow_alloc((void**)&res->tokens, &res->cap, sizeof(struct token));
    }
}

static void add_token_copy(struct preproc_state* res,
                           enum token_type type,
                           const char* spell,
                           struct source_location loc,
                           const char* filename) {
    realloc_tokens_if_needed(res);
    res->tokens[res->len] = create_token_copy(type, spell, loc, filename);
    ++res->len;
}

static void add_token(struct preproc_state* res,
                      enum token_type type,
                      char* spell,
                      struct source_location loc,
                      const char* filename) {
    realloc_tokens_if_needed(res);
    res->tokens[res->len] = create_token(type, spell, loc, filename);
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

static void unterminated_literal_err(char terminator,
                                     struct source_location start_loc,
                                     const char* filename) {
    const char* literal_type_str;
    if (terminator == '\"') {
        literal_type_str = "String";
    } else {
        literal_type_str = "Char";
    }
    set_error_file(ERR_TOKENIZER,
                   filename,
                   start_loc,
                   "%s literal not properly terminated",
                   literal_type_str);
}

static bool handle_character_literal(struct tokenizer_state* s,
                                     struct preproc_state* res) {
    assert(*s->it == '\'' || *s->it == '\"' || *s->it == 'L');
    enum { BUF_STRLEN = 512 };
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
    while (
        *s->it != '\0'
        && ((s->prev_prev != '\\' && s->prev == '\\') || *s->it != terminator)
        && buf_idx != BUF_STRLEN) {
        spell_buf[buf_idx] = *s->it;
        ++buf_idx;

        advance_newline(s);
    }

    char* dyn_buf = NULL;
    if (buf_idx == BUF_STRLEN) {
        size_t buf_len = BUF_STRLEN + BUF_STRLEN / 2;
        dyn_buf = xmalloc(buf_len * sizeof(char));
        strcpy(dyn_buf, spell_buf);

        while (*s->it != '\0' 
               && ((s->prev_prev != '\\' && s->prev == '\\') || *s->it != terminator)) {
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

        enum token_type type = get_char_lit_type(is_dyn ? dyn_buf : spell_buf,
                                                 buf_idx,
                                                 terminator);

        if (type == INVALID) {
            if (is_dyn) {
                free(dyn_buf);
            }
            set_error_file(ERR_TOKENIZER,
                           s->current_file,
                           start_loc,
                           "Terminated literal is of unknown type");
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
    return *s->it == '\0' || isspace(*s->it)
           || (type != INVALID
               && is_valid_singlec_token(type, s->prev, s->prev_prev));
}

static bool handle_other(struct tokenizer_state* s, struct preproc_state* res) {
    enum { BUF_STRLEN = 512 };
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
    enum token_type type;
    if (is_dec_const(buf_to_check, buf_idx)
        || is_hex_const(buf_to_check, buf_idx)
        || is_oct_const(buf_to_check, buf_idx)) {
        type = I_CONSTANT;
    } else if (is_float_const(buf_to_check, buf_idx)) {
        type = F_CONSTANT;
    } else if (is_valid_identifier(buf_to_check, buf_idx)) {
        type = IDENTIFIER;
    } else {
        set_error_file(ERR_TOKENIZER,
                       s->current_file,
                       start_loc,
                       "Invalid identifier: %s",
                       buf_to_check);
        if (buf_to_check == dyn_buf) {
            free(dyn_buf);
        }
        return false;
    }

    if (buf_to_check == dyn_buf) {
        add_token(res, type, dyn_buf, start_loc, s->current_file);
    } else {
        add_token_copy(res, type, spell_buf, start_loc, s->current_file);
    }

    return true;
}
