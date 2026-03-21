//
// Created by Harel on 3/21/26.
//

#include <errno.h>
#include "../allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *a[80];

    for (int i = 1; i <= 50; i++) {
        a[i] = my_malloc(sizeof(char) * i); // First malloc
        if (a[i] == NULL) {
            printf("Error. %s\n.", strerror(errno));
            return -1;
        }
    }
    for (int i = 50; i > 0; i--) {
        my_free(a[i]);
    }

    for (int i = 50; i > 0; i--) {
        a[i] = my_malloc(sizeof(char) * i); // Second malloc
        if (a[i] == NULL) {
            printf("Error. %s\n.", strerror(errno));
            return -1;
        }
    }
    for (int i = 1; i <= 50; i++) {
        my_free(a[i]);
    }

    return 0;
}