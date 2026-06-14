//
// Created by kali on 3/19/26.
//

#include <errno.h>
#include "../allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *test = my_malloc(sizeof(*test) * 4000);
    if (test == NULL) {
        printf("Failed. %s.\n", strerror(errno));
        return 1;
    }
    test[7] = '\0';

    for (int i = 0; i < 7; i++) {
        test[i] = 'a';
    }

    printf("%s\n", test);
    my_free(test);

    test = my_malloc(sizeof(*test) * 4001);
    if (test == NULL) {
        printf("Failed. %s.\n", strerror(errno));
        return 1;
    }
    test[7] = '\0';

    for (int i = 0; i < 7; i++) {
        test[i] = 'a';
    }

    printf("%s\n", test);
    my_free(test);

    return 0;
}
