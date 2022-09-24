#include "frontend/preproc/regex.h"

#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

static bool is_dec_const(const char* str, size_t num);
static bool is_hex_const(const char* str, size_t num);
static bool is_oct_const(const char* str, size_t num);

bool is_int_const(const char* str, size_t num) {
    return is_dec_const(str, num) || is_hex_const(str, num)
           || is_oct_const(str, num);
}

static bool is_id_char(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static bool is_hex_digit(char c) {
    unsigned char uc = (unsigned char)c;
    return isdigit(c) || (tolower(uc) >= 'a' && tolower(uc) <= 'f');
}

static bool is_float_suffix(char c) {
    unsigned char uc = (unsigned char)c;
    return tolower(uc) == 'l' || tolower(uc) == 'f';
}

static bool is_exp_suffix(const char* str, size_t num, bool is_hex) {
    const char exp_char = is_hex ? 'p' : 'e';
    size_t i = 0;
    if (num < 2 || tolower((unsigned char)str[0]) != exp_char) {
        return false;
    }
    ++i;

    if ((str[i] != '+' && str[i] != '-' && !isdigit(str[i]))
        || (!isdigit(str[i]) && i + 1 == num)) {
        return false;
    }

    ++i;

    while (i != num && tolower(str[i]) != 'f' && tolower(str[i]) != 'l') {
        if (!isdigit(str[i])) {
            return false;
        }
        ++i; 
    }

    if (i != num) {
        if (i != num - 1) {
            return false;
        }
        return is_float_suffix(str[i]);
    } else {
        return true;
    }
}

static bool is_int_suffix(const char* str, size_t num) {
    for (size_t i = 0; i != num; ++i) {
        unsigned char uc = (unsigned char)str[i];
        if (tolower(uc) != 'u' && tolower(uc) != 'l') {
            return false;
        }
    }
    return true;
}

static bool is_hex_const(const char* str, size_t num) {
    size_t i = 0;
    if (num < 3 || str[i] != '0') {
        return false;
    }
    ++i;
    if (tolower((unsigned char)str[i]) != 'x') {
        return false;
    }
    ++i;

    if (!is_hex_digit(str[i])) {
        return false;
    }
    ++i;

    while (i != num && is_hex_digit(str[i])) {
        ++i;
    }

    if (i == num || is_int_suffix(str + i, num - i)) {
        return true;
    } else {
        return false;
    }
}

static bool is_oct_const_start(const char* str, size_t num) {
    return num >= 2 && str[0] == '0';
}

static bool is_oct_const(const char* str, size_t num) {
    size_t i = 0;
    if (!is_oct_const_start(str, num)) {
        return false;
    }
    ++i;

    while (i != num && isdigit(str[i])) {
        ++i;
    }

    if (i == num || is_int_suffix(str + i, num - i)) {
        return true;
    } else {
        return false;
    }
}

static bool is_dec_const(const char* str, size_t num) {
    size_t i = 0;
    if (num < 1 || !isdigit(str[i]) || is_oct_const_start(str, num)) {
        return false;
    }
    ++i;

    while (i != num && isdigit(str[i])) {
        ++i;
    }

    if (i == num || is_int_suffix(str + i, num - i)) {
        return true;
    } else {
        return false;
    }
}

bool is_char_const(const char* str, size_t num) {
    size_t last = num - 1;
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

static bool is_float_const_no_dec_point(const char* str, size_t num) {
    size_t i = 0;
    if (num < 1 || !isdigit(str[i])) {
        return false;
    }
    ++i;

    while (i != num && isdigit(str[i])) {
        ++i;
    }

    if (is_exp_suffix(str + i, num - i - 1, false)
        || is_exp_suffix(str + i, num - i, false)) {
        return true;
    } else {
        return false;
    }
}

static bool is_after_decimal_float_const(const char* str, size_t num) {
    if (num < 2) {
        return false;
    }

    size_t i = 0;
    while (i != num && isdigit(str[i])) {
        ++i;
    }

    if (i == num || str[i] != '.') {
        return false;
    }
    ++i;

    while (i != num && isdigit(str[i])) {
        ++i;
    }

    if (i == num || (i == num - 1 && is_float_suffix(str[num - 1]))
        || (is_float_suffix(str[num - 1])
            && is_exp_suffix(str + i, num - i - 1, false))
        || is_exp_suffix(str + i, num - i, false)) {
        return true;
    } else {
        return false;
    }
}

static bool is_hex_float_const(const char* str, size_t num);

bool is_float_const(const char* str, size_t num) {
    assert(num > 0);
    if (str[0] == '0' && num >= 2 && tolower(str[1]) == 'x') {
        return is_hex_float_const(str, num);
    } else {
        return is_float_const_no_dec_point(str, num)
               || is_after_decimal_float_const(str, num);
    }
}

static bool is_hex_float_const(const char* str, size_t num) {
    assert(num >= 2);
    assert(str[0] == '0' && tolower(str[1]) == 'x');

    size_t i = 2;
    while (i < num && tolower(str[i]) != 'p') {
        if (!is_hex_digit(str[i]) && str[i] != '.') {
            return false;
        }
        ++i;
    }

    if (i != num) {
        return is_exp_suffix(str + i, num - i, true);
    } else {
        return true;
    }
}

bool is_string_literal(const char* str, size_t num) {
    size_t last = num - 1;
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

bool is_valid_identifier(const char* str, size_t num) {
    if (!isalpha(str[0]) && str[0] != '_') {
        return false;
    }
    for (size_t i = 1; i != num; ++i) {
        if (!is_id_char(str[i])) {
            return false;
        }
    }

    return true;
}
