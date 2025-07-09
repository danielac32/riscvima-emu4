/* xsh_echo.c - xsh_echo */

#include <os.h>
#include <fat_filelib.h>



shellcmd xsh_mkdir(int nargs, char *args[]) {
    if (nargs != 2) {
        fprintf(stdout,"Uso: mkdir <directorio>\n");
        return 1;
    }

    char *tmp = full_path(args[1]);
    if (!tmp) return -1;

    if (fl_is_dir(tmp)) {
        fprintf(stdout,"Directorio ya existe: %s\n", tmp);
        return 0;
    }

    mkdir(tmp);
    

    return 0;
}

