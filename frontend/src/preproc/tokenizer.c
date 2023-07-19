#include "tokenizer.h"

#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/Token.h"

#include "regex.h"

typedef struct {
    Str it;
    char prev;
    char prev_prev;
    FileLoc file_loc;
    size_t current_file_idx;
} TokenizerState;

static TokenKind singlec_token_kind(char c);
static TokenKind check_next(TokenKind kind, Str next);
static bool is_singlec_token(const TokenizerState* s, TokenKind t);

static void advance(TokenizerState* s, size_t num);
static void advance_one(TokenizerState* s);
static void advance_newline(TokenizerState* s);

static void handle_comments(TokenizerState* s, bool* comment_not_terminated);
static void handle_ongoing_comment(TokenizerState* s,
                                   bool* comment_not_terminated);

static bool handle_character_literal(TokenizerState* s,
                                     Token* res,
                                     LineInfo* info,
                                     PreprocErr* err);
static bool handle_other(TokenizerState* s,
                         Token* res,
                         LineInfo* info,
                         PreprocErr* err);

static void write_line_info(const TokenizerState* s, LineInfo* info) {
    info->next = s->it;
    info->curr_loc.file_loc = s->file_loc;
}

bool tokenize_next_token(Token* res, PreprocErr* err, LineInfo* info) {
    assert(res);
    assert(info);
    assert(info->next.data);
    assert(info->curr_loc.file_idx != (size_t)-1);

    TokenizerState s = {
        .it = info->next,
        .prev = '\0',
        .prev_prev = '\0',
        .file_loc = info->curr_loc.file_loc,
        .current_file_idx = info->curr_loc.file_idx,
    };

    while (isspace(*s.it.data)) {
        advance_newline(&s);
    }
    if (*s.it.data == '\0') {
        const StrBuf null_str = StrBuf_null();
        *res = (Token){
            .kind = TOKEN_INVALID,
            .spelling = null_str,
            .loc =
                {
                    .file_idx = (size_t)-1,
                    .file_loc = (FileLoc){(size_t)-1, (size_t)-1},
                },
        };
        write_line_info(&s, info);
        return true;
    } else if (*s.it.data == '\\') {
        const SourceLoc loc = {
            s.current_file_idx,
            s.file_loc,
        };
        advance_one(&s);
        while (isblank(*s.it.data)) {
            advance_one(&s);
        }
        if (*s.it.data != '\n') {
            PreprocErr_set(err, PREPROC_ERR_INVALID_BACKSLASH, loc);
            return false;
        }
        advance_newline(&s);
        write_line_info(&s, info);
        return tokenize_next_token(res, err, info);
    }
    if (info->is_in_comment) {
        handle_ongoing_comment(&s, &info->is_in_comment);
        write_line_info(&s, info);
        return tokenize_next_token(res, err, info);
    }

    TokenKind kind = singlec_token_kind(Str_at(s.it, 0));
    if (kind != TOKEN_INVALID && is_singlec_token(&s, kind)) {
        if (kind == TOKEN_DIV
            && (s.it.len > 1
                && (Str_at(s.it, 1) == '/' || Str_at(s.it, 1) == '*'))) {
            handle_comments(&s, &info->is_in_comment);
            write_line_info(&s, info);
            return tokenize_next_token(res, err, info);
        }
        if (s.it.len != 1) {
            kind = check_next(kind, Str_incr(s.it));
        }

        const StrBuf null_str = StrBuf_null();
        *res = Token_create(kind, &null_str, s.file_loc, s.current_file_idx);

        size_t len = TokenKind_get_spelling(kind).len;
        advance(&s, len);

        write_line_info(&s, info);
        return true;
    } else if (Str_at(s.it, 0) == '\"' || Str_at(s.it, 0) == '\''
               || (Str_at(s.it, 0) == 'L'
                   && (s.it.len > 1
                       && (Str_at(s.it, 1) == '\"'
                           || Str_at(s.it, 1) == '\'')))) {
        return handle_character_literal(&s, res, info, err);
    } else {
        return handle_other(&s, res, info, err);
    }
}

bool tokenize_line(TokenArr* res, PreprocErr* err, LineInfo* info) {
    assert(res);
    assert(info->next.data);
    assert(info->curr_loc.file_idx != (size_t)-1);

    while (*info->next.data != '\0') {
        Token curr;
        do {
            if (!tokenize_next_token(&curr, err, info)) {
                return false;
            }
        } while (curr.kind == TOKEN_INVALID && *info->next.data != '\0');

        if (curr.kind != TOKEN_INVALID) {
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

static TokenKind singlec_token_kind(char c) {
    switch (c) {
        case ';':
            return TOKEN_SEMICOLON;
        case '(':
            return TOKEN_LBRACKET;
        case ')':
            return TOKEN_RBRACKET;
        case '{':
            return TOKEN_LBRACE;
        case '}':
            return TOKEN_RBRACE;
        case '[':
            return TOKEN_LINDEX;
        case ']':
            return TOKEN_RINDEX;
        case '.':
            return TOKEN_DOT;
        case '&':
            return TOKEN_AND;
        case '|':
            return TOKEN_OR;
        case '^':
            return TOKEN_XOR;
        case '!':
            return TOKEN_NOT;
        case '~':
            return TOKEN_BNOT;
        case '-':
            return TOKEN_SUB;
        case '+':
            return TOKEN_ADD;
        case '*':
            return TOKEN_ASTERISK;
        case '/':
            return TOKEN_DIV;
        case '%':
            return TOKEN_MOD;
        case '<':
            return TOKEN_LT;
        case '>':
            return TOKEN_GT;
        case '?':
            return TOKEN_QMARK;
        case ':':
            return TOKEN_COLON;
        case '=':
            return TOKEN_ASSIGN;
        case ',':
            return TOKEN_COMMA;
        case '#':
            return TOKEN_PP_STRINGIFY;
        default:
            return TOKEN_INVALID;
    }
}

static bool check_kind(TokenKind kind, Str next_chars) {
    Str spell = TokenKind_get_spelling(kind);
    assert(spell.len != 0);
    assert(spell.len >= 2);
    return Str_eq(Str_incr(spell), Str_substr(next_chars, 0, spell.len - 1));
}

static TokenKind check_next(TokenKind kind, Str next) {
    assert(next.len != 0);
    switch (kind) {
        case TOKEN_ADD:
            if (check_kind(TOKEN_ADD_ASSIGN, next)) {
                return TOKEN_ADD_ASSIGN;
            } else if (check_kind(TOKEN_INC, next)) {
                return TOKEN_INC;
            } else {
                break;
            }
        case TOKEN_SUB:
            if (check_kind(TOKEN_PTR_OP, next)) {
                return TOKEN_PTR_OP;
            } else if (check_kind(TOKEN_DEC, next)) {
                return TOKEN_DEC;
            } else if (check_kind(TOKEN_SUB_ASSIGN, next)) {
                return TOKEN_SUB_ASSIGN;
            } else {
                break;
            }
        case TOKEN_ASTERISK:
            if (check_kind(TOKEN_MUL_ASSIGN, next)) {
                return TOKEN_MUL_ASSIGN;
            } else {
                break;
            }
        case TOKEN_DIV:
            if (check_kind(TOKEN_DIV_ASSIGN, next)) {
                return TOKEN_DIV_ASSIGN;
            } else {
                break;
            }
        case TOKEN_LT:
            if (check_kind(TOKEN_LSHIFT_ASSIGN, next)) {
                return TOKEN_LSHIFT_ASSIGN;
            } else if (check_kind(TOKEN_LSHIFT, next)) {
                return TOKEN_LSHIFT;
            } else if (check_kind(TOKEN_LE, next)) {
                return TOKEN_LE;
            } else {
                break;
            }
        case TOKEN_GT:
            if (check_kind(TOKEN_RSHIFT_ASSIGN, next)) {
                return TOKEN_RSHIFT_ASSIGN;
            } else if (check_kind(TOKEN_RSHIFT, next)) {
                return TOKEN_RSHIFT;
            } else if (check_kind(TOKEN_GE, next)) {
                return TOKEN_GE;
            } else {
                break;
            }
        case TOKEN_AND:
            if (check_kind(TOKEN_LAND, next)) {
                return TOKEN_LAND;
            } else if (check_kind(TOKEN_AND_ASSIGN, next)) {
                return TOKEN_AND_ASSIGN;
            } else {
                break;
            }
        case TOKEN_OR:
            if (check_kind(TOKEN_LOR, next)) {
                return TOKEN_LOR;
            } else if (check_kind(TOKEN_OR_ASSIGN, next)) {
                return TOKEN_OR_ASSIGN;
            } else {
                break;
            }
        case TOKEN_XOR:
            if (check_kind(TOKEN_XOR_ASSIGN, next)) {
                return TOKEN_XOR_ASSIGN;
            } else {
                break;
            }
        case TOKEN_MOD:
            if (check_kind(TOKEN_MOD_ASSIGN, next)) {
                return TOKEN_MOD_ASSIGN;
            } else {
                break;
            }
        case TOKEN_DOT:
            if (check_kind(TOKEN_ELLIPSIS, next)) {
                return TOKEN_ELLIPSIS;
            } else {
                break;
            }
        case TOKEN_ASSIGN:
            if (check_kind(TOKEN_EQ, next)) {
                return TOKEN_EQ;
            } else {
                break;
            }
        case TOKEN_NOT:
            if (check_kind(TOKEN_NE, next)) {
                return TOKEN_NE;
            } else {
                break;
            }
        case TOKEN_PP_STRINGIFY:
            if (check_kind(TOKEN_PP_CONCAT, next)) {
                return TOKEN_PP_CONCAT;
            } else {
                break;
            }
        default:
            break;
    }

    return kind;
}

static bool is_singlec_token(const TokenizerState* s, TokenKind t) {
    switch (t) {
        case TOKEN_DOT:
            return s->it.len < 2 || !isdigit(s->it.data[1]);
        default:
            return true;
    }
}

static void advance(TokenizerState* s, size_t num) {
    assert(num > 0);
    s->it = Str_advance(s->it, num);
    s->file_loc.index += num;

    if (num > 1 || s->prev != '\0') {
        s->prev = *(s->it.data - 1);
        s->prev_prev = *(s->it.data - 2);
    }
}

static void advance_one(TokenizerState* s) {
    ++s->file_loc.index;
    s->prev_prev = s->prev;
    s->prev = *s->it.data;
    s->it = Str_incr(s->it);
}

static void advance_newline(TokenizerState* s) {
    if (*s->it.data == '\n') {
        s->file_loc.line += 1;
        s->file_loc.index = 1;
    } else {
        ++s->file_loc.index;
    }

    s->prev_prev = s->prev;
    s->prev = *s->it.data;
    s->it = Str_incr(s->it);
}

static void handle_comments(TokenizerState* s, bool* comment_not_terminated) {
    assert(*s->it.data == '/');
    assert(s->it.data[1] == '*' || s->it.data[1] == '/');

    assert(s->it.data[1] == '*' || s->it.data[1] == '/');
    if (s->it.data[1] == '*') {
        advance(s, 2);

        handle_ongoing_comment(s, comment_not_terminated);
    } else {
        while (*s->it.data != '\0' && *s->it.data != '\n') {
            advance_one(s);
        }
    }
}

static void handle_ongoing_comment(TokenizerState* s,
                                   bool* comment_not_terminated) {
    assert(comment_not_terminated);

    while (*s->it.data != '\0'
           && (*s->it.data != '*' || s->it.data[1] != '/')) {
        advance_newline(s);
    }

    if (*s->it.data == '\0') {
        *comment_not_terminated = true;
    } else {
        assert(*s->it.data == '*');
        assert(s->it.data[1] == '/');
        advance(s, 2);
        *comment_not_terminated = false;
    }
}

static TokenKind get_char_lit_kind(Str spell, char terminator) {
    if (terminator == '\"' && is_string_literal(spell)) {
        return TOKEN_STRING_LITERAL;
    } else if (terminator == '\'' && is_char_const(spell)) {
        return TOKEN_I_CONSTANT;
    } else {
        return TOKEN_INVALID;
    }
}

static void unterminated_literal_err(PreprocErr* err,
                                     char terminator,
                                     FileLoc start_loc,
                                     size_t file_idx) {
    const bool is_char_lit = terminator == '\'';

    const SourceLoc loc = {
        .file_idx = file_idx,
        .file_loc = start_loc,
    };
    PreprocErr_set(err, PREPROC_ERR_UNTERMINATED_LIT, loc);

    err->is_char_lit = is_char_lit;
}

static bool handle_character_literal(TokenizerState* s,
                                     Token* res,
                                     LineInfo* info,
                                     PreprocErr* err) {
    assert(*s->it.data == '\'' || *s->it.data == '\"' || *s->it.data == 'L');
    Str spell_view = {
        .len = 0,
        .data = s->it.data,
    };
    const FileLoc start_loc = s->file_loc;

    char terminator;
    if (*s->it.data == 'L') {
        ++spell_view.len;

        advance_one(s);
        assert(*s->it.data == '\"' || *s->it.data == '\'');
    }

    terminator = *s->it.data;
    ++spell_view.len;

    advance_one(s);
    while (*s->it.data != '\0'
           && ((s->prev_prev != '\\' && s->prev == '\\')
               || *s->it.data != terminator)) {
        ++spell_view.len;

        advance_newline(s);
    }

    StrBuf spell = StrBuf_create(spell_view);

    if (*s->it.data == '\0') {
        StrBuf_free(&spell);
        unterminated_literal_err(err,
                                 terminator,
                                 start_loc,
                                 s->current_file_idx);
        return false;
    } else {
        StrBuf_push_back(&spell, *s->it.data);

        advance_one(s);

        const TokenKind kind = get_char_lit_kind(StrBuf_as_str(&spell),
                                                 terminator);
        assert(kind != TOKEN_INVALID);
        *res = Token_create(kind, &spell, start_loc, s->current_file_idx);

        write_line_info(s, info);
        return true;
    }
}

static bool token_is_over(const TokenizerState* s, bool is_num) {
    if (*s->it.data == '\0' || isspace(*s->it.data)) {
        return true;
    }
    TokenKind kind = singlec_token_kind(*s->it.data);
    if (kind == TOKEN_INVALID) {
        return false;
    } else if (is_num) {
        switch (kind) {
            case TOKEN_SUB:
            case TOKEN_ADD: {
                char prev = (char)tolower((unsigned char)s->prev);
                return (prev != 'e' && prev != 'p');
            }
            case TOKEN_DOT:
                return false;
            default:
                return true;
        }
    } else {
        return true;
    }
}

static bool handle_other(TokenizerState* s,
                         Token* res,
                         LineInfo* info,
                         PreprocErr* err) {
    const bool is_num = isdigit(*s->it.data) || *s->it.data == '.';
    Str spell_view = {
        .len = 0,
        .data = s->it.data,
    };
    const FileLoc start_loc = s->file_loc;
    while (!token_is_over(s, is_num)) {
        ++spell_view.len;

        advance_one(s);
    }

    StrBuf spell = StrBuf_create(spell_view);

    TokenKind kind;
    if (is_num) {
        if (is_int_const(spell_view)) {
            kind = TOKEN_I_CONSTANT;
        } else if (is_float_const(spell_view)) {
            kind = TOKEN_F_CONSTANT;
        } else {
            const SourceLoc loc = {
                .file_idx = s->current_file_idx,
                .file_loc = start_loc,
            };
            PreprocErr_set(err, PREPROC_ERR_INVALID_NUMBER, loc);
            err->invalid_num = spell;
            return false;
        }
    } else if (is_valid_identifier(spell_view)) {
        kind = TOKEN_IDENTIFIER;
    } else {
        const SourceLoc loc = {
            .file_idx = s->current_file_idx,
            .file_loc = start_loc,
        };
        PreprocErr_set(err, PREPROC_ERR_INVALID_ID, loc);

        err->invalid_id = spell;
        return false;
    }

    *res = Token_create(kind, &spell, start_loc, s->current_file_idx);
    write_line_info(s, info);
    return true;
}

