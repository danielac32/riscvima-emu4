/* xsh_echo.c - xsh_echo */

#include <os.h>
 #include <fat_filelib.h>
#include <elf.h>
#include <data.h>



extern uint32 load_elf(const char *file);
shellcmd xsh_run(int nargs, char *args[])
{




int child;

        int (*p)(void) = (int (*)(void))bin;
        child = create((int *)p,SHELL_CMDSTK, SHELL_CMDPRIO,"run", 2, nargs, &args[0]);
        
        struct  procent *prptr;
        prptr = &proctab[child];
        prptr->elf = TRUE;

    update_path();
    printf("pid: %d\n", child);
    resume(child);



   #if 0

FILE *fp = fopen("/out", "w");
if (!fp) {
    fprintf(stdout,"No se pudo abrir archivo: %s\n", "/out.bin");
    return 1;
}

fl_fwrite(hello,sizeof(hello), 1, fp);
fclose(fp);


	char *tmp=full_path((char*)args[1]);
    if (tmp==NULL)return -1;
    FILE *fptr;
    exec_img ximg;
    int32	tmparg;
    int child;
    

   //load_elf(tmp);
    uint32_t ret = elf_execve(tmp,&ximg);
    if(ret > 0){
        int (*p)(void) = (int (*)(void))ret;
        child = create((int *)p,SHELL_CMDSTK, SHELL_CMDPRIO,tmp, 2, nargs, &args[0]);
        
        struct	procent *prptr;
        prptr = &proctab[child];
        prptr->elf = TRUE;
        prptr->img = ximg.start;
        prptr->size = ximg.size;
        printf("%d\n", prptr->size);
        hexDump2(0,ret,ximg.size);
        //free(prptr->img);
        //resume(child);

    }else {
        printf("error loading elf process  %d %s\n",ret,tmp);
        return -1;
    }
    
    update_path();
    printf("pid: %d\n", child);
    resume(child);

  #endif
	 
	return 0;
}
