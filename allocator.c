//
// TODO(#1): Handle Double Free.
// Created by Harel on 3/19/26.
//

#include "allocator.h"
#include "arena.h"
#include <stddef.h>
#include <stdint.h>

struct malloc_state marena;

void release_arena() __attribute__((destructor));
void defragmentation() __attribute__((no_sanitize_address));

void release_arena() {
    __asan_unpoison_memory_region(marena.heap, marena.heap->size);
    munmap(marena.heap, marena.heap->size);
}

static int init_arena(mstate *arena) {
    void *ptr = MMAP(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, 0);
    if (ptr == MAP_FAILED) {
        return 0;
    }

    heap_info *heap = ptr;
    heap->size = HEAP_SIZE - sizeof(heap_info);
    heap->ar = arena;

    mchunkptr *chunk = (void *)heap + sizeof(heap_info);
    chunk->size = heap->size & CHUNK_SIZE_MASK;

    arena->heap = heap;
    arena->top = chunk;

    for (int i = 0; i < 128; i++) {
        arena->bins[i].fd = &arena->bins[i];
        arena->bins[i].bk = &arena->bins[i];
    }

    __asan_poison_memory_region(chunk->user_data, HEAP_SIZE);
    return 1;
}

void defragmentation() {}

int resize_mem(void *map_ptr, unsigned long long *page_len) {}

void *check_uns_bin(size_t size) { return NULL; }

mchunkptr *check_free_list(size_t size) {
    uint32_t idx = size / 16 >= 128 ? 0 : size / 16;
    if (idx == 0)
        return check_uns_bin(size);

    mchunkptr *head = &marena.bins[idx];
    if (head->fd == head)
        return NULL;

    __asan_unpoison_memory_region(head->fd, CHUNK_STRUCT_SIZE * 2);
    __asan_unpoison_memory_region(head->fd->fd, CHUNK_STRUCT_SIZE * 2);
    mchunkptr *ret = head->fd;
    head->fd = head->fd->fd;
    head->fd->bk = head;
    if (head->fd != head)
        __asan_poison_memory_region(head->fd, CHUNK_STRUCT_SIZE * 2);

    __asan_unpoison_memory_region((void *)ret + get_size(ret),
                                  CHUNK_STRUCT_SIZE);
    __asan_unpoison_memory_region(ret, get_size(ret));
    ((mchunkptr *)((void *)ret + get_size(ret)))->size |= PREV_INUSE;
    __asan_poison_memory_region((void *)ret + get_size(ret), CHUNK_STRUCT_SIZE);
    return ret;
}

void *my_malloc(size_t size) {
    if (marena.heap == NULL) {
        if (!init_arena(&marena))
            return NULL;
    }
    if (size == 0)
        return NULL;

    size = size + 15 & ~15;
    // TODO: First check free lists for matching chunk.
    mchunkptr *mdata = check_free_list(size);
    if (mdata != NULL) {
        return mdata->user_data;
    }

    // After chunk not found in free lists, get a chunk from the heap
    __asan_unpoison_memory_region(marena.top, marena.heap->size);
    mdata = marena.top;
    marena.top = (void *)marena.top + (size + CHUNK_STRUCT_SIZE);
    marena.top->prev_size = size + CHUNK_STRUCT_SIZE;
    marena.heap->size -= (size + CHUNK_STRUCT_SIZE);
    marena.top->size = marena.heap->size | PREV_INUSE;

    mdata->size = (size + CHUNK_STRUCT_SIZE) | PREV_INUSE;

    __asan_poison_memory_region(mdata, CHUNK_STRUCT_SIZE);
    __asan_poison_memory_region(marena.top, marena.heap->size);
    return mdata->user_data;
}

void insert_chunk(mchunkptr *list, mchunkptr *chunk) {
    mchunkptr *last = list->bk;
    __asan_unpoison_memory_region(&last->fd, CHUNK_STRUCT_SIZE * 2);

    list->bk = chunk;
    last->fd = chunk;
    chunk->fd = list;
    chunk->bk = last;
}

void my_free(void *ptr) {
    if (ptr == NULL)
        return;

    __asan_unpoison_memory_region(ptr - CHUNK_STRUCT_SIZE, CHUNK_STRUCT_SIZE);
    mchunkptr *mdata = ptr - CHUNK_STRUCT_SIZE;
    __asan_unpoison_memory_region(mdata, get_size(mdata));
    __asan_unpoison_memory_region((void *)mdata + get_size(mdata),
                                  CHUNK_STRUCT_SIZE);
    mchunkptr *next = (void *)mdata + get_size(mdata);

    if (!(next->size & PREV_INUSE)) {
        __builtin_trap();
    }

    next->size -= PREV_INUSE;

    int idx = ((mdata->size & CHUNK_SIZE_MASK) / 16) - 1;
    if (idx < 0 || idx >= 128)
        idx = 0;
    __asan_unpoison_memory_region(mdata->user_data, CHUNK_STRUCT_SIZE);
    insert_chunk(&marena.bins[idx], mdata);

    __asan_poison_memory_region(next, CHUNK_STRUCT_SIZE);
    __asan_poison_memory_region((void *)mdata + CHUNK_STRUCT_SIZE,
                                mdata->size - CHUNK_STRUCT_SIZE);
}
