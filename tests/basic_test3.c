#include "../allocator.h"

int main() {
    int *test;
    for (int i = 0; i < 100; i++) {
        test = my_malloc(10 * sizeof(int));
        my_free(test);
    }

    return 0;
}
