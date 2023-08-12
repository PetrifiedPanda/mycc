#ifndef MYCC_FRONTEND_PREPROC_PREPROC_STATE_H
#define MYCC_FRONTEND_PREPROC_PREPROC_STATE_H

#include <stddef.h>

#include "util/StringMap.h"
#include "util/File.h"

#include "frontend/FileInfo.h"

#include "PreprocErr.h"

typedef struct {
    StrBuf line;
    Str next;
    SourceLoc curr_loc;
    bool is_in_comment;
} LineInfo;

typedef struct {
    bool had_true_branch;
    bool had_else;
    SourceLoc loc;
} PreprocCond;

typedef struct OpenedFileInfo OpenedFileInfo;

typedef struct {
    File files[FOPEN_MAX];
    uint32_t opened_info_indices[FOPEN_MAX];
    uint32_t current_file_idx;
    uint32_t opened_info_len, opened_info_cap;
    OpenedFileInfo* opened_info;
    uint32_t prefixes_len, prefixes_cap;
    StrBuf* prefixes;
} FileManager;

typedef struct {
    TokenArr res;

    LineInfo line_info;

    FileManager file_manager;

    uint32_t conds_len, conds_cap;
    PreprocCond* conds;

    StringMap _macro_map;
    FileInfo file_info;
    PreprocErr* err;
} PreprocState;

PreprocState PreprocState_create(CStr start_file, PreprocErr* err);

PreprocState PreprocState_create_string(Str code,
                                        Str filename,
                                        PreprocErr* err);

void PreprocState_read_line(PreprocState* state);
bool PreprocState_over(const PreprocState* state);

typedef struct PreprocMacro PreprocMacro;

const PreprocMacro* find_preproc_macro(const PreprocState* state,
                                       const StrBuf* spelling);

bool PreprocState_open_file(PreprocState* s,
                            const StrBuf* filename_str,
                            const SourceLoc* include_loc);

void PreprocState_register_macro(PreprocState* state,
                                 const StrBuf* spelling,
                                 const PreprocMacro* macro);

void PreprocState_remove_macro(PreprocState* state, const StrBuf* spelling);

void PreprocState_push_cond(PreprocState* state, SourceLoc loc, bool was_true);

void PreprocState_pop_cond(PreprocState* state);

PreprocCond* peek_preproc_cond(PreprocState* state);

void TokenArr_free_preproc(TokenArr* arr);

void PreprocState_free(PreprocState* state);

#endif

