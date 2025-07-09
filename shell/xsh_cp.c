
#include <os.h>
#include <fat_filelib.h>


shellcmd xsh_cp(int nargs, char *args[]) {
    if (nargs != 3) {
        fprintf(stdout,"Uso: cp <origen> <destino>\n");
        return 1;
    }

    char *src = full_path(args[1]);
    char *dst = full_path(args[2]);

    FILE *fin = fopen(src, "rb");
    if (!fin) {
        fprintf(stdout,"No se pudo abrir origen: %s\n", src);
        return 1;
    }

    FILE *fout = fopen(dst, "wb");
    if (!fout) {
        fprintf(stdout,"No se pudo abrir destino: %s\n", dst);
        fclose(fin);
        return 1;
    }

    char buffer[512];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
        fwrite(buffer, 1, n, fout);
    }

    fclose(fin);
    fclose(fout);
    fprintf(stdout,"Copiado: %s -> %s\n", src, dst);

    return 0;
}