#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mach/mach_time.h>

// to avoid compiler (clang) optimization
void prevent_optimization(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

struct Node {
    struct Node* next;
    uint8_t padding[120];  // 56 is also working
};

// shuffle the nodes
void shuffle_nodes(struct Node* array, size_t count) {
    for (size_t i = 0; i < count; i++) {
        array[i].next = &array[(i + 1) % count];
    }
    srand((unsigned int)time(NULL));
    for (size_t i = count - 1; i > 0; i--) {
        size_t j = rand() % i;
        struct Node* temp = array[i].next;
        array[i].next = array[j].next;
        array[j].next = temp;
    }
}

// shuffle pointer arrays (for test 2 and test 3)
void shuffle_node_pointers(struct Node** nodes, size_t count) {
    srand((unsigned int)time(NULL));
    for (size_t i = count - 1; i > 0; i--) {
        size_t j = rand() % i;
        struct Node* temp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = temp;
    }
    for (size_t i = 0; i < count; i++) {
        nodes[i]->next = nodes[(i + 1) % count];
    }
}

// Cache size test L1 L2 L3
void run_cache_size_test(mach_timebase_info_data_t* timebase) {
    uint64_t func_start = mach_absolute_time(); 

    printf("\n=== TEST 1: Extracting Cache Levels (Cache Sizes) ===\n");
    printf("%-20s\t%-15s\t%-15s\n", "Working Set (KB)", "Num Nodes", "Latency (ns)");
    printf("----------------------------------------------------------------------\n");

    size_t min_size = 4 * 1024; 
    size_t max_size = 64 * 1024 * 1024; 

    for (size_t size = min_size; size <= max_size; size *= 2) {
        size_t num_nodes = size / sizeof(struct Node);
        if (num_nodes < 2) continue;

        struct Node* array = (struct Node*)malloc(num_nodes * sizeof(struct Node));
        if (!array) continue;

        //shuffle
        shuffle_nodes(array, num_nodes);

        // Warm-up
        struct Node* curr = &array[0];
        for (size_t i = 0; i < num_nodes * 5; i++) curr = curr->next;
        prevent_optimization(curr);

        uint64_t start = mach_absolute_time();
        size_t traversals = 10000000;
        curr = &array[0];
        
        for (size_t i = 0; i < traversals; i++) {
            curr = curr->next;
        }
        prevent_optimization(curr);
        uint64_t end = mach_absolute_time();

        uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
        double latency = (double)elapsed_nano / traversals;

        printf("%-20.2f\t%-15zu\t%-15.3f\n", (double)size / 1024.0, num_nodes, latency);
        free(array);
    }

    uint64_t func_end = mach_absolute_time();
    double func_elapsed_ms = (double)(func_end - func_start) * timebase->numer / timebase->denom / 1000000.0;
    printf(">> TEST 1 Total Execution Time: %.3f ms (%.3f s)\n", func_elapsed_ms, func_elapsed_ms / 1000.0);
}

// Cache line size test 
void run_cache_line_test(mach_timebase_info_data_t* timebase) {
    uint64_t func_start = mach_absolute_time(); 

    printf("\n=== TEST 2: Extracting Cache Line Size ===\n");
    printf("%-20s\t%-15s\n", "Stride (Bytes)", "Latency (ns)");
    printf("-----------------------------------------\n");


    const size_t DATA_SIZE = 1024 * 1024 * 32; 

    for (size_t stride = 8; stride <= 1024; stride *= 2) {
        
        size_t num_nodes = DATA_SIZE / stride;
        if (num_nodes < 2) continue;

        void* raw_mem = NULL;
        if (posix_memalign(&raw_mem, 128, DATA_SIZE) != 0) continue;

        struct Node** nodes = malloc(num_nodes * sizeof(struct Node*));
        if (!nodes) {
            free(raw_mem);
            continue;
        }

        for (size_t i = 0; i < num_nodes; i++) {
            nodes[i] = (struct Node*)((uintptr_t)raw_mem + (i * stride));
        }

        shuffle_node_pointers(nodes, num_nodes);

        struct Node* curr = nodes[0];
        
        // Warm-up 
        for (size_t i = 0; i < num_nodes * 5; i++) curr = curr->next;
        prevent_optimization(curr);

        uint64_t start = mach_absolute_time();
        
        // start
        size_t traversals = 10000000;
        for (size_t t = 0; t < traversals; t++) {
            curr = curr->next;
        }
        prevent_optimization(curr);
        uint64_t end = mach_absolute_time();

        uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
        double latency = (double)elapsed_nano / traversals;

        printf("%-20zu\t%-15.3f\n", stride, latency);
        
        free(nodes);
        free(raw_mem);
    }

    uint64_t func_end = mach_absolute_time(); 
    double func_elapsed_ms = (double)(func_end - func_start) * timebase->numer / timebase->denom / 1000000.0;
    printf(">> TEST 2 Total Execution Time: %.3f ms (%.3f s)\n", func_elapsed_ms, func_elapsed_ms / 1000.0);
}

// set-associative test
void run_associativity_test(mach_timebase_info_data_t* timebase) {
    uint64_t func_start = mach_absolute_time(); 

    printf("\n=== TEST 3: Estimating Cache Associativity ===\n");
    printf("%-20s\t%-15s\n", "Num Ways (N)", "Latency (ns)");
    printf("-----------------------------------------\n");

    size_t stride = 16 * 1024 ; 
    size_t max_ways = 16;
    void* raw_mem = NULL;
    posix_memalign(&raw_mem, stride, (max_ways + 2) * stride);
    if (!raw_mem) return;

    for (size_t ways = 1; ways <= max_ways; ways++) {
        struct Node** nodes = malloc(ways * sizeof(struct Node*));
        if (!nodes) continue;

        for (size_t i = 0; i < ways; i++) {
            nodes[i] = (struct Node*)((uintptr_t)raw_mem + (i * stride));
        }

        // shuffle the nodes to create a random access pattern
        shuffle_node_pointers(nodes, ways);

        struct Node* curr = nodes[0];
        // Warm-up
        for (size_t i = 0; i < ways * 5; i++) curr = curr->next;
        prevent_optimization(curr);

        // start
        uint64_t start = mach_absolute_time();
        size_t traversals = 10000000;
        for (size_t t = 0; t < traversals; t++) {
            curr = curr->next;
        }
        prevent_optimization(curr);
        uint64_t end = mach_absolute_time();

        uint64_t elapsed_nano = (end - start) * timebase->numer / timebase->denom;
        double latency = (double)elapsed_nano / traversals;

        printf("%-20zu\t%-15.3f\n", ways, latency);
        free(nodes);
    }
    free(raw_mem);

    uint64_t func_end = mach_absolute_time(); 
    double func_elapsed_ms = (double)(func_end - func_start) * timebase->numer / timebase->denom / 1000000.0;
    printf(">> TEST 3 Total Execution Time: %.3f ms (%.3f s)\n", func_elapsed_ms, func_elapsed_ms / 1000.0);
}

int main() {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    srand((unsigned int)time(NULL));

    uint64_t total_start = mach_absolute_time();

    run_cache_size_test(&timebase);
    run_cache_line_test(&timebase);
    run_associativity_test(&timebase);

    uint64_t total_end = mach_absolute_time();
double total_elapsed_ms = (double)(total_end - total_start) * timebase.numer / timebase.denom / 1000000.0;
    printf("\n=========================================\n");
    printf(">> TOTAL PROGRAM EXECUTION TIME: %.3f ms (%.3f s)\n", total_elapsed_ms, total_elapsed_ms / 1000.0);
    printf("=========================================\n");
    printf("\n- Finished\n");
    return 0;
}