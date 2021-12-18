#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

struct parser_state {
    struct token* it;
};

bool is_enum_constant(const struct parser_state* s, const char* spell);
bool is_typedef_name(const struct parser_state* s, const char* spell);

#endif
