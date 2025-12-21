#define _GNU_SOURCE
#include "memory.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static void *heap_start_addr = NULL;
static size_t heap_total_size = 0;

#define BLOCK_HEADER_SIZE sizeof(block_header_t)
#define DEFAULT_HEAP_SIZE (640) // 640 Byte For Visual Test
// #define DEFAULT_HEAP_SIZE (65536) //64KB For Benchmark

void heap_init(void *start_addr, size_t size)
{
    if (!start_addr || size < BLOCK_HEADER_SIZE)
    {
        return;
    }

    heap_start_addr = start_addr;
    heap_total_size = size;

    block_header_t *first_block = (block_header_t *)start_addr;
    first_block->size = size - BLOCK_HEADER_SIZE;
    first_block->is_free = 1;
    first_block->next = NULL;
    first_block->prev = NULL;
}

int my_memory_init(size_t size)
{
    void *mem = sbrk(size);
    if (mem == (void *)-1)
    {
        return -1;
    }

    heap_init(mem, size);
    return 0;
}

void my_memory_reset(void)
{
    heap_start_addr = NULL;
    heap_total_size = 0;
}

static block_header_t *find_free_block(size_t size, alloc_algo_t algo)
{
    block_header_t *current = (block_header_t *)heap_start_addr;
    block_header_t *best_block = NULL;

    while (current != NULL)
    {
        if (current->is_free && current->size >= size)
        {
            if (algo == ALGO_FIRST_FIT)
            {
                return current;
            }
            else if (algo == ALGO_BEST_FIT)
            {
                if (best_block == NULL || current->size < best_block->size)
                {
                    best_block = current;
                }
            }
            else if (algo == ALGO_WORST_FIT)
            {
                if (best_block == NULL || current->size > best_block->size)
                {
                    best_block = current;
                }
            }
        }
        current = current->next;
    }
    return best_block;
}

static void split_block(block_header_t *block, size_t size)
{
    if (block->size > size + BLOCK_HEADER_SIZE)
    {
        block_header_t *new_block = (block_header_t *)((char *)block + BLOCK_HEADER_SIZE + size);

        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;

        if (block->next != NULL)
        {
            block->next->prev = new_block;
        }

        block->next = new_block;
        block->size = size;
    }
}

static void coalesce(block_header_t *block)
{
    if (block->next && block->next->is_free)
    {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next)
        {
            block->next->prev = block;
        }
    }

    if (block->prev && block->prev->is_free)
    {
        block->prev->size += BLOCK_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next)
        {
            block->next->prev = block->prev;
        }
    }
}

static block_header_t *extend_heap(size_t size)
{
    size = (size + 7) & ~7;

    size_t num_units = (size + DEFAULT_HEAP_SIZE - 1) / DEFAULT_HEAP_SIZE;
    size_t alloc_size = num_units * DEFAULT_HEAP_SIZE;

    void *p = sbrk(alloc_size);
    if (p == (void *)-1)
    {
        return NULL;
    }

    heap_total_size += alloc_size;

    block_header_t *new_block = (block_header_t *)p;
    new_block->size = alloc_size - BLOCK_HEADER_SIZE;
    new_block->is_free = 1;
    new_block->next = NULL;
    new_block->prev = NULL;

    block_header_t *current = (block_header_t *)heap_start_addr;
    if (current)
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_block;
        new_block->prev = current;
    }
    else
    {
        heap_start_addr = new_block;
    }

    coalesce(new_block);

    return new_block;
}

void *my_malloc(size_t size, alloc_algo_t algo)
{
    if (size == 0)
        return NULL;

    if (heap_start_addr == NULL)
    {
        if (my_memory_init(DEFAULT_HEAP_SIZE) != 0)
        {
            return NULL;
        }
    }

    size = (size + 7) & ~7;

    block_header_t *block = find_free_block(size, algo);

    if (block == NULL)
    {
        size_t needed = size + BLOCK_HEADER_SIZE;
        block = extend_heap(needed);

        if (block == NULL)
        {
            return NULL;
        }

        block = find_free_block(size, algo);
    }

    if (block)
    {
        split_block(block, size);
        block->is_free = 0;
        return (void *)((char *)block + BLOCK_HEADER_SIZE);
    }

    return NULL;
}

void my_free(void *ptr)
{
    if (!ptr)
        return;

    block_header_t *block = (block_header_t *)((char *)ptr - BLOCK_HEADER_SIZE);
    block->is_free = 1;

    coalesce(block);
}

void *my_calloc(size_t num, size_t size, alloc_algo_t algo)
{
    size_t total_size = num * size;
    void *ptr = my_malloc(total_size, algo);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void print_heap_stats(void *highlight_ptr)
{
    printf("--- Heap Stats ---\n");
    block_header_t *current = (block_header_t *)heap_start_addr;
    int i = 0;
    while (current != NULL)
    {
        void *data_ptr = (void *)((char *)current + BLOCK_HEADER_SIZE);
        int highlight = (highlight_ptr != NULL && data_ptr == highlight_ptr);

        if (highlight)
            printf("----------------------------------------\n");
        printf("Block %d: [%s] Size: %zu bytes (Addr: %p)\n",
               i++,
               current->is_free ? "FREE" : "USED",
               current->size,
               current);
        if (highlight)
            printf("----------------------------------------\n");

        current = current->next;
    }
    printf("Total Blocks: %d\n", i);
    printf("------------------\n");
}

void print_block_count(void)
{
    printf("Total Blocks: %d\n", get_total_block_count());
}

int get_total_block_count(void)
{
    block_header_t *current = (block_header_t *)heap_start_addr;
    int count = 0;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}

void print_total_size(void)
{
    block_header_t *current = (block_header_t *)heap_start_addr;
    size_t total_size = 0;
    while (current != NULL)
    {
        total_size += current->size;
        current = current->next;
    }

    if (total_size < 1024)
    {
        printf("Total Size: %zu Bytes\n", total_size);
    }
    else if (total_size < 1024 * 1024)
    {
        printf("Total Size: %.2f KB\n", (double)total_size / 1024.0);
    }
    else
    {
        printf("Total Size: %.2f MB\n", (double)total_size / (1024.0 * 1024.0));
    }
}

void dump_heap_state(const char *filepath, int step, const char *op, void *highlight_ptr, const char *algo_name)
{
    FILE *f = fopen(filepath, "a");
    if (!f)
        return;

    fprintf(f, "{\"step\": %d, \"algo\": \"%s\", \"op\": \"%s\", \"highlight\": \"%p\", \"blocks\": [",
            step, algo_name, op, highlight_ptr);

    block_header_t *current = (block_header_t *)heap_start_addr;
    int first = 1;
    while (current != NULL)
    {
        if (!first)
            fprintf(f, ",");
        fprintf(f, "{\"addr\": \"%p\", \"size\": %zu, \"is_free\": %s}",
                (void *)(current + 1), current->size, current->is_free ? "true" : "false");
        first = 0;
        current = current->next;
    }
    fprintf(f, "]}\n");
    fclose(f);
}
