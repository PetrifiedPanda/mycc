#include "frontend/preproc/regex.h"

#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

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

