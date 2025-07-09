/* xsh_echo.c - xsh_echo */

#include <os.h>
#include <fat_filelib.h>

/*------------------------------------------------------------------------
 * xhs_echo - write argument strings to stdout
 *------------------------------------------------------------------------
 */
shellcmd xsh_echo(int nargs, char *args[]) {
    if (nargs < 2) {
        fprintf(stdout,"Uso: echo [texto] [> archivo]\n");
        return 1;
    }

    int i = 1;
    int redirect = 0;
    char *filename = NULL;

    // Verificar si hay redirección
    for (i = 1; i < nargs; ++i) {
        if (!strcmp(args[i], ">")) {
            redirect = 1;
            if (++i >= nargs) {
                fprintf(stdout,"Falta nombre de archivo después de '>'\n");
                return 1;
            }
            filename = full_path(args[i]);
            break;
        }
    }

    if (redirect) {
        FILE *fp = fopen(filename, "w");
        if (!fp) {
            fprintf(stdout,"No se pudo abrir archivo: %s\n", filename);
            return 1;
        }

        for (int j = 1; j < i - 1; ++j) {
            fl_fputs(args[j], fp);
            fl_fputc(' ', fp);
        }
        fl_fputc('\n', fp);

        fclose(fp);
        fprintf(stdout,"Texto escrito a: %s\n", filename);
    } else {
        for (int j = 1; j < nargs; ++j) {
            fprintf(stdout,"%s ", args[j]);
        }
        fprintf(stdout,"\n");
    }

    return 0;
}