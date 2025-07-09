/* xsh_echo.c - xsh_echo */
#include <os.h>
#include <fat_filelib.h>

shellcmd xsh_ls(int nargs, char *args[]) {
    FL_DIR dirstat;
    char *tmp = full_path("");
    if (!tmp) return -1;

    if (fl_opendir(tmp, &dirstat)) {
        struct fs_dir_ent dirent;
        while (fl_readdir(&dirstat, &dirent) == 0) {
            if (dirent.is_dir)
                fprintf(stdout,"%s <DIR>\n", dirent.filename);
            else
                fprintf(stdout,"%s [%d bytes]\n", dirent.filename, dirent.size);
        }
        fl_closedir(&dirstat);
    } else {
        fprintf(stdout,"Error abriendo directorio\n");
    }

    return 0;
}