
#include <os.h>
#include <fat_filelib.h>

#include "compiler.h"
#include "object.h"
#include "vm.h"


char *readfile(char *path) {
    // Obtener la ruta absoluta
    char *tmp = full_path(path);
    if (!tmp) {
        fprintf(stdout,"Error: no se pudo resolver la ruta\n");
        update_path();
        return NULL;
    }

    // Abrir archivo
    FILE *fp = fopen(tmp, "rb");  // Leer en modo binario
    if (!fp) {
        fprintf(stdout,"%s: archivo no encontrado\n", tmp);
        update_path();
        return NULL;
    }

    // Obtener tamaño del archivo
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= 0) {
        fprintf(stdout,"%s: archivo vacío\n", tmp);
        fclose(fp);
        return NULL;
    }

    // Asignar memoria para el contenido + \0
    char *buffer = (char *)malloc(size + 100);
    if (!buffer) {
        fprintf(stdout,"No hay memoria suficiente para leer %s\n", tmp);
        fclose(fp);
        return NULL;
    }

    // Leer contenido
    size_t bytes_read = fread(buffer, size, 1, fp);
    fclose(fp);

    if (bytes_read != size) {
        fprintf(stdout,"Error leyendo %s (%ld bytes leídos de %ld)\n", tmp, bytes_read, size);
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';  // Terminador nulo
    return buffer;
}




shellcmd xsh_monkey(int nargs, char *args[]){

    int32	i;			/* walks through args array	*/

    char *input=readfile(args[1]);
    if(input==NULL)return 0;
    
    struct lexer lexer = new_lexer(input);
    struct parser parser = new_parser(&lexer);
    struct program *program = parse_program(&parser);
  

    if (parser.errors > 0) {
        for (int8_t i = 0; i < parser.errors; i++) {
            fprintf(stdout,"%s\n",parser.error_messages[i]);
        }

        goto exit;//return 0;
    }

    struct compiler *compiler = compiler_new();
    int err = compile_program(compiler, program);
    if (err) {
        fprintf(stdout,"%s\n",compiler_error_str(err));
        goto exit;//return 0;
    }

    struct bytecode *code = get_bytecode(compiler);
    struct vm *machine = vm_new(code);

    err = vm_run(machine);
    
    if (err) {
        fprintf(stdout,"Error executing bytecode: %d\n", err);
        goto exit;//return 0;
    }
     
    exit:
    free_program(program);
    compiler_free(compiler);
    free(code);
    vm_free(machine);
    free(input);
return 0;
}