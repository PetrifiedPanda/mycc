#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "frontend/token.h"

#include "parser_err.h"

#include "util/string_hash_map.h"

struct parser_state {
    struct token* it;
    size_t _len;
    struct string_hash_map* _scope_maps;
    struct parser_err* err;
};

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err);
void free_parser_state(struct parser_state* s);

bool accept(struct parser_state* s, enum token_type expected);
void accept_it(struct parser_state* s);

void parser_push_scope(struct parser_state* s);
void parser_pop_scope(struct parser_state* s);

bool register_enum_constant(struct parser_state* s, const struct token* token);
bool register_typedef_name(struct parser_state* s, const struct token* token);

bool is_enum_constant(const struct parser_state* s, const char* spell);
bool is_typedef_name(const struct parser_state* s, const char* spell);

#endif

