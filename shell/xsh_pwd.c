/* xsh_echo.c - xsh_echo */

#include <os.h>
 

shellcmd xsh_pwd(int nargs, char *args[])
{
	char *s=full_path("");
    fprintf(stdout,"%s\n",s );
	return 0;
}
