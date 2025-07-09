#include <os.h>
#include <csr.h>
#include <syscall.h>
#include "disk.h"
#include "fat_filelib.h"


extern void *csr_read_dynamic(...);
 

static	void sysinit(); 	/* Internal system initialization	*/
 
struct  procent proctab[NPROC]; /* Process table            */
struct  sentry  semtab[NSEM];
struct  memblk  memlist;
int prcount;        /* Total number of live processes   */
pid32   currpid;        /* ID of currently executing process    */

 
void ilde(){

    while(1){
    	 yield();
    }
} 
 
 
 
int  initFat32(){
	init:
	uint32 size=sd_init();
  fl_init();
      // Attach media access functions to library
	if (fl_attach_media(sd_readsector, sd_writesector) != FAT_INIT_OK)
	{
	      printf("ERROR: Failed to init file system\n");
        if (fl_format(size, "")){
            kprintf("format ok\n");
            goto init;
        }
	      
	      return -1;
	}
    printf("fat32 (%d) %d\n",512,size);
  // List the root directory
    fl_listdirectory("/");
    return OK;
}

extern int telnet(int nargs, char *args[]);
extern int telnet2(int nargs, char *args[]);
int nullprocess(void) {
kprintf("init process....\n");
int32	msg;

//open(TELNET,"","");

	/*uint8_t *buffer = (uint8_t *)malloc(512);  
	__syscall(0xcafe0003,buffer,0);
	hexDump2(0,buffer, 512); */


  int pid = create(initFat32, 2048, 50, "fat32", 0);
  msg = recvclr();
	resume(pid);
  msg = receive();
	while (msg != pid) {
		msg = receive();
	}
	
char num[10];
sprintf(num, "%d", CONSOLE);
resume(create(shell, 2048*2, 50, "shell", 1,num));
//resume(create(telnet, 2048*2, 10, "telnet-server", 0));
resume(create(telnet2, 2048*2, 10, "telnet-server", 0));
return 0;
}

 
void	nulluser(){
uint32	free_mem;	
kprintf ("Build date: %s %s\n\n", __DATE__, __TIME__);
disable();
sysinit();
preempt = QUANTUM;
 

int pid=create(ilde, 2048, 1, "ilde", 0);
struct procent * prptr = &proctab[pid];
prptr->prstate = PR_CURR; 
//ready(create(ilde, 2048, 3, "ilde", 0));
enable();
ready(create(nullprocess, 2048, 3, "start process", 0));
//nullprocess();
for(;;){
	//kprintf("riscv32 ......... \n");
	yield();
}
}


static	void	sysinit()
{
	int32	i;
	struct procent *prptr;     /* Ptr to process table entry   */
    struct  sentry  *semptr;
    clkinit();
    syscall_os_init();
	for (int i = 0; i < 2; ++i)
	{
		kprintf("\n");
	}
	
	prcount = 0;
//	prcount = 1;


	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->elf = FALSE;
        prptr->prprio = 0;
	}


    for (i = 0; i < NSEM; i++) {
        semptr = &semtab[i];
        semptr->sstate = S_FREE;
        semptr->scount = 0;
        semptr->squeue = newqueue();
    }

    
    readylist = newqueue();

	
	for (i = 0; i < NDEVS; i++) {
	    init(i);
	}
	return;
}
