#include "../allocator.h"

int main() {
    char *test1, *test2;
    for (int i = 1; i < 32; i++) {
        test1 = my_malloc(i * 16);
        test2 = my_malloc(i * 16);
        my_free(test1);
        my_free(test2);
    }
    for (int i = 1; i < 64; i++) {
        test1 = my_malloc(i * 16);
        test2 = my_malloc(i * 16);
        my_free(test1);
        my_free(test2);
    }

    return 0;
}
