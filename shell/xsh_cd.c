/* xsh_echo.c - xsh_echo */


#include <os.h>
 #include <fat_filelib.h>


shellcmd xsh_cd(int nargs, char *args[]){
FL_DIR dirstat;
 char *s=args[1];
   if (strcmp(s, ".") == 0){
       goto cd_done;
   }else if (strcmp(s, "..") == 0) {
      if (strcmp(curdir, "/") == 0) {
          fprintf(stdout,"not a directory\n");
          //goto cd_done;//
          return 0;
      }
      strcpy(path, curdir);
      char* cp = strrchr(path, '/');
        if (cp == NULL)
            cp = curdir;
        *cp = 0;
        cp = strrchr(path, '/');
        if (cp != NULL)
            *(cp + 1) = 0;
        goto cd_done;
   }
   full_path(s);

   if(!fl_opendir(path,&dirstat)){
       fprintf(stdout,"%s not found!\n",path );
       strcpy(path, curdir);
       return 0;
   }
   fl_closedir(&dirstat);
   //fprintf(stdout,"%s\n",path);
    
   cd_done:
    strcpy(curdir, path);
    if (curdir[strlen(curdir) - 1] != '/'){
        strcat(curdir, "/");
    }
    return 0;
}
