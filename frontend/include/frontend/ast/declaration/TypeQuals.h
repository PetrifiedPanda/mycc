#ifndef MYCC_FRONTEND_DECLARATION_TYPE_QUALS_H
#define MYCC_FRONTEND_DECLARATION_TYPE_QUALS_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

typedef struct TypeQuals {
    bool is_const: 1;
    bool is_restrict: 1;
    bool is_volatile: 1;
    bool is_atomic: 1;
} TypeQuals;

TypeQuals TypeQuals_create(void);

/*
 * @brief update type quals with current token, which must already be a type
 *        qual
 */
void update_type_quals(ParserState* s, TypeQuals* quals);

bool parse_type_qual_list(ParserState* s, TypeQuals* res);

#endif

