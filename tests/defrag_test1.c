//
// Created by kali on 3/20/26.
//

#include <errno.h>
#include "../allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *a1 = my_malloc(sizeof(char) * 400);
    if (a1 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a2 = my_malloc(sizeof(char) * 400);
    if (a2 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a3 = my_malloc(sizeof(char) * 400);
    if (a3 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a4 = my_malloc(sizeof(char) * 400);
    if (a4 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a5 = my_malloc(sizeof(char) * 400);
    if (a5 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a6 = my_malloc(sizeof(char) * 400);
    if (a6 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *a7 = my_malloc(sizeof(char) * 400);
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

    char *b1 = my_malloc(sizeof(char) * 400);
    if (b1 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b2 = my_malloc(sizeof(char) * 400);
    if (b2 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b3 = my_malloc(sizeof(char) * 400);
    if (b3 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b4 = my_malloc(sizeof(char) * 400);
    if (b4 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b5 = my_malloc(sizeof(char) * 400);
    if (b5 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b6 = my_malloc(sizeof(char) * 400);
    if (b6 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }
    char *b7 = my_malloc(sizeof(char) * 400);
    if (b7 == NULL) {
        printf("a1 error. %s\n.", strerror(errno));
        return -1;
    }

    my_free(b1);
    my_free(b2);
    my_free(b3);
    my_free(b4);
    my_free(b5);
    my_free(b6);
    my_free(b7);

    return 0;
}