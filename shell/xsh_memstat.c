/* xsh_memstat.c - xsh_memstat */

#include <os.h>
#include <stdio.h>
#include <string.h>

static	void	printMemUse(void);
static	void	printFreeList(void);

/*------------------------------------------------------------------------
 * xsh_memstat - Print statistics about memory use and dump the free list
 *------------------------------------------------------------------------
 */
shellcmd xsh_memstat(int nargs, char *args[])
{


	uint32_t f=heap_free();
    fprintf(stdout,"free memory: %d\n",f);
	 #if 1
	/* For argument '--help', emit help about the 'memstat' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		fprintf(stdout,"use: %s \n\n", args[0]);
		fprintf(stdout,"Description:\n");
		fprintf(stdout,"\tDisplays the current memory use and prints the\n");
		fprintf(stdout,"\tfree list.\n");
		fprintf(stdout,"Options:\n");
		fprintf(stdout,"\t--help\t\tdisplay this help and exit\n");
		return 0;
	}

	/* Check for valid number of arguments */

	if (nargs > 1) {
		fprintf(stdout,"%s: too many arguments\n", args[0]);
		fprintf(stdout,"Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}



    /*struct	memblk	*memptr;	
	uint32	free_mem;	


    free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf(" bytes of free memory.  Free list: %10d\n", heap_free());
    //kprintf(" bytes of free memory.  Free list2: %10d\n", heap_freecc());
*/
	
	//printMemUse();
	printFreeList();
#endif
	return 0;
}


/*------------------------------------------------------------------------
 * printFreeList - Walk the list of free memory blocks and print the
 *			location and size of each
 *------------------------------------------------------------------------
 */
static void printFreeList(void)
{
	struct memblk *block;

	/* Output a heading for the free list */



	fprintf(stdout,"Free List:\n");
	fprintf(stdout,"Block address  Length (dec)  Length (hex)\n");
	fprintf(stdout,"-------------  ------------  ------------\n");
	
	for (block = memlist.mnext; block != NULL; block = block->mnext) {
		fprintf(stdout,"  0x%08x    %9d     0x%08x\n", block,
			block->mlength, block->mlength);
	}
	fprintf(stdout,"\n");
}

