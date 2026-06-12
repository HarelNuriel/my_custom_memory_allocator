#include "../allocator.h"

int main() {

    char *test = my_malloc(200000);
    my_free(test);

    return 0;
}
