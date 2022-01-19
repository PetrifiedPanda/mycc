#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "token.h"

struct parser_state {
    struct token* it;
};

bool accept(struct parser_state* s, enum token_type expected);
void accept_it(struct parser_state* s);

void register_enum_constant(struct parser_state* s, const char* spell);

bool is_enum_constant(const struct parser_state* s, const char* spell);
bool is_typedef_name(const struct parser_state* s, const char* spell);

#endif
