// Q3-1 optimized version

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mach/mach_time.h>

void prevent_optimization(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

const int SIZE = 4096;
const int TILE_SIZE = 32; 

void run_optimized_row_major(mach_timebase_info_data_t* timebase) {
    printf("\n=== OPTIMIZED 1: Row-Major Traversal ===\n");

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
    printf("Optimized Row-Major Time: %.3f ms\n", (double)elapsed_nano / 1000000.0);

    free(matrix);
}

void run_optimized_column_major(mach_timebase_info_data_t* timebase) {
    printf("\n=== OPTIMIZED 2: Column-Major Traversal (Cache Tiling) ===\n");

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


    // optimized 
    for (int jj = 0; jj < SIZE; jj += TILE_SIZE) {
        for (int ii = 0; ii < SIZE; ii += TILE_SIZE) {
            for (int j = jj; j < jj + TILE_SIZE; j++) {
                for (int i = ii; i < ii + TILE_SIZE; i++) {
                    matrix[i * SIZE + j] += i + j;
                }
            }
        }
    }

    uint64_t end = mach_absolute_time();
    prevent_optimization(matrix);

    uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
    printf("Optimized Column-Major Time: %.3f ms\n", (double)elapsed_nano / 1000000.0);

    free(matrix);
}

int main() {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    uint64_t total_start = mach_absolute_time();

    run_optimized_row_major(&timebase);
    run_optimized_column_major(&timebase);

    uint64_t total_end = mach_absolute_time();
    double total_elapsed_ms = (double)(total_end - total_start) * timebase.numer / timebase.denom / 1000000.0;

    printf("\n=========================================\n");
    printf(">> TOTAL OPTIMIZED EXECUTION TIME: %.3f ms\n", total_elapsed_ms);
    printf("=========================================\n");

    return 0;
}