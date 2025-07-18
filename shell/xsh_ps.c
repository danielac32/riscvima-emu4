/* xsh_ps.c - xsh_ps */

#include <os.h>
 
/*------------------------------------------------------------------------
 * xsh_ps - shell command to print the process table
 *------------------------------------------------------------------------
 */
shellcmd xsh_ps(int nargs, char *args[])
{

//	uint32 * linkreg;
//	asm volatile ("mov %0, lr" : : "r" (linkreg));
//	kprintf("LINK REG: %x\n", linkreg);

	struct	procent	*prptr;		/* pointer to process		*/
	int32	i;			/* index into proctabl		*/
	char *pstate[]	= {		/* names for process states	*/
		"free ", "curr ", "ready", "recv ", "sleep", "susp ",
		"wait ", "rtime"};

	/* For argument '--help', emit help about the 'ps' command	*/


	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		fprintf(stdout,"Use: %s\n\n", args[0]);
		fprintf(stdout,"Description:\n");
		fprintf(stdout,"\tDisplays information about running processes\n");
		fprintf(stdout,"Options:\n");
		fprintf(stdout,"\t--help\t display this help and exit\n");
		return 0;
	}

	/* Check for valid number of arguments */

	if (nargs > 1) {
		fprintf(stdout, "%s: too many arguments\n", args[0]);
		fprintf(stdout, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	/* Print header for items from the process table */

	fprintf(stdout,"%3s %-16s %5s %10s %-10s %10s\n",
		   "Pid", "Name", "State", "Stack Base",
		   "Stack Ptr", "Stack Size");

	fprintf(stdout,"%3s %-16s %5s %10s %-10s %10s\n",
		   "---", "----------------", "-----",
		   "----------", "----------", "----------");

	/* Output information for each process */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		fprintf(stdout,"%3d %-16s %s 0x%08X 0x%08X %8d\n",
			i, prptr->prname, pstate[(int)prptr->prstate], prptr->prstkbase,
			prptr->prstkptr, prptr->prstklen);
	}

	return 0;
}
