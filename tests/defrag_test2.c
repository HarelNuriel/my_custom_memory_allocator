//
// Created by kali on 3/20/26.
//

#include <errno.h>
#include "../allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *a1 = my_malloc(sizeof(char) * 500);
    if (a1 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a2 = my_malloc(sizeof(char) * 500);
    if (a2 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a3 = my_malloc(sizeof(char) * 500);
    if (a3 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a4 = my_malloc(sizeof(char) * 500);
    if (a4 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a5 = my_malloc(sizeof(char) * 500);
    if (a5 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a6 = my_malloc(sizeof(char) * 500);
    if (a6 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a7 = my_malloc(sizeof(char) * 500);
    if (a7 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }

    my_free(a1);
    my_free(a2);
    my_free(a3);
    my_free(a4);
    my_free(a5);
    my_free(a6);
    my_free(a7);

    char *test = my_malloc(sizeof(*test) * 4000);
    if (test == NULL) {
        printf("Failed. %s.\n", strerror(errno));
        return 1;
    }

    for (int i = 0; i < 4000; i++) {
        test[i] = 'a';
    }

    my_free(test);

    return 0;
}