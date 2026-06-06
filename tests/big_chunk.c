#include "../allocator.h"

int main() {

    char *test = my_malloc(10000);
    my_free(test);

    return 0;
}
