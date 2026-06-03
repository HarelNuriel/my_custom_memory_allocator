//
// TODO(#1): Handle Double Free.
// Created by Harel on 3/19/26.
//

#include "allocator.h"

static void *map;
static void *free_map;
static int page_size;
static unsigned long long f_page_size;
static unsigned long long m_page_size;
static struct metadata *first_node;
static struct double_list *free_list;

void release_map() __attribute__((destructor));
void defragmentation() __attribute__((no_sanitize_address));

void release_map() {
    __asan_unpoison_memory_region(map, m_page_size);
    munmap(map, m_page_size);
}

int is_mem_init() { return map == NULL ? 0 : 1; }

int init_mem() {
    page_size = getauxval(AT_PAGESZ);
    m_page_size = page_size;
    f_page_size = page_size;

    map = mmap(NULL, m_page_size, PROT_READ | PROT_WRITE,
               MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    free_map = mmap(NULL, f_page_size, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (map == MAP_FAILED || free_map == MAP_FAILED) {
        return 0;
    }

    first_node = map;
    first_node->size = m_page_size - sizeof(struct metadata);
    first_node->is_free = 1;
    first_node->next_node = NULL;

    free_list = free_map;
    free_list->address = NULL;
    free_list->next = NULL;
    free_list->prev = NULL;

    __asan_poison_memory_region(first_node + 1,
                                m_page_size - sizeof(struct metadata));
    return 1;
}

void defragmentation() {
    struct metadata *node = first_node;

    while (node->next_node != NULL) {
        const struct metadata *temp = node->next_node;
        if (node->is_free && temp->is_free) {
            node->size += temp->size + sizeof(struct metadata);
            node->next_node = temp->next_node;
        } else {
            node = node->next_node;
        }
    }
}

int resize_mem(void *map_ptr, unsigned long long *page_len) {
    void *new_map = mremap(map_ptr, m_page_size, m_page_size + page_size, 0);

    if (new_map == MAP_FAILED) {
        return 0;
    }

    (*page_len) += page_size;
    return 1;
}

struct metadata *find_first_free_node(const unsigned long long size) {
    struct metadata *node = first_node;
    int is_fragmented = 1;
    unsigned long long page = m_page_size;

    while (!node->is_free || node->size < size) {
        if (page == 0) {
            resize_mem(map, &m_page_size);
            // Figure out how to deal with this case after implementing
            // resize_mem
            continue;
        }
        if (page <= node->size + sizeof(struct metadata)) {
            if (is_fragmented) {
                defragmentation();
                is_fragmented = 0;
            } else {
                resize_mem(map, &m_page_size);
                // Figure out how to deal with this case after implementing
                // resize_mem
            }
            page = m_page_size;
            node = first_node;
            continue;
        }
        if (node->next_node == NULL) {
            struct metadata *next = (void *)(node + 1) + node->size;
            next->is_free = 1;
            next->size = m_page_size - (page + sizeof(struct metadata));
        }
        page -= sizeof(struct metadata) + node->size;
        node = node->next_node;
    }

    return node;
}

void was_addr_freed(void *addr) {
    struct double_list *temp = free_list;
    while (temp != NULL) {
        if (temp->address == addr) {
            temp->address = NULL;
            return;
        }
        temp = temp->next;
    }
}

void *my_malloc(size_t size) {
    if (size == 0)
        return NULL;
    if (!is_mem_init()) {
        if (!init_mem()) {
            return NULL;
        }
    }

    size = size % 16 == 0 ? size : size + (16 - size % 16);
    struct metadata *mdata = find_first_free_node(size);
    was_addr_freed(mdata + 1);
    __asan_unpoison_memory_region(mdata + 1, mdata->size);

    struct metadata *next_node = (void *)(mdata + 1) + size;
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

void fill_free_fields(struct double_list *node, void *addr,
                      struct double_list *prev) {
    node->address = addr;
    node->prev = prev;

    struct metadata *mdata = (struct metadata *)addr - 1;
    node->size = mdata->size;
}

int is_free(void *ptr) {
    int i = 1;
    struct double_list *temp = free_list;

    if (temp == NULL) {
        fill_free_fields(temp, ptr, NULL);
    }

    while (temp != NULL) {
        if (temp->address == ptr) {
            return 1;
        }
        if (temp->next == NULL) {
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL) {
        return 1;
    }

    struct double_list *new = temp + 1;
    fill_free_fields(new, ptr, temp);
    temp->next = new;

    return 0;
}

void my_free(void *ptr) {
    if (is_free(ptr)) {
        __builtin_trap();
    }

    struct metadata *mdata = (struct metadata *)ptr - 1;
    mdata->is_free = 1;
    __asan_poison_memory_region(mdata + 1, mdata->size);
}
