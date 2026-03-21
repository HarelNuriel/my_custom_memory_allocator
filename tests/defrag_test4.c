//
// Created by Harel on 3/21/26.
//

#include <errno.h>
#include "../allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *a[5];

    for (int i = 0; i < 5; i++) {
        a[i] = my_malloc(sizeof(char) * 200); // First malloc
        if (a[i] == NULL) {
            printf("Error. %s\n.", strerror(errno));
            return -1;
        }
    }
    for (int i = 0; i < 5; i += 2) {
        my_free(a[i]);
    }
    for (int i = 1; i < 5; i += 2) {
        my_free(a[i]);
    }

    char *b = my_malloc(sizeof(char) * 4000);
    my_free(b);

    return 0;
}