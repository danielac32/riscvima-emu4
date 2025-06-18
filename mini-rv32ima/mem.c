
#include <mem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 

 
uint32_t get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END); // Moverse al final del archivo
    uint32_t size = ftell(file); // Obtener la posición actual
    rewind(file); // Volver al inicio del archivo
    return size; // Retornar el tamaño
}

Memory create_memory(const char *filename) {
    Memory mem;

    mem.file = fopen(filename, "rb"); // Abrir el archivo en modo lectura
    if (mem.file == NULL) {
        printf("Error: No se pudo abrir el archivo.\n");
        while(1);
    }

    // Obtener el tamaño del archivo original
    mem.size = get_file_size(mem.file);
    mem.p = malloc(DRAM_SIZE);
    
    uint8_t buffer[BLOCK_SIZE]; // Buffer para copiar datos
    size_t bytes_read;
    fread(mem.p, 1, mem.size,mem.file);
 
    fclose(mem.file);
    //hexDump2(0,mem.p,mem.size);
    printf("END\n");
    return mem;
}



// Función para liberar la memoria
void free_memory(Memory *mem) {
    fclose(mem->file);
    mem->file = NULL;
    mem->size = 0;
    free(mem->p);
}
 

