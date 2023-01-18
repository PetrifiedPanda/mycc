#ifndef CODE_SOURCE_H
#define CODE_SOURCE_H

#include <stdbool.h>
#include <stdio.h>

struct code_source {
#ifdef MYCC_TEST_FUNCTIONALITY
    bool _is_str;
    union {
        const char* _str;
#else
    struct {
#endif
        FILE* _file;
    };
    const char* path;
    size_t current_line;
    bool comment_not_terminated;
};

#ifdef MYCC_TEST_FUNCTIONALITY
struct code_source create_code_source_string(const char* str, const char* path);
#endif

struct code_source create_code_source_file(FILE* file, const char* path);

bool code_source_over(struct code_source* src);

char* code_source_read_line(struct code_source* src, size_t static_buf_len, char* static_buf);

#endif

