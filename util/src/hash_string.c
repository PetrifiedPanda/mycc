#include "hash_string.h"

// Hash function taken from K&R version 2 (page 144)
uint32_t hash_string(Str str) {
    uint32_t hash = 0;

    uint32_t i = 0;
    while (i != str.len) {
        hash = Str_at(str, i) + 32 * hash;
        ++i;
    }
    return hash;
}
