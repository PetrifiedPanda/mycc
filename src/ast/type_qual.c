#include "ast/type_qual.h"

#include <stdlib.h>
#include <assert.h>

#include "parser/parser_util.h"

struct type_qual parse_type_qual(struct parser_state* s) {
    switch (s->it->type) {
        case CONST:
        case RESTRICT:
        case VOLATILE:
        case ATOMIC: {
            enum token_type type = s->it->type;
            accept_it(s);
            return (struct type_qual){.type = type};
        }

        default: {
            enum token_type expected[] = {
                    CONST,
                    RESTRICT,
                    VOLATILE,
                    ATOMIC
            };

            expected_tokens_error(expected, sizeof(expected) / sizeof(enum token_type), s->it);

            return (struct type_qual){.type = INVALID};
        }
    }
}

