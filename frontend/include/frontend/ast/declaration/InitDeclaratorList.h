#ifndef MYCC_FRONTEND_DECLARATION_INIT_DECLARATOR_LIST_H
#define MYCC_FRONTEND_DECLARATION_INIT_DECLARATOR_LIST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct InitDeclarator InitDeclarator;

typedef struct {
    uint32_t len;
    InitDeclarator* decls;
} InitDeclaratorList;

/**
 *
 * @param s The current state
 * @param first_decl A heap allocated init declarator
 * @return A list parsed with first_decl as the
 * first element in the list
 */
bool parse_init_declarator_list_first(ParserState* s,
                                      InitDeclaratorList* res,
                                      InitDeclarator* first_decl);

bool parse_init_declarator_list(ParserState* s, InitDeclaratorList* res);

bool parse_init_declarator_list_typedef_first(ParserState* s,
                                              InitDeclaratorList* res,
                                              InitDeclarator* first_decl);

bool parse_init_declarator_list_typedef(ParserState* s,
                                        InitDeclaratorList* res);

void InitDeclaratorList_free(InitDeclaratorList* l);

#endif

