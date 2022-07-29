#ifndef TYPE_QUALS_H
#define TYPE_QUALS_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

struct type_quals {
    bool is_const;
    bool is_restrict;
    bool is_volatile;
    bool is_atomic;
};

struct type_quals create_type_quals(void);

void update_type_quals(struct parser_state* s, struct type_quals* quals);

struct type_quals parse_type_qual_list(struct parser_state* s);

bool is_valid_type_quals(const struct type_quals* q);

#endif

