//
// Created by Harel on 3/19/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_ALLOCATOR_H
#define MY_SECURE_FILE_TRANSFER_ALLOCATOR_H

#define _GNU_SOURCE

#include <sanitizer/asan_interface.h>
#include <stdint.h> // IWYU pragma: export
#include <sys/auxv.h>
#include <sys/mman.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

static struct metadata {
    unsigned long long size;
    char is_free;
    struct metadata *next_node;
} metadata;

static struct list {
    void *addr;
    struct list *next_node;
} list;

void *my_malloc(size_t size);
void my_free(void *ptr);

#endif // MY_SECURE_FILE_TRANSFER_ALLOCATOR_H
