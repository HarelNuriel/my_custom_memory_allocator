//
// Created by Harel on 3/19/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_ALLOCATOR_H
#define MY_SECURE_FILE_TRANSFER_ALLOCATOR_H

#include <sys/mman.h>
#include <sys/auxv.h>
#include <sanitizer/asan_interface.h>

#define NULL (void*)0

struct metadata{
    unsigned long long size;
    char is_free;
    struct metadata *next_node;
};

void* my_malloc(size_t size);
void my_free(void* ptr);

#endif //MY_SECURE_FILE_TRANSFER_ALLOCATOR_H