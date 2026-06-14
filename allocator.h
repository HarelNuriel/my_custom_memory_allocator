//
// Created by Harel on 3/19/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_ALLOCATOR_H
#define MY_SECURE_FILE_TRANSFER_ALLOCATOR_H

#define _GNU_SOURCE

#include "arena.h"

#include <elf.h>
#include <stdint.h> // IWYU pragma: export
#include <sys/auxv.h>
#include <sys/mman.h>

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>

#define poison(addr, size) __asan_poison_memory_region(addr, size)
#define unpoison(addr, size) __asan_unpoison_memory_region(addr, size)

#else
#define __asan_poison_memory_region(addr, size) ((void)0)
#define __asan_unpoison_memory_region(addr, size) ((void)0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define get_size(ptr) ((ptr)->size & CHUNK_SIZE_MASK)

#define MMAP(addr, size, prot, flags)                                          \
    mmap((addr), (size), (prot), (flags) | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)

void *my_malloc(size_t size);
void my_free(void *ptr);

#endif // MY_SECURE_FILE_TRANSFER_ALLOCATOR_H
