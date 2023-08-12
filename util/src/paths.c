#include "util/paths.h"

bool is_file_sep(char c) {
    switch (c) {
        case '/':
#ifdef _WIN32
        case '\\':
#endif
            return true;
        default:
            return false;
    }
}

uint32_t get_last_file_sep(Str path) {
    uint32_t i = path.len - 1;
    while (i != (uint32_t)-1 && !is_file_sep(Str_at(path, i))) {
        --i;
    }
    return i;
}
