//
// Created by kali on 3/21/26.
//

#include "../allocator.h"
#include <stdio.h>

int main() {
    char *test = my_malloc(sizeof(*test) * 4000);
    if (test == NULL) {
        return -1;
    }

    my_free(test);
    my_free(test);

    printf("This Shouldn't show.");

    return 0;
}
