#include <stdio.h>

extern void error_test();

extern void tokenizer_test();

extern void parser_test();

int main() {
    error_test();
    tokenizer_test();
    parser_test();
    printf("All tests successful\n");
}
