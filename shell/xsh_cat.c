/* xsh_echo.c - xsh_echo */


#include <os.h>
 #include <fat_filelib.h>


shellcmd xsh_cat(int nargs, char *args[]) {
    if (nargs < 2) {
        fprintf(stdout,"Uso: cat [archivo] | cat > archivo\n");
        return 1;
    }

    // Mostrar archivo
    char *filename = full_path(args[1]);
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        fprintf(stdout,"No se pudo abrir archivo: %s\n", filename);
        return 1;
    }

    char c;

    while(!feof(fp)){
          char c = fl_fgetc(fp);
          fputc(c,stdout);
    }

    fclose(fp);
    return 0;
}