

#define MAX_TELNET_DEVICES 6
#define TELNET_CTRL_GET_STATUS   0   // Devuelve 1 si hay cliente conectado, 0 si no
#define TELNET_CTRL_GET_CLIENTID 1   // Devuelve el ID del cliente asignado
#define TELNET_CTRL_DISCONNECT   2   // Desconecta al cliente
#define TELNET_CTRL_SET_TIMEOUT  3   // Establece timeout de lectura


#define IAC  255  // Interpret As Command
#define DONT 254
#define DO   253
#define WONT 252
#define WILL 251
#define SB   250  // Subnegotiation Begin
#define SE   240  // Subnegotiation End
#define TELOPT_ECHO   1
#define TELOPT_SGA    3  // Suppress Go Ahead


struct ttytelnet {
    sid32 sem;          // Semáforo para sincronización
    int client_id;      // ID del cliente conectado
    bool active;        // Estado de conexión
    pid32 owner;        // Proceso que tiene abierto el dispositivo
    bool assigned;      // Si el dispositivo tiene un cliente asignado
    //char input_buffer[256]; // Buffer para entrada
    //char output_buffer[256]; // Buffer para salida
};
extern struct ttytelnet ttytel[];
static sid32 server_mutex;  // Semáforo global para inicialización