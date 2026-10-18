//
// Created by Harel on 3/19/26.
//

#include "allocator.h"
#include "arena.h"
#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/auxv.h>
#include <sys/mman.h>

struct malloc_state marena;

void release_arena() __attribute__((destructor));
void defragmentation() __attribute__((no_sanitize_address));

void release_arena() {
    unpoison(marena.heap, marena.heap->size);
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
    chunk->size = (heap->size - sizeof(heap_info)) & CHUNK_SIZE_MASK;
    chunk->prev_size = 0;

    arena->heap = heap;
    arena->top = chunk;

    for (int i = 0; i < 128; i++) {
        arena->bins[i].fd = &arena->bins[i];
        arena->bins[i].bk = &arena->bins[i];
    }

    int to_poison = HEAP_SIZE - (sizeof(heap_info) + CHUNK_STRUCT_SIZE);
    poison(chunk->user_data, to_poison);
    return 1;
}

void insert_in_sbin(mchunkptr *chunk) {}

void *check_uns_bin(size_t size) {
    mchunkptr *uns_list = &marena.bins[UNSORTED_BIN];
    unpoison(uns_list->fd, CHUNK_STRUCT_SIZE * 2);
    mchunkptr *head = uns_list;

    while (uns_list->size < size && uns_list != head) {
        unpoison(uns_list->fd, CHUNK_STRUCT_SIZE * 2);
        uns_list = uns_list->fd;
        poison(uns_list->bk, CHUNK_STRUCT_SIZE * 2);
    }
    if (uns_list == head) {
        return NULL;
    }

    unpoison(uns_list, uns_list->size);
    unpoison(uns_list->bk, CHUNK_STRUCT_SIZE * 2);
    mchunkptr *carved = (void *)uns_list + uns_list->size;
    carved->size = uns_list->size - size;
    carved->prev_size = size + PREV_INUSE;
    carved->bk = uns_list->bk;
    carved->fd = uns_list->fd;
    uns_list->bk->fd = carved;
    poison(uns_list, CHUNK_STRUCT_SIZE * 2);

    return carved;
}

/*
 * check for chunks in the bins. dep on size, check fast then small/large then
 * unsorted
 */
mchunkptr *check_bins(size_t size) {
    // TODO: First should check fast bin.

    // NOTE:If not in small bin, check unsorted bin.
    uint32_t idx = size / 16 >= 128 ? 0 : size / 16;
    if (idx == 0)
        return check_uns_bin(size);

    // NOTE:If small bin is empty, check unsorted bin.
    mchunkptr *head = &marena.bins[idx];
    if (head->fd == head)
        return check_uns_bin(size);

    // NOTE:return the first chunk in bin and move head to the next.
    unpoison(head->fd, CHUNK_STRUCT_SIZE * 2);
    unpoison(head->fd->fd, CHUNK_STRUCT_SIZE * 2);
    mchunkptr *ret = head->fd;
    head->fd = head->fd->fd;
    head->fd->bk = head;
    if (head->fd != head)
        poison(head->fd, CHUNK_STRUCT_SIZE * 2);

    unpoison((void *)ret + get_size(ret), CHUNK_STRUCT_SIZE);
    unpoison(ret, get_size(ret));
    ((mchunkptr *)((void *)ret + get_size(ret)))->size |= PREV_INUSE;
    poison((void *)ret + get_size(ret), CHUNK_STRUCT_SIZE);
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
    mchunkptr *mdata = check_bins(size);
    if (mdata != NULL) {
        poison(mdata, CHUNK_STRUCT_SIZE);
        return mdata->user_data;
    }

    if (size > marena.top->size)
        return NULL;

    // NOTE:After chunk not found in free lists, get a chunk from the heap
    unpoison(marena.top, marena.heap->size);
    mdata = marena.top;
    marena.top = (void *)marena.top + (size + CHUNK_STRUCT_SIZE);
    marena.top->prev_size = size + CHUNK_STRUCT_SIZE;
    marena.heap->size -= (size + CHUNK_STRUCT_SIZE);
    marena.top->size = marena.heap->size | PREV_INUSE;

    // NOTE: every free merges, so top->prev always inuse.
    mdata->size = (size + CHUNK_STRUCT_SIZE) | PREV_INUSE;

    poison(mdata, CHUNK_STRUCT_SIZE);
    poison(marena.top, marena.heap->size);
    return mdata->user_data;
}

mchunkptr *merge_chunk(mchunkptr *chunk) {
    mchunkptr *prev = (void *)chunk - chunk->prev_size;
    mchunkptr *next = (void *)chunk + (chunk->size & CHUNK_SIZE_MASK);

    unpoison(prev, CHUNK_STRUCT_SIZE);
    unpoison(next, CHUNK_STRUCT_SIZE);

    // NOTE:Check next/prev chunks for PREV_INUSE flag and merge if 0
    if (!(chunk->size & PREV_INUSE) && chunk->prev_size > 0) {
        prev->size += (chunk->size & CHUNK_SIZE_MASK);
        prev_chunk(chunk);
    } else {
        poison(prev, CHUNK_STRUCT_SIZE);
    }

    // NOTE:Check if next is top chunk and move top chunk if true
    if ((void *)next == (void *)marena.top) {
        chunk->size += marena.top->size & CHUNK_SIZE_MASK;
        marena.top = chunk;
    } else {
        // NOTE:Check if next is free, and merge if it is.
        mchunkptr *n_next = (void *)next + (next->size & CHUNK_SIZE_MASK);
        unpoison(n_next, CHUNK_STRUCT_SIZE);
        if (n_next->size & PREV_INUSE) {
            next->size &= ~PREV_INUSE;
        } else {
            chunk->size += next->size;
        }
        poison(n_next, CHUNK_STRUCT_SIZE);
    }
    poison(next, CHUNK_STRUCT_SIZE);

    poison((void *)chunk + (CHUNK_STRUCT_SIZE * 2), chunk->size);

    return chunk;
}

void insert_chunk(mchunkptr *list, mchunkptr *chunk) {
    mchunkptr *uchunk = merge_chunk(chunk);
    if (marena.top == uchunk) {
        return;
    }

    mchunkptr *last = list->bk;
    unpoison(&last->fd, CHUNK_STRUCT_SIZE * 2);

    list->bk = chunk;
    last->fd = chunk;
    chunk->fd = list;
    chunk->bk = last;
}

void my_free(void *ptr) {
    if (ptr == NULL)
        return;
    if (ptr - CHUNK_STRUCT_SIZE == marena.top) {
        __builtin_trap();
    }

    // TODO: merge adjacent chunks in unsorted list.
    unpoison(ptr - CHUNK_STRUCT_SIZE, CHUNK_STRUCT_SIZE);
    mchunkptr *mdata = ptr - CHUNK_STRUCT_SIZE;
    unpoison(mdata, get_size(mdata));
    unpoison((void *)mdata + get_size(mdata), CHUNK_STRUCT_SIZE);
    mchunkptr *next = (void *)mdata + get_size(mdata);

    if (!(next->size & PREV_INUSE)) {
        __builtin_trap();
    }

    // NOTE:insert chunk into unsorted bin.
    unpoison(mdata->user_data, CHUNK_STRUCT_SIZE);
    insert_chunk(&marena.bins[UNSORTED_BIN], mdata);

    poison(next, CHUNK_STRUCT_SIZE);
    poison((void *)mdata + CHUNK_STRUCT_SIZE, mdata->size - CHUNK_STRUCT_SIZE);
}
