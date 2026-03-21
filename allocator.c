//
// Created by Harel on 3/19/26.
//

#include "allocator.h"

static void* map;
static uint64_t page_size;
static struct metadata* first_node;

void release_map() __attribute__((destructor));
void defragmentation() __attribute__((no_sanitize_address));

void release_map() {
    __asan_unpoison_memory_region(map, page_size);
    munmap(map, page_size);
}

int is_mem_init() {
    return map == NULL ? 0 : 1;
}

int init_mem() {
    page_size = getauxval(AT_PAGESZ);
    map = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (map == MAP_FAILED) {
        return 0;
    }

    first_node = map;
    first_node->size = page_size - sizeof(struct metadata);
    first_node->is_free = 1;
    first_node->next_node = NULL;

    __asan_poison_memory_region(first_node + 1, page_size - sizeof(struct metadata));
    return 1;
}

void defragmentation() {
    struct metadata *node = first_node;

    while (node->next_node != NULL) {
        const struct metadata* temp = node->next_node;
        if (node->is_free && temp->is_free) {
            node->size += temp->size + sizeof(struct metadata);
            node->next_node = temp->next_node;
        } else {
            node = node->next_node;
        }
    }
}

void resize_mem() {

}

struct metadata* find_first_free_node(const uint64_t size) {
    struct metadata *node = first_node;
    int is_fragmented = 1;
    uint64_t page = page_size;

    while (!node->is_free || node->size < size) {
        if (page == 0) {
            resize_mem();
            // Figure out how to deal with this case after implementing resize_mem
            continue;
        }
        if (page <= node->size + sizeof(struct metadata)) {
            if (is_fragmented) {
                defragmentation();
                is_fragmented = 0;
            } else {
                resize_mem();
                // Figure out how to deal with this case after implementing resize_mem
            }
            page = page_size;
            node = first_node;
            continue;
        }
        if (node->next_node == NULL) {
            struct metadata *next = (void*)(node + 1) + node->size;
            next->is_free = 1;
            next->size = page_size - (page + sizeof(struct metadata));
        }
        page -= sizeof(struct metadata) + node->size;
        node = node->next_node;
    }

    return node;
}

void* my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (!is_mem_init()) {
        if (!init_mem()) {
            return NULL;
        }
    }

    size = size % 16 == 0 ? size : size + size % 16;
    struct metadata *mdata = find_first_free_node(size);
    __asan_unpoison_memory_region(mdata + 1, mdata->size);

    struct metadata *next_node = (void*)(mdata + 1) + size;
    if (mdata->size > size + sizeof(struct metadata)) {
        next_node->size = mdata->size - size - sizeof(struct metadata);
        next_node->is_free = 1;
    }
    mdata->is_free = 0;
    mdata->size = size;
    mdata->next_node = next_node;

    __asan_poison_memory_region(next_node + 1, next_node->size);
    return mdata + 1;
}

void my_free(void *ptr) {
    struct metadata *mdata = (struct metadata*)ptr - 1;
    mdata->is_free = 1;
    __asan_poison_memory_region(mdata + 1, mdata->size);
}