/* xsh_tnetstat.c - comando para mostrar estado de conexiones TELNET */

#include <os.h>
#include <telnet.h>

/*------------------------------------------------------------------------
 * xsh_tnetstat - muestra información detallada de clientes TELNET y shells
 *------------------------------------------------------------------------
 */
shellcmd xsh_tnetstat(int nargs, char *args[]) {
    struct procent *prptr;  // Puntero a entrada de proceso
    struct ttytelnet *tty;  // Puntero a estructura TELNET
    int i, j;
    bool header_printed = FALSE;
    int total_shell=0;

    /* Ayuda del comando */
    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
        fprintf(stdout, "Uso: %s\n\n", args[0]);
        fprintf(stdout, "Descripción:\n");
        fprintf(stdout, "\tMuestra información detallada de conexiones TELNET\n");
        fprintf(stdout, "\ty shells asociados\n");
        fprintf(stdout, "Opciones:\n");
        fprintf(stdout, "\t--help\t muestra esta ayuda y sale\n");
        return 0;
    }

    /* Verificar argumentos */
    if (nargs > 1) {
        fprintf(stdout, "%s: demasiados argumentos\n", args[0]);
        fprintf(stdout, "Pruebe '%s --help' para más información\n", args[0]);
        return 1;
    }

    /* Encabezado para dispositivos TELNET */
    fprintf(stdout, "\nDispositivos TELNET:\n");
    fprintf(stdout, "%-10s %-8s %-8s %-8s %-12s %-12s\n",
           "Dispositivo", "Minor", "Activo", "Asignado", "Client ID", "Owner PID");
    fprintf(stdout, "%-10s %-8s %-8s %-8s %-12s %-12s\n",
           "----------", "------", "------", "--------", "---------", "---------");

    /* Mostrar información de cada dispositivo TELNET */
    for (i = 0; i < MAX_TELNET_DEVICES; i++) {
        tty = &ttytel[i];
        
        /* Obtener nombre del dispositivo */
        char devname[20];
        sprintf(devname, "/dev/telnet%d", i+1);
        
        fprintf(stdout, "%-10s %-8d %-8s %-8s %-12d %-12d\n",
               devname, i,
               tty->active ? "Sí" : "No",
               tty->assigned ? "Sí" : "No",
               tty->client_id,
               tty->owner);
    }

       /* Mostrar solo shells TELNET relevantes */
    fprintf(stdout, "\n%-6s %-16s %-10s %-15s %-12s\n",
           "PID", "NOMBRE", "ESTADO", "DISPOSITIVO", "CLIENTE ID");
    fprintf(stdout, "%-6s %-16s %-10s %-15s %-12s\n",
           "----", "----------------", "----------", "---------------", "-----------");

    for (i = 0; i < NPROC; i++) {
        prptr = &proctab[i];
        
        if (prptr->prstate == PR_FREE) continue;
        
        /* Filtrar mejor los shells TELNET */
        bool is_telnet_shell = FALSE;
        int client_id = -1;
        int dev_minor = -1;
        char devname[20] = "N/A";
        
        // Verificar si es un shell (por nombre)
        if (strstr(prptr->prname, "SHELL") != NULL || 
            strstr(prptr->prname, "telnet") != NULL) {
            
            // Verificar si tiene descriptor TELNET
            for (j = 0; j < MAX_TELNET_DEVICES; j++) {
                if (ttytel[j].owner == i) {
                    is_telnet_shell = TRUE;
                    client_id = ttytel[j].client_id;
                    dev_minor = j;
                    sprintf(devname,  "telnet%d", j+1);
                    break;
                }
            }
            
            // Si no encontró dispositivo, pero parece shell TELNET
            if (!is_telnet_shell && 
                (strstr(prptr->prname, "TELNET") != NULL || 
                 strstr(prptr->prname, "telnet") != NULL)) {
                is_telnet_shell = TRUE;
            }

        }
        
        if (is_telnet_shell) {
            const char *state;
            switch(prptr->prstate) {
                case PR_CURR: state = "ACTIVO"; break;
                case PR_READY: state = "LISTO"; break;
                default: state = "EN BG";
            }
            
            fprintf(stdout, "%-6d %-16s %-10s %-15s %-12d\n",
                   i, prptr->prname, state, devname, client_id);
            
            header_printed = TRUE;
        }
    }

    

    return 0;
}