#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#define SEED 12345

#define NUM_INITIAL_ALLOCS 10
#define NUM_FREES 5
#define NUM_SECOND_ALLOCS 5
#define MIN_SIZE 32
#define MAX_SIZE 512

#define BENCH_INITIAL_ALLOCS 1000
#define BENCH_FREES 500
#define BENCH_SECOND_ALLOCS 500

void *ptrs[BENCH_INITIAL_ALLOCS]; 

void run_test(alloc_algo_t algo, const char *name) {
    printf("========================================\n");
    printf("TESTING ALGORITHM (Visual): %s\n", name);
    printf("========================================\n");
    
    my_memory_reset();
    srand(SEED);

    for (int i = 0; i < NUM_INITIAL_ALLOCS; i++) ptrs[i] = NULL;

    printf("Step 1: Allocating %d blocks...\n", NUM_INITIAL_ALLOCS);
    for (int i = 0; i < NUM_INITIAL_ALLOCS; i++) {
        size_t size = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
        printf("Allocating [%d]: Size %zu\n", i, size);
        ptrs[i] = my_malloc(size, algo);
    }

    print_heap_stats(NULL);
    print_block_count();

    printf("Step 2: Freeing %d blocks...\n", NUM_FREES);
    int freed_count = 0;
    while (freed_count < NUM_FREES) {
        int idx = rand() % NUM_INITIAL_ALLOCS;
        if (ptrs[idx] != NULL) {
            printf("Freeing slot [%d]\n", idx);
            my_free(ptrs[idx]);
            ptrs[idx] = NULL;
            freed_count++;
        }
    }
    
    print_heap_stats(NULL);
    print_block_count();

    printf("Step 3: Allocating %d new blocks...\n", NUM_SECOND_ALLOCS);
    int alloc_count = 0;
    for (int i = 0; i < NUM_INITIAL_ALLOCS && alloc_count < NUM_SECOND_ALLOCS; i++) {
        if (ptrs[i] == NULL) {
            size_t size = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
            printf("----------------------------------------\n");
            printf("Re-allocating slot [%d]: Size %zu\n", i, size);
            ptrs[i] = my_malloc(size, algo);
            print_heap_stats(ptrs[i]);
            print_block_count();
            print_total_size();
            printf("----------------------------------------\n");
            alloc_count++;
        }
    }
    
    print_heap_stats(NULL);
    print_block_count();
    print_total_size();
    printf("Visual Test %s Completed.\n\n", name);
}

typedef struct {
    const char *name;
    double time;
    int total_blocks;
} BenchmarkResult;

BenchmarkResult run_benchmark(alloc_algo_t algo, const char *name) {
    printf("========================================\n");
    printf("BENCHMARK ALGORITHM: %s\n", name);
    printf("========================================\n");
    
    my_memory_reset();
    srand(12345);

    for (int i = 0; i < BENCH_INITIAL_ALLOCS; i++) ptrs[i] = NULL;

    for (int i = 0; i < BENCH_INITIAL_ALLOCS; i++) {
        size_t size = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
        ptrs[i] = my_malloc(size, algo);
    }

    int freed_count = 0;
    while (freed_count < BENCH_FREES) {
        int idx = rand() % BENCH_INITIAL_ALLOCS;
        if (ptrs[idx] != NULL) {
            my_free(ptrs[idx]);
            ptrs[idx] = NULL;
            freed_count++;
        }
    }

    clock_t start_time = clock();

    int alloc_count = 0;
    for (int i = 0; i < BENCH_INITIAL_ALLOCS && alloc_count < BENCH_SECOND_ALLOCS; i++) {
        if (ptrs[i] == NULL) {
            size_t size = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
            ptrs[i] = my_malloc(size, algo);
            alloc_count++;
        }
    }

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Benchmark %s Completed.\n\n", name);

    printf("--- Performance Stats ---\n");
    printf("Time taken for Step 3 (%d allocs): %f seconds\n", BENCH_SECOND_ALLOCS, time_taken);
    print_block_count();
    print_total_size();
    printf("-------------------------\n");

    BenchmarkResult result;
    result.name = name;
    result.time = time_taken;
    result.total_blocks = get_total_block_count();
    return result;
}

void save_results_to_json(const char *filename, BenchmarkResult *results, int count) {
    FILE *fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "[\n");
        for (int i = 0; i < count; i++) {
            fprintf(fp, "  {\"name\": \"%s\", \"time\": %f, \"total_blocks\": %d}%s\n", 
                    results[i].name, results[i].time, results[i].total_blocks, (i < count - 1) ? "," : "");
        }
        fprintf(fp, "]\n");
        fclose(fp);
        printf("Benchmark results written to %s\n", filename);
    } else {
        printf("Error writing to %s\n", filename);
    }
}

int main() {
    run_test(ALGO_FIRST_FIT, "FIRST_FIT");
    run_test(ALGO_BEST_FIT, "BEST_FIT");
    run_test(ALGO_WORST_FIT, "WORST_FIT");
    
    BenchmarkResult results[3];
    results[0] = run_benchmark(ALGO_FIRST_FIT, "FIRST_FIT");
    results[1] = run_benchmark(ALGO_BEST_FIT, "BEST_FIT");
    results[2] = run_benchmark(ALGO_WORST_FIT, "WORST_FIT");
    
    save_results_to_json("results.json", results, 3);

    return 0;
}
