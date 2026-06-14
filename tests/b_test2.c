//
// Created by Harel on 3/21/26.
//

#include <errno.h>
#include "../allocator.h"
#include <string.h>

int main() {
    char *a = my_malloc(0);
    if (a == NULL) {
        return 0;
    }

    return -1;
}