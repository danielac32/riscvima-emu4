

#define MAX_TELNET_DEVICES 6
#define TELNET_CTRL_GET_STATUS 7
#define TELNET_CTRL_SET_TIMEOUT 8
struct ttytelnet {
    sid32 sem;          // Semáforo para sincronización
    int client_id;      // ID del cliente conectado
    bool active;        // Estado de conexión
    char name[16];      // Nombre del dispositivo
    pid32 owner;        // Proceso que abrió el dispositivo
};

extern struct ttytelnet ttytel[];
static sid32 global_mutex;  // Semáforo global para inicialización