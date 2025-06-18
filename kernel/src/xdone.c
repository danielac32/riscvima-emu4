/* xdone.c - xdone */

#include <os.h>

/*------------------------------------------------------------------------
 *  xdone  -  Print system completion message as last process exits
 *------------------------------------------------------------------------
 */
void	xdone(void)
{
	kprintf("\n\nAll user processes have completed.\n\n");
	for(;;);				/* Halt the processor		*/
}
