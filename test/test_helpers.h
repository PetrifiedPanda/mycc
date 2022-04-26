#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

struct token* tokenize(const char* file);
struct token* tokenize_string(const char* str, const char* file);

#endif

