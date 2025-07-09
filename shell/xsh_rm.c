/* xsh_echo.c - xsh_echo */

#include <os.h>
 #include <fat_filelib.h>

shellcmd xsh_rm(int nargs, char *args[]) {
    if (nargs != 2) {
        fprintf(stdout,"Uso: rm <archivo>\n");
        return 1;
    }

    char *tmp = full_path(args[1]);
    if (!tmp) return -1;

    if (remove(tmp) == 0) {
        fprintf(stdout,"Archivo eliminado: %s\n", tmp);
    } else {
        fprintf(stdout,"No se pudo eliminar %s\n", tmp);
    }

    return 0;
}