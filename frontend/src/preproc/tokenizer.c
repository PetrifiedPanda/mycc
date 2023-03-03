#include "frontend/preproc/tokenizer.h"

#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/token.h"

#include "frontend/preproc/regex.h"

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

static void handle_comments(struct tokenizer_state* s,
                            bool* comment_not_terminated);
static void handle_ongoing_comment(struct tokenizer_state* s,
                                   bool* comment_not_terminated);

static bool handle_character_literal(struct tokenizer_state* s,
                                     struct token* res,
                                     struct line_info* info,
                                     struct preproc_err* err);
static bool handle_other(struct tokenizer_state* s,
                         struct token* res,
                         struct line_info* info,
                         struct preproc_err* err);

static void write_line_info(const struct tokenizer_state* s,
                            struct line_info* info) {
    info->next = s->it;
    info->curr_loc.file_loc = s->file_loc;
}

// TODO: what to do when comment is whole line
bool next_preproc_token(struct token* res,
                        struct preproc_err* err,
                        struct line_info* info) {
    assert(res);
    assert(info);
    assert(info->next);
    assert(info->curr_loc.file_idx != (size_t)-1);

    struct tokenizer_state s = {
        .it = info->next,
        .prev = '\0',
        .prev_prev = '\0',
        .file_loc = info->curr_loc.file_loc,
        .current_file_idx = info->curr_loc.file_idx,
    };

    while (isspace(*s.it)) {
        // TODO: how to handle escaped newlines
        advance_newline(&s);
    }
    if (*s.it == '\0') {
        // TODO: not sure if this is the best idea
        const struct str null_str = create_null_str();
        *res = (struct token){
            .type = INVALID,
            .spelling = null_str,
            .loc =
                {
                    .file_idx = (size_t)-1,
                    .file_loc = (struct file_loc){(size_t)-1, (size_t)-1},
                },
        };
        write_line_info(&s, info);
        return true;
    }
    if (info->is_in_comment) {
        handle_ongoing_comment(&s, &info->is_in_comment);
        write_line_info(&s, info);
        return next_preproc_token(res, err, info);
    }

    enum token_type type = singlec_token_type(*s.it);
    if (type != INVALID && is_singlec_token(&s, type)) {
        if (type == DIV && (s.it[1] == '/' || s.it[1] == '*')) {
            handle_comments(&s, &info->is_in_comment);
            write_line_info(&s, info);
            return next_preproc_token(res, err, info);
        }
        if (s.it[1] != '\0') {
            type = check_next(type, s.it + 1);
        }

        struct str null_str = create_null_str();
        *res = create_token(type, &null_str, s.file_loc, s.current_file_idx);

        size_t len = strlen(get_spelling(type));
        advance(&s, len);

        write_line_info(&s, info);
        return true;
    } else if (*s.it == '\"' || *s.it == '\''
               || (*s.it == 'L' && (s.it[1] == '\"' || s.it[1] == '\''))) {
        return handle_character_literal(&s, res, info, err);
    } else {
        return handle_other(&s, res, info, err);
    }
}

bool tokenize_line(struct token_arr* res,
                   struct preproc_err* err,
                   struct line_info* info) {
    assert(res);
    assert(info->next);
    assert(info->curr_loc.file_idx != (size_t)-1);

    while (*info->next != '\0') {
        struct token curr;
        do {
            if (!next_preproc_token(&curr, err, info)) {
                return false;
            }
        } while (curr.type == INVALID && *info->next != '\0');

        if (curr.type != INVALID) {
            if (res->len == res->cap) {
                mycc_grow_alloc((void**)&res->tokens,
                                &res->cap,
                                sizeof *res->tokens);
            }
            res->tokens[res->len] = curr;
            ++res->len;
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
        //assert(s->it[-1] == '\\');
        s->file_loc.line += 1;
        s->file_loc.index = 1;
    } else {
        ++s->file_loc.index;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it;
    ++s->it;
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
                                     struct token* res,
                                     struct line_info* info,
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
        *res = create_token(type, &spell, start_loc, s->current_file_idx);

        write_line_info(s, info);
        return true;
    }
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
                         struct token* res,
                         struct line_info* info,
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

    *res = create_token(type, &spell, start_loc, s->current_file_idx);
    write_line_info(s, info);
    return true;
}

