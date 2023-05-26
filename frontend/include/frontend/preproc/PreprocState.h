#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

#include "util/StringMap.h"

#include "frontend/FileInfo.h"

#include "PreprocErr.h"

typedef struct {
    size_t len, cap;
    Token* tokens;
} TokenArr;

typedef struct {
    Str line;
    const char* next;
    SourceLoc curr_loc;
    bool is_in_comment;
    char static_buf[200];
} LineInfo;

typedef struct {
    bool had_true_branch;
    bool had_else;
    SourceLoc loc;
} PreprocCond;

typedef struct OpenedFileInfo OpenedFileInfo;

typedef struct {
    FILE* files[FOPEN_MAX];
    size_t opened_info_indices[FOPEN_MAX];
    size_t current_file_idx;
    size_t opened_info_len, opened_info_cap;
    OpenedFileInfo* opened_info;
    size_t prefixes_len, prefixes_cap;
    Str* prefixes;
} FileManager;

typedef struct {
    TokenArr res;

    LineInfo line_info;

    FileManager file_manager;

    size_t conds_len, conds_cap;
    PreprocCond* conds;

    PreprocErr* err;
    StringMap _macro_map;
    FileInfo file_info;
} PreprocState;

PreprocState PreprocState_create(const char* start_file, PreprocErr* err);

PreprocState PreprocState_create_string(const char* code, const char* filename, PreprocErr* err);

void PreprocState_read_line(PreprocState* state);
bool PreprocState_over(const PreprocState* state);

typedef struct PreprocMacro PreprocMacro;

const PreprocMacro* find_preproc_macro(const PreprocState* state, const Str* spelling);

bool PreprocState_open_file(PreprocState* s, const Str* filename_str, SourceLoc include_loc);

void register_preproc_macro(PreprocState* state, const Str* spelling, const PreprocMacro* macro);

void remove_preproc_macro(PreprocState* state, const Str* spelling);

void push_preproc_cond(PreprocState* state, SourceLoc loc, bool was_true);

void pop_preproc_cond(PreprocState* state);

PreprocCond* peek_preproc_cond(PreprocState* state);

void TokenArr_free(TokenArr* arr);

void PreprocState_free(PreprocState* state);

#include "PreprocMacro.h"

#endif

