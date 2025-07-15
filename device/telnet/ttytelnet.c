
#include <os.h>
#include <telnet.h>
sid32 ttytelnetsem; 
extern int telnet_start();
extern int telnet_accept();
extern int telnet_send(int client_id, char *buffer, int len);
extern int telnet_recv(int client_id, char *buffer, int len);
extern int telnet_close(int client_id);


struct ttytelnet ttytel[MAX_TELNET_DEVICES];

devcall telnetinit(struct dentry *devptr) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    ttytel[minor].sem = semcreate(1);
    ttytel[minor].client_id = -1;
    ttytel[minor].active = FALSE;
    ttytel[minor].owner = -1;
    ttytel[minor].assigned = FALSE;
    
    return OK;
}



devcall telnetopen(struct dentry *devptr, char *name, char *mode) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    if (!ttytel[minor].assigned || !ttytel[minor].active) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }
    
    // Solo permitir un proceso dueño a la vez
    /*if (ttytel[minor].owner != -1 && ttytel[minor].owner != getpid()) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }*/

    
    ttytel[minor].owner = getpid();
    signal(ttytel[minor].sem);

    return OK;
}

devcall telnetclose(struct dentry *devptr) {

    int minor = devptr->dvminor;
    //fprintf(CONSOLE,"%d client task ending\n",ttytel[minor].client_id);
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    /*if (ttytel[minor].owner != getpid()) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }*/
    

    if (ttytel[minor].owner == getpid()) {
        telnet_close(ttytel[minor].client_id);
        ttytel[minor].active = FALSE;
        ttytel[minor].assigned = FALSE;
        ttytel[minor].client_id = -1;
        ttytel[minor].owner = -1;
    }

    // No cerramos la conexión, solo liberamos el ownership
    /*ttytel[minor].owner = -1;
    telnet_close(ttytel[minor].client_id);
    ttytel[minor].active = FALSE;
    ttytel[minor].assigned = FALSE;
    ttytel[minor].client_id = -1;*/
    signal(ttytel[minor].sem);
    //semdelete(ttytel[minor].sem);
    return OK;
}


devcall telnetread(struct dentry *devptr, void *buf, uint32 len) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    // Adquirir semáforo con timeout para evitar deadlocks
    if (wait(ttytel[minor].sem) == SYSERR) {
        return SYSERR;
    }
    
    // Verificar ownership y estado
    /*if (ttytel[minor].owner != getpid() || !ttytel[minor].active) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }*/
    
    char *char_buf = (char *)buf;  // Cast a char* para manipulación de bytes
    int bytes = 0;
    int attempts = 0;
    const int max_attempts = 200;  // Máximo de intentos
    
    while (attempts < max_attempts) {
        bytes = telnet_recv(ttytel[minor].client_id, char_buf, len);
        
        // Si recibimos datos, procesarlos
        if (bytes > 0) {
            // Eliminar CR/LF del final si existen
            while (bytes > 0 && (char_buf[bytes-1] == '\n' || char_buf[bytes-1] == '\r')) {
                char_buf[--bytes] = '\0';  // Eliminar y terminar con null
            }
            break;
        }
        
        // Si no hay datos, esperar un poco
        signal(ttytel[minor].sem);  // Liberar durante la espera
        sleepms(10);              // Esperar 100ms
        wait(ttytel[minor].sem);    // Re-adquirir antes de continuar
        attempts++;
    }
    
    signal(ttytel[minor].sem);
    
    // Devolver bytes leídos (0 si timeout, SYSERR si error)
    return (bytes > 0) ? bytes : 
          (attempts >= max_attempts) ? 0 : SYSERR;
}


devcall telnetwrite(struct dentry *devptr, void *buf, uint32 len) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    /*if (ttytel[minor].owner != getpid() || !ttytel[minor].active) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }*/
    
    int bytes = telnet_send(ttytel[minor].client_id, buf, len);
    
    signal(ttytel[minor].sem);
    return (bytes < 0) ? SYSERR : bytes;
}



devcall telnetcontrol(struct dentry *devptr, int32 func, int32 arg1, int32 arg2) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    int ret = SYSERR;
    
    switch (func) {
        case TELNET_CTRL_GET_STATUS:
            ret = (ttytel[minor].active && ttytel[minor].assigned) ? 1 : 0;
            break;
            
        case TELNET_CTRL_GET_CLIENTID:
            ret = ttytel[minor].client_id;
            break;
            
        case TELNET_CTRL_DISCONNECT:
            if (ttytel[minor].active && ttytel[minor].assigned) {
                telnet_close(ttytel[minor].client_id);
                ttytel[minor].active = FALSE;
                ttytel[minor].assigned = FALSE;
                ttytel[minor].client_id = -1;
                ret = OK;
            }
            break;
            
        case TELNET_CTRL_SET_TIMEOUT:
            signal(ttytel->sem);
            ret = OK;
            break;
            
        default:
            ret = SYSERR;
            break;
    }
    
    signal(ttytel[minor].sem);
    return ret;
}

devcall telnetputc(struct dentry *devptr, char c) {
    return telnetwrite(devptr, &c, 1);
}

devcall telnetgetc(struct dentry *devptr) {
    char c;
    if (telnetread(devptr, &c, 1) != 1) {
        return SYSERR;
    }
    return c;
}