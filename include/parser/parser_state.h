#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "token.h"

struct identifier_type_pair;

struct identifier_type_map {
    struct identifier_type_pair* pairs;
    size_t len;
    size_t cap;
};

struct parser_state {
    struct token* it;
    struct identifier_type_map map;
};

struct parser_state create_parser_state(struct token* tokens);
void free_parser_state(struct parser_state* s);

bool accept(struct parser_state* s, enum token_type expected);
void accept_it(struct parser_state* s);

bool register_enum_constant(struct parser_state* s, const struct token* token);
bool register_typedef_name(struct parser_state* s, const struct token* token);

bool is_enum_constant(const struct parser_state* s, const char* spell);
bool is_typedef_name(const struct parser_state* s, const char* spell);

#endif
