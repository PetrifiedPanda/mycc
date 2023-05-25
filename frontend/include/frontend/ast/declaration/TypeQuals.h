#ifndef TYPE_QUALS_H
#define TYPE_QUALS_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

typedef struct TypeQuals {
    bool is_const;
    bool is_restrict;
    bool is_volatile;
    bool is_atomic;
} TypeQuals;

TypeQuals create_type_quals(void);

/*
 * @brief update type quals with current token, which must already be a type
 *        qual
 */
void update_type_quals(ParserState* s, TypeQuals* quals);

bool parse_type_qual_list(ParserState* s, TypeQuals* res);

#endif

