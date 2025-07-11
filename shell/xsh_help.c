/* xsh_help.c - xsh_help */

#include <os.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xhs_help - display help message that lists shell commands
 *------------------------------------------------------------------------
 */
shellcmd xsh_help(int nargs, char *args[])
{
	int32	i;
	char	*argv[2];		/* argument vector for call	*/
	char	*src, *cmp;		/* used for string compare	*/
	int32	len;			/* length of a command name	*/
	int32	maxlen;			/* maximum length of all	*/
					/*   command names		*/
	int32	cols;			/* number of columns in the	*/
					/*   formatted command list	*/
	int32	spac;			/* space per column in the	*/
					/*   formatted command list	*/
	int32	lines;			/* total lines of output in the	*/
					/*   formatted command list	*/
	int32	j;			/* index of commands across one	*/
					/*   line of formatted output	*/
	
	/* For argument '--help', emit help about the 'help' command	*/


   ///int _stdout	= ((proctab[getpid()]).prdesc[1]);

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {

		fprintf(stdout,"Use:\n");
		fprintf(stdout,"\t%s [command]\n", args[0]);
		fprintf(stdout,"Description:\n");
		fprintf(stdout,"\tProvides a list of shell commands or\n");
		fprintf(stdout,"\thelp information for a specific command\n");
		fprintf(stdout,"Options:\n");
		fprintf(stdout,"\tcommand\tspecific command for which to\n");
		fprintf(stdout,"\t\tdisplay help information\n");
		fprintf(stdout,"\t--help\tdisplay this help and exit\n");
		return OK;
	}
     
	/* Check for valid number of arguments */

	if (nargs > 2) {
		fprintf(stdout, "%s: too many arguments\n", args[0]);
		fprintf(stdout, "Try '%s --help' for more information\n",
				args[0]);
		return OK;
	}

	/* Output help for specific command given as an argument */

	if (nargs == 2) {
		for (i = 0; i < ncmd; i++) {
			src = cmdtab[i].cname;
			cmp = args[1];
			while (*src != NULLCH) {
				if (*src != *cmp) {
					break;
				}
				src++;
				cmp++;
			}
			if ( (*src != NULLCH) || (*cmp != NULLCH) ) {
				continue;
			}

			/* prepare args for shell command */

			argv[0] = args[1];
			argv[1] = "--help";
			(*cmdtab[i].cfunc) (2, argv);
			return OK;
		}
		fprintf(stdout,"%s: no such command as '%s'\n", args[0], args[1]);
		return OK;
	}

	/* No arguments -- print a list of shell commands */

	fprintf(stdout,"\nshell commands are:\n\n");

	/* Calculate the maximum length of a command name */

	maxlen = 0;
	for (i = 0; i < ncmd; i++) {
		len = strnlen(cmdtab[i].cname, 80);
		if (len > maxlen) {
			maxlen = len;
		}
	}

	/* Calculate the number of command names per line */

	cols = 80/(maxlen+1);
	if (cols > 6) {
		cols = 6;
	}

	/* Calculate the width of a column */

	spac = 80/cols;

	/* Calculate the number of lines of output */

	lines = (ncmd+(cols-1))/cols;

	/* print the lines of command names */

	for (i=0; i<lines; i++) {
		for (j=i; j<ncmd; j+=lines) {
			len = strnlen(cmdtab[j].cname,80);
			fprintf(stdout,"%s", cmdtab[j].cname);
			while (len < spac) {
				fprintf(stdout," ");
				len++;
			}
		}
		fprintf(stdout,"\n");
	}
	return OK;
}
