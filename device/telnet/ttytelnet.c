
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
    static bool initialized = FALSE;
    
    // Inicialización global (solo una vez)
    if (!initialized) {
        global_mutex = semcreate(1);
        initialized = TRUE;
    }
    
    // Inicialización del dispositivo específico
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    tty->sem = semcreate(1);
    tty->client_id = -1;
    tty->active = FALSE;
    tty->owner = -1;
    sprintf(tty->name, "TELNET%d", devptr->dvminor + 1);
    
    return OK;
}



devcall telnetopen(struct dentry *devptr, char *name, char *mode) {
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    
    wait(tty->sem);
    
    // Si ya está abierto y activo, devolver error
    if (tty->active) {
        signal(tty->sem);
        return SYSERR;
    }
    
    // Aceptar nueva conexión
    int client_id = telnet_accept();
    if (client_id < 0) {
        signal(tty->sem);
        return SYSERR;
    }
    
    tty->client_id = client_id;
    tty->active = TRUE;
    tty->owner = getpid();
    
    // Configurar el CSR del dispositivo con el client_id
    devptr->pid = /*(void *)(intptr_t)*/client_id;
    
    signal(tty->sem);
    return OK;
}

devcall telnetclose(struct dentry *devptr) {
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    
    wait(tty->sem);
    
    if (!tty->active || tty->owner != getpid()) {
        signal(tty->sem);
        return SYSERR;
    }
    
    // Cerrar conexión
    if (telnet_close(tty->client_id) < 0) {
        signal(tty->sem);
        return SYSERR;
    }
    
    tty->client_id = -1;
    tty->active = FALSE;
    tty->owner = -1;
    
    signal(tty->sem);
    return OK;
}

devcall telnetread(struct dentry *devptr, void *buf, uint32 len) {
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    
    wait(tty->sem);
    
    if (!tty->active || tty->owner != getpid()) {
        signal(tty->sem);
        return SYSERR;
    }
    
    int bytes = telnet_recv(tty->client_id, buf, len);
    
    signal(tty->sem);
    return (bytes < 0) ? SYSERR : bytes;
}



devcall telnetwrite(struct dentry *devptr, void *buf, uint32 len) {
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    
    wait(tty->sem);
    
    if (!tty->active || tty->owner != getpid()) {
        signal(tty->sem);
        return SYSERR;
    }
    
    int bytes = telnet_send(tty->client_id, buf, len);
    
    signal(tty->sem);
    return (bytes < 0) ? SYSERR : bytes;
}


devcall telnetcontrol(struct dentry *devptr, int32 func, int32 arg1, int32 arg2) {
    if (devptr->dvminor < 0 || devptr->dvminor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    struct ttytelnet *tty = &ttytel[devptr->dvminor];
    
    wait(tty->sem);
    
    switch (func) {
        case TELNET_CTRL_GET_STATUS:
            signal(tty->sem);
            return tty->active ? 1 : 0;
            
        case TELNET_CTRL_SET_TIMEOUT:
            // Implementar si es necesario
            signal(tty->sem);
            return OK;
            
        default:
            signal(tty->sem);
            return SYSERR;
    }
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