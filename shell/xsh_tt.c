#if 0
#include <os.h>
#include <stdio.h>
#include "compilermonkey.h"
#include "object.h"
#include "vm.h"


/*------------------------------------------------------------------------
 * xhs_echo - write argument strings to stdout
 *------------------------------------------------------------------------
 */

char *readfile(char *path){
    char *tmp=full_path(path);
    if (tmp==NULL){
        update_path();
        return NULL;
    }

    if(!disk.exist(tmp)){
       printf("%s file not found!\n",tmp );
       update_path();
       return NULL;
    }

    uint8 *buff;
  
    FILE *fs=disk.open(tmp,LFS_O_RDWR);
    int size = disk.size(fs);
    printf("size file %d\n",size);
    buff = malloc(size+100);
    memset(buff,0,size);
    disk.read(buff,size,fs);
    disk.close(fs);
    buff[size] = '\0';
    return buff;
}



shellcmd xsh_monkey(int nargs, char *args[])
{
    int32   i;          /* walks through args array */


    uint32_t f=heap_free();
    kprintf("free memory: %d\n",f);

    char *input=readfile(args[1]);
    
    f=heap_free();
    kprintf("free memory readfile: %d\n",f);


    if(input==NULL)return 0;
    
    struct lexer lexer = new_lexer(input);
    f=heap_free();
    kprintf("free memory new_lexer: %d\n",f);

    struct parser parser = new_parser(&lexer);
    f=heap_free();
    kprintf("free memory new_parser: %d\n",f);

    struct program *program = parse_program(&parser);
    f=heap_free();
    kprintf("free memory parse_program: %d\n",f);


    if (parser.errors > 0) {
        for (int8_t i = 0; i < parser.errors; i++) {
            printf("%s\n",parser.error_messages[i]);
        }

        goto exit;//return 0;
    }

    struct compiler *compiler = compiler_new();

    f=heap_free();
    kprintf("free memory compiler_new: %d\n",f);


    int err = compile_program(compiler, program);
    f=heap_free();
    kprintf("free memory compile_program: %d\n",f);


    if (err) {
        printf("%s\n",compiler_error_str(err));
        goto exit;//return 0;
    }

    struct bytecode *code = get_bytecode(compiler);
    struct vm *machine = vm_new(code);

    f=heap_free();
    kprintf("free memory vm_new: %d\n",f);


    err = vm_run(machine);
    f=heap_free();
    kprintf("free memory vm_run: %d\n",f);

    if (err) {
        printf("Error executing bytecode: %d\n", err);
        goto exit;//return 0;
    }
     
    exit:
    free_program(program);
    compiler_free(compiler);
    free(code);
    vm_free(machine);
    free(input);
    f=heap_free();
    kprintf("free memory: %d\n",f);
 
    return 0;
}
 #endif