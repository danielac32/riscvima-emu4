/* xsh_memdump.c - xsh_memdump */

#include <os.h>

 
/*------------------------------------------------------------------------
 * xsh_memdump - dump a region of memory by displaying values in hex
 *			 and ascii
 *------------------------------------------------------------------------
 */


 

shellcmd xsh_memdump(int nargs, char *args[])
{
	
long from = 0;
long to = 0x8FF;		/* AVR atmega328p top RAM address */

 if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		fprintf(stdout,"Usage: %s memdump [text,data,bss]\n\n", args[0]);
		fprintf(stdout,"Description:\n");
		
		return OK;
	}
     

	return 0;
}
