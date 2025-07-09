
#include <os.h>
#include <fat_filelib.h>


shellcmd xsh_rmdir(int nargs, char *args[]) {
    if (nargs != 2) {
        fprintf(stdout,"Uso: rmdir <directorio>\n");
        return 1;
    }

    char *tmp = full_path(args[1]);
    if (!tmp) return -1;

    if (fl_is_dir(tmp)) {
        if (rmdir(tmp) == 0) {
            fprintf(stdout,"Directorio eliminado: %s\n", tmp);
        } else {
            fprintf(stdout,"No se pudo eliminar directorio: %s\n", tmp);
        }
    } else {
        fprintf(stdout,"No es un directorio: %s\n", tmp);
    }

    return 0;
}