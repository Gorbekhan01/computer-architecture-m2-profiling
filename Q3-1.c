#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mach/mach_time.h>

// to avoid compiler (clang) optimization
void prevent_optimization(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

// Row major
void run_row_major_test(mach_timebase_info_data_t* timebase) {
    uint64_t func_start = mach_absolute_time();
    printf("\n=== TEST 1: Row-Major Matrix Traversal ===\n");

    const int SIZE = 4096;
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    if (!matrix) return;

    // Warm-up
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i * SIZE + j] = 0;
        }
    }
    prevent_optimization(matrix);

    uint64_t start = mach_absolute_time();
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i * SIZE + j] += i + j;
        }
    }
    uint64_t end = mach_absolute_time();
    prevent_optimization(matrix);

    uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
    double elapsed_ms = (double)elapsed_nano / 1000000.0;
    printf("Row-Major Traversal Time: %.3f ms\n", elapsed_ms);

    free(matrix);
    uint64_t func_end = mach_absolute_time();
    double func_elapsed_ms = (double)(func_end - func_start) * timebase->numer / timebase->denom / 1000000.0;
    printf(">> TEST 1 Total Execution Time: %.3f ms\n", func_elapsed_ms);
}

// Column major
void run_column_major_test(mach_timebase_info_data_t* timebase) {
    uint64_t func_start = mach_absolute_time();
    printf("\n=== TEST 2: Column-Major Matrix Traversal ===\n");

    const int SIZE = 4096;
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    if (!matrix) return;

    // Warm-up
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i * SIZE + j] = 0;
        }
    }
    prevent_optimization(matrix);

    uint64_t start = mach_absolute_time();
    
    for (int j = 0; j < SIZE; j++) {
        for (int i = 0; i < SIZE; i++) {
            matrix[i * SIZE + j] += i + j;
        }
    }
    uint64_t end = mach_absolute_time();
    prevent_optimization(matrix);

    uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
    double elapsed_ms = (double)elapsed_nano / 1000000.0;
    printf("Column-Major Traversal Time: %.3f ms\n", elapsed_ms);

    free(matrix);
    uint64_t func_end = mach_absolute_time();
    double func_elapsed_ms = (double)(func_end - func_start) * timebase->numer / timebase->denom / 1000000.0;
    printf(">> TEST 2 Total Execution Time: %.3f ms\n", func_elapsed_ms);
}

int main() {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    srand((unsigned int)time(NULL));

    uint64_t total_start = mach_absolute_time();

    run_row_major_test(&timebase);
    run_column_major_test(&timebase);

    uint64_t total_end = mach_absolute_time();
    double total_elapsed_ms = (double)(total_end - total_start) * timebase.numer / timebase.denom / 1000000.0;
    
    printf("\n=========================================\n");
    printf(">> TOTAL PROGRAM EXECUTION TIME: %.3f ms (%.3f s)\n", total_elapsed_ms, total_elapsed_ms / 1000.0);
    printf("=========================================\n");
    printf("\n- Finished\n");
    return 0;
}