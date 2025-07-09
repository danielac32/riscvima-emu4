
#include <stdint.h>
#include <stdio.h>
#define DRAM_SIZE  (6 * 1024 * 1024)
#define BLOCK_SIZE 32

typedef struct {
    FILE *file;
    uint32_t size;
    char *p;
} Memory;


extern Memory mem;
uint32_t get_file_size(FILE *file);
Memory create_memory(const char *filename);
 