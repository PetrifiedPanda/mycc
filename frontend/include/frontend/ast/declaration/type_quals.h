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

/*
 * @brief update type quals with current token, which must already be a type
 *        qual
 */
void update_type_quals(struct parser_state* s, struct type_quals* quals);

bool parse_type_qual_list(struct parser_state* s, struct type_quals* res);

#endif

