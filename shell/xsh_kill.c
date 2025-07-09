/* xsh_kill.c - xsh_kill */

#include <os.h>
#include <string.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_kill - obtain and print the current month, day, year, and time
 *------------------------------------------------------------------------
 */
shellcmd xsh_kill(int nargs, char *args[]) {

	int32	retval;			/* return value			*/
	pid32	pid;			/* ID of process to kill	*/
	char	ch;			/* next character of argument	*/
	char	*chptr;			/* walks along argument string	*/

	/* Output info for '--help' argument */

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		fprintf(stdout,"Usage: %s PID\n\n", args[0]);
		fprintf(stdout,"Description:\n");
		fprintf(stdout,"\tterminates a process\n");
		fprintf(stdout,"Options:\n");
		fprintf(stdout,"\tPID \tthe ID of a process to terminate\n");
		fprintf(stdout,"\t--help\tdisplay this help and exit\n");
		return OK;
	}

	/* Check argument count */

	if (nargs != 2) {
		fprintf(stdout, "%s: incorrect argument\n", args[0]);
		fprintf(stdout, "Try '%s --help' for more information\n",
			args[0]);
		return SYSERR;
	}

	/* compute process ID from argument string */

	chptr = args[1];
	ch = *chptr++;
	pid = 0;
	while(ch != NULLCH) {
		if ( (ch < '0') || (ch > '9') ) {
			fprintf(stdout, "%s: non-digit in process ID\n",
				args[0]);
			return 1;
		}
		pid = 10*pid + (ch - '0');
		ch = *chptr++;
	}
	if (pid == 0) {
		fprintf(stdout, "%s: cannot kill the null process\n",
			args[0]);
		return 1;
	}

	retval = kill(pid);
	if (retval == SYSERR) {
		fprintf(stdout, "%s: cannot kill process %d\n",
			args[0], pid);
		return 1;
	}
	return 0;
}
