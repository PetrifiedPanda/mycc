#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "frontend/token.h"

#include "parser_err.h"

#include "util/string_hash_map.h"

struct parser_state {
    struct token* it;
    size_t _len, _cap;
    struct string_hash_map* _scope_maps;
    struct parser_err* err;
};

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err);
void free_parser_state(struct parser_state* s);

bool parser_accept(struct parser_state* s, enum token_kind expected);
void parser_accept_it(struct parser_state* s);

void parser_push_scope(struct parser_state* s);
void parser_pop_scope(struct parser_state* s);

bool parser_register_enum_constant(struct parser_state* s, const struct token* token);
bool parser_register_typedef_name(struct parser_state* s, const struct token* token);

bool parser_is_enum_constant(const struct parser_state* s, const struct str* spell);
bool parser_is_typedef_name(const struct parser_state* s, const struct str* spell);

struct parser_identifier_data;

const struct parser_identifier_data* parser_get_prev_definition(
    const struct parser_state* s,
    const struct str* spell);

void parser_set_redefinition_err(struct parser_state* s,
                                 const struct parser_identifier_data* prev_def,
                                 const struct token* redef_tok);
#endif

