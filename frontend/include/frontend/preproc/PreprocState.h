#ifndef MYCC_FRONTEND_PREPROC_PREPROC_STATE_H
#define MYCC_FRONTEND_PREPROC_PREPROC_STATE_H

#include <stddef.h>

#include "util/File.h"

#include "frontend/FileInfo.h"

#include "PreprocTokenArr.h"
#include "PreprocErr.h"

typedef struct LineInfo {
    StrBuf line;
    Str next;
    SourceLoc curr_loc;
    bool is_in_comment;
} LineInfo;

typedef struct PreprocCond {
    bool had_true_branch;
    bool had_else;
    SourceLoc loc;
} PreprocCond;

typedef struct OpenedFileInfo OpenedFileInfo;

typedef struct FileManager {
    File files[FOPEN_MAX];
    uint32_t opened_info_indices[FOPEN_MAX];
    uint32_t current_file_idx;
    uint32_t opened_info_len, opened_info_cap;
    OpenedFileInfo* opened_info;
    uint32_t prefixes_len, prefixes_cap;
    StrBuf* prefixes;
} FileManager;

typedef struct PreprocMacro PreprocMacro;

typedef struct PreprocMacroMap {
    uint32_t _cap;
    PreprocMacro* _macros;
} PreprocMacroMap;

typedef struct PreprocState {
    PreprocTokenArr toks;
    PreprocTokenValList vals;

    LineInfo line_info;

    FileManager file_manager;

    uint32_t conds_len, conds_cap;
    PreprocCond* conds;

    PreprocMacroMap _macro_map;
    FileInfo file_info;
    uint32_t num_include_dirs;
    const Str* include_dirs;
    PreprocErr* err;
} PreprocState;

PreprocState PreprocState_create(CStr start_file,
                                 uint32_t num_include_dirs,
                                 const Str* include_dirs,
                                 PreprocErr* err);

PreprocState PreprocState_create_string(Str code,
                                        Str filename,
                                        uint32_t num_include_dirs,
                                        const Str* include_dirs,
                                        PreprocErr* err);

void PreprocState_read_line(PreprocState* state);
bool PreprocState_over(const PreprocState* state);

typedef struct PreprocMacro PreprocMacro;

const PreprocMacro* find_preproc_macro(const PreprocState* state,
                                       uint32_t identifier_idx);

bool PreprocState_open_file(PreprocState* s,
                            const StrBuf* filename_str,
                            const SourceLoc* include_loc);

void PreprocState_register_macro(PreprocState* state,
                                 uint32_t identifier_idx,
                                 const PreprocMacro* macro);

void PreprocState_remove_macro(PreprocState* state, uint32_t identifier_idx);

void PreprocState_push_cond(PreprocState* state, SourceLoc loc, bool was_true);

void PreprocState_pop_cond(PreprocState* state);

PreprocCond* peek_preproc_cond(PreprocState* state);

void TokenArr_free_preproc(TokenArr* arr);

void PreprocState_free(PreprocState* state);

#endif

