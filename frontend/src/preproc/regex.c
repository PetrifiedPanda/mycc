#include "regex.h"

#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

static bool is_dec_const(Str str);
static bool is_hex_const(Str str);
static bool is_oct_const(Str str);

static bool is_oct_or_hex_const_start(Str str);

bool is_int_const(Str str) {
    if (is_oct_or_hex_const_start(str)) {
        if (str.len >= 2 && tolower((unsigned char)Str_at(str, 1)) == 'x') {
            return is_hex_const(str);
        } else {
            return is_oct_const(str);
        }
    } else {
        return is_dec_const(str);
    }
}

static bool is_hex_digit(char c) {
    unsigned char uc = (unsigned char)c;
    return isdigit(c) || (tolower(uc) >= 'a' && tolower(uc) <= 'f');
}

static bool is_int_suffix(Str str) {
    for (size_t i = 0; i != str.len; ++i) {
        unsigned char uc = (unsigned char)Str_at(str, i);
        if (tolower(uc) != 'u' && tolower(uc) != 'l') {
            return false;
        }
    }
    return true;
}

static bool is_hex_const(Str str) {
    assert(str.len > 2 && Str_at(str, 0) == '0' && tolower((unsigned char)Str_at(str, 1)) == 'x');

    size_t i = 2;
    if (!is_hex_digit(Str_at(str, i))) {
        return false;
    }
    ++i;

    while (i != str.len && is_hex_digit(Str_at(str, i))) {
        ++i;
    }

    if (i == str.len || is_int_suffix(Str_advance(str, i))) {
        return true;
    } else {
        return false;
    }
}

static bool is_oct_or_hex_const_start(Str str) {
    return str.len >= 2 && Str_at(str, 0) == '0';
}

static bool is_oct_const(Str str) {
    assert(is_oct_or_hex_const_start(str)
           && tolower((unsigned char)Str_at(str, 1)) != 'x');
    size_t i = 1;

    while (i != str.len && isdigit(Str_at(str, i))) {
        ++i;
    }

    if (i == str.len || is_int_suffix(Str_advance(str, i))) {
        return true;
    } else {
        return false;
    }
}

static bool is_dec_const(Str str) {
    assert(str.len > 0 && !is_oct_or_hex_const_start(str));
    if (!isdigit(Str_at(str, 0))) {
        return false;
    }
    size_t i = 1;

    while (i != str.len && isdigit(Str_at(str, i))) {
        ++i;
    }

    if (i == str.len || is_int_suffix(Str_advance(str, i))) {
        return true;
    } else {
        return false;
    }
}

bool is_char_const(Str str) {
    size_t last = str.len - 1;
    size_t i = 0;
    if (Str_at(str, i) == 'L') {
        ++i;
    }

    if (Str_at(str, i) != '\'' || Str_at(str, last) != '\'') {
        return false;
    }

    ++i;

    char prev = Str_at(str, i - 1);
    for (; i != last; ++i) {
        if (Str_at(str, i) == '\'' && prev != '\\') {
            return false;
        }

        prev = Str_at(str, i);
    }

    return true;
}

static bool is_dec_float_const(Str str);
static bool is_hex_float_const(Str str);

bool is_float_const(Str str) {
    assert(str.len > 0);
    if (Str_at(str, 0) == '0' && str.len >= 2 && tolower(Str_at(str, 1)) == 'x') {
        return is_hex_float_const(str);
    } else {
        return is_dec_float_const(str);
    }
}

static bool is_float_suffix(char c) {
    unsigned char uc = (unsigned char)c;
    return tolower(uc) == 'l' || tolower(uc) == 'f';
}

static bool is_exp_suffix(Str str, bool is_hex) {
    const char exp_char = is_hex ? 'p' : 'e';
    size_t i = 0;
    if (str.len < 2 || tolower((unsigned char)Str_at(str, 0)) != exp_char) {
        return false;
    }
    ++i;

    if ((Str_at(str, i) != '+' && Str_at(str, i) != '-' && !isdigit(Str_at(str, i)))
        || (!isdigit(Str_at(str, i)) && i + 1 == str.len)) {
        return false;
    }

    ++i;

    while (i != str.len && tolower(Str_at(str, i)) != 'f' && tolower(Str_at(str, i)) != 'l') {
        if (!isdigit(Str_at(str, i))) {
            return false;
        }
        ++i;
    }

    if (i != str.len) {
        if (i != str.len - 1) {
            return false;
        }
        return is_float_suffix(Str_at(str, i));
    } else {
        return true;
    }
}

static bool is_dec_float_const(Str str) {

    size_t i = 0;
    while (i != str.len && isdigit(Str_at(str, i))) {
        ++i;
    }

    if (i == str.len) {
        return false;
    } else if (Str_at(str, i) == '.') {
        ++i;
        while (i != str.len && isdigit(Str_at(str, i))) {
            ++i;
        }
        if (i == str.len || (i == str.len - 1 && is_float_suffix(Str_at(str, str.len - 1)))
            || is_exp_suffix(Str_advance(str, i), false)) {
            return true;
        } else {
            return false;
        }
    } else if (Str_at(str, i) == 'e') {
        return is_exp_suffix(Str_advance(str, i), false);
    } else {
        return false;
    }
}

static bool is_hex_float_const(Str str) {
    assert(str.len >= 2);
    assert(Str_at(str, 0) == '0' && tolower(Str_at(str, 1)) == 'x');

    size_t i = 2;
    while (i < str.len && tolower(Str_at(str, i)) != 'p') {
        if (!is_hex_digit(Str_at(str, i)) && Str_at(str, i) != '.') {
            return false;
        }
        ++i;
    }

    if (i != str.len) {
        return is_exp_suffix(Str_advance(str, i), true);
    } else {
        return true;
    }
}

bool is_string_literal(Str str) {
    size_t last = str.len - 1;
    size_t i = 0;
    if (Str_at(str, i) == 'L') {
        ++i;
    }

    if (Str_at(str, i) != '\"' || Str_at(str, last) != '\"') {
        return false;
    }
    ++i;
    char prev = Str_at(str, i - 1);
    for (; i != last; ++i) {
        if ((Str_at(str, i) == '\"' || Str_at(str, i) == '\n') && prev != '\\') {
            return false;
        }
        prev = Str_at(str, i);
    }

    return true;
}

static bool is_id_char(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

bool is_valid_identifier(Str str) {
    if (!isalpha(Str_at(str, 0)) && Str_at(str, 0) != '_') {
        return false;
    }
    for (size_t i = 1; i != str.len; ++i) {
        if (!is_id_char(Str_at(str, i))) {
            return false;
        }
    }

    return true;
}

