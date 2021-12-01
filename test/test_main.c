#include <stdio.h>

extern void error_test();

extern void tokenizer_test();

int main() {
    error_test();
    tokenizer_test();
    printf("All tests successful\n");
}
