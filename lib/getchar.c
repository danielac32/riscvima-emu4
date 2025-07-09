/* getchar.c - getchar */
#if 1
#include <os.h>
//#include <stdio.h>

/*------------------------------------------------------------------------
 *  getchar  -  DOCUMENT
 *------------------------------------------------------------------------
 */
int	getchar(
	  void
	)
{
    return fgetc(stdin);
}
#endif