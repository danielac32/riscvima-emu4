/* xsh_echo.c - xsh_echo */


#include <os.h>
#include <fat_filelib.h>

shellcmd xsh_touch(int nargs, char *args[]) {
    if (nargs != 2) {
        fprintf(stdout,"Uso: touch <archivo>\n");
        return 1;
    }

    char *tmp = full_path(args[1]);
    if (!tmp) return -1;

    FILE *file = fopen(tmp, "w");
    if (!file) {
        fprintf(stdout,"No se pudo crear %s\n", tmp);
        return 1;
    }

    fclose(file);
    fprintf(stdout,"Archivo creado: %s\n", tmp);
    return 0;
}