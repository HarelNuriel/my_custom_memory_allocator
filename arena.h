#pragma once

#include <stddef.h>
#include <stdint.h>

#define CHUNK_SIZE_MASK ~(0x07)
#define CHUNK_STRUCT_SIZE offsetof(struct malloc_chunk, fd)
#define HEAP_SIZE 128 * 1024 // needs to be a power of 2

typedef struct malloc_chunk mchunkptr;
typedef struct malloc_state mstate;
typedef struct _heap_info heap_info;

#define prev_chunk(ptr)                                                        \
    ((ptr) = (mchunkptr *)((char *)(ptr) - ((ptr)->size & CHUNK_SIZE_MASK)))

#define next_chunk(ptr)                                                        \
    ((ptr) = (mchunkptr *)((char *)(ptr) + ((ptr)->size & CHUNK_SIZE_MASK)))

struct _heap_info {
    size_t size;
    mstate *ar;
    heap_info *heap;
};

struct malloc_chunk {
    size_t size;
    size_t prev_size;

    // if in free list used as double link list. else user data fills this
    // space.
    union {
        struct {
            mchunkptr *fd;
            mchunkptr *bk;
        };
        char user_data[0];
    };
};

struct malloc_state {
    heap_info *heap;
    mstate *next;
    struct mutex_t *m;
    int flags;

    mchunkptr bins[128 * 2 - 2];
    mchunkptr *top;
};

enum chunk_bits { PREV_INUSE = 0x1, IS_MAPPED = 0x2, NON_MAIN_ARENA = 0x4 };

///////////////////////////////////////////////////////////////////////////////////////////////

extern mstate marena;
