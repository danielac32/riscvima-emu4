
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
    if (ttytel[minor].owner != -1 && ttytel[minor].owner != getpid()) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }
    
    ttytel[minor].owner = getpid();
    signal(ttytel[minor].sem);
    
    return OK;
}

devcall telnetclose(struct dentry *devptr) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    if (ttytel[minor].owner != getpid()) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }
    
    // No cerramos la conexión, solo liberamos el ownership
    ttytel[minor].owner = -1;
    
    signal(ttytel[minor].sem);
    return OK;
}


devcall telnetread(struct dentry *devptr, void *buf, uint32 len) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    if (ttytel[minor].owner != getpid() || !ttytel[minor].active) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }
    
    int bytes = telnet_recv(ttytel[minor].client_id, buf, len);
    
    signal(ttytel[minor].sem);
    return (bytes < 0) ? SYSERR : bytes;
}



devcall telnetwrite(struct dentry *devptr, void *buf, uint32 len) {
    int minor = devptr->dvminor;
    if (minor < 0 || minor >= MAX_TELNET_DEVICES) {
        return SYSERR;
    }
    
    wait(ttytel[minor].sem);
    
    if (ttytel[minor].owner != getpid() || !ttytel[minor].active) {
        signal(ttytel[minor].sem);
        return SYSERR;
    }
    
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
            signal(tty->sem);
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