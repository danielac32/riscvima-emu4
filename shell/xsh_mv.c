
#include <os.h>
#include <fat_filelib.h>



int rename(char *src,char *dst){
    return 0;
}
shellcmd xsh_mv(int nargs, char *args[]) {
    if (nargs != 3) {
        fprintf(stdout,"Uso: mv <origen> <destino>\n");
        return 1;
    }

    char *src = full_path(args[1]);
    char *dst = full_path(args[2]);

    if (!src || !dst) return -1;

    if (rename(src, dst) == 0) {
        fprintf(stdout,"Renombrado/movido: %s -> %s\n", src, dst);
    } else {
        fprintf(stdout,"Error al mover/renombrar %s -> %s\n", src, dst);
    }

    return 0;
}