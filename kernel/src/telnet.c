#include <os.h>
#include <csr.h>
#include <syscall.h>
#include "disk.h"
#include "fat_filelib.h"
#include <telnet.h>

// Funciones de acceso desde Xinu


int telnet_start() {
    return __syscall(0xcafe0009); // TELNET_ACCEPT
}

int telnet_accept() {
    return __syscall(0xcafe0010); // TELNET_ACCEPT
}

int telnet_send(int client_id, char *buffer, int len) {
    return __syscall(0xcafe0011, client_id, buffer, len);
}

int telnet_recv(int client_id, char *buffer, int len) {
    return __syscall(0xcafe0012, client_id, buffer, len);
}
int telnet_close(int client_id) {
    return __syscall(0xcafe0013, client_id, 0, 0, 0, 0, 0);
}


#define MAX_TELNET_CLIENTS 5
#define BUFFER_SIZE 128
/*
int telnet(int nargs, char *args[]) {
    printf("Esperando clientes...\n");

    while (1) {
        int client_id = telnet_accept();
        if (client_id >= 0) {
            printf("Cliente [%d] conectado\n", client_id);

            char rx_buffer[128];
            while (1) {
                int bytes_read = telnet_recv(client_id, rx_buffer, sizeof(rx_buffer));
                if (bytes_read > 0) {
                    rx_buffer[bytes_read] = '\0';
                    printf("Cliente[%d]: %s", client_id, rx_buffer);
                    telnet_send(client_id, rx_buffer, bytes_read);
                } else if (bytes_read == 0) {
                    printf("Cliente[%d] se desconectó\n", client_id);
                    break;
                }
            }
        }
    }
    return OK;
}

*/


  
/*
int telnet(int nargs, char *args[]) {
    printf("Iniciando servidor Telnet...\n");

    while (TRUE) {
        int client_id = telnet_accept();
        if (client_id >= 0) {
            printf("Cliente [%d] conectado\n", client_id);

            char rx_buffer[128];
            while (TRUE) {
                int bytes_read = telnet_recv(client_id, rx_buffer, sizeof(rx_buffer));
                if (bytes_read > 0) {
                    rx_buffer[bytes_read] = '\0';
                    printf("Cliente[%d]: %s", client_id, rx_buffer);

                    // Detectar comando 'exit'
                    if (strncmp(rx_buffer, "exit", 4) == 0) {
                        printf("Cerrando cliente [%d]\n", client_id);
                        break; // Salir del bucle interno
                    }

                    // Reenviar texto recibido
                    telnet_send(client_id, rx_buffer, bytes_read);
                } else if (bytes_read == 0) {
                    printf("Cliente[%d] desconectado\n", client_id);
                    break;
                }
            }

            // Notificar al host que el cliente se cerró
           telnet_close(client_id);  // Nueva syscall: TELNET_CLOSE
        }
    }

    return OK;
}
*/

/*
#define MAX_CLIENTS 5
#define TELNET_STACKSIZE     1024
#define TELNET_BUFFER_SIZE   128

struct telnet_client {
     int active;                 // Flag de cliente activo (1 = activo, 0 = inactivo)
    int client_id;              // ID del socket del cliente
    char buffer[TELNET_BUFFER_SIZE];
};

struct telnet_client clients[MAX_CLIENTS] = {0};



 

int telnet(int nargs, char *args[]) {
    printf("Xinu Telnet Server\n");
    
    // Iniciar servidor en ESP32
    if (telnet_start() <= 0) {
        printf("Failed to start telnet server\n");
        return SYSERR;
    }
    
    while (1) {
        // 1. Aceptar nuevos clientes
        int new_id = telnet_accept();
        if (new_id >= 0 && new_id < MAX_CLIENTS) {
            printf("New client connected: %d\n", new_id);
            clients[new_id].active = 1;
            telnet_send(new_id, "Xinu Telnet>\r\n", 14);
        }
        
        // 2. Manejar clientes existentes
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) continue;
            
            int bytes = telnet_recv(i, clients[i].buffer, sizeof(clients[i].buffer)-1);
            if (bytes > 0) {
                clients[i].buffer[bytes] = '\0';
                printf("Client %d: %s", i, clients[i].buffer);
                
                // Ejemplo: comando "help"
                if (strstr(clients[i].buffer, "help")) {
                    telnet_send(i, "Available commands: help, exit\r\n> ", 32);
                } 
                // Comando "exit"
                else if (strstr(clients[i].buffer, "exit")) {
                    telnet_send(i, "Goodbye!\r\n", 10);
                    telnet_close(i);
                    clients[i].active = 0;
                } 
                // Echo por defecto
                else {
                    telnet_send(i, clients[i].buffer, bytes);
                }
            } 
            else if (bytes == 0) {
                printf("Client %d disconnected\n", i);
                telnet_close(i);
                clients[i].active = 0;
            }
        }
    }
    
    return OK;
}*/







#define MAX_CLIENTS 20
#define TELNET_STACKSIZE 4096
#define TELNET_BUFFER_SIZE 256
#define TELNET_TIMEOUT_MS 100

struct telnet_client {
    int client_id;
    bool active;
    char buffer[TELNET_BUFFER_SIZE];
};

static struct telnet_client clients[MAX_CLIENTS];
static sid32 global_mutex; // Un único semáforo global para simplificar

void client_task(int nargs, char *args[]) {
    // Obtener el slot de los argumentos

    /*for (int i = 0; i < nargs; ++i)
    {
    	printf("%s\n", args[i]);
    }*/
    if (nargs < 1 || args[0] == NULL) {
        printf("ERROR: Missing slot argument\n");
        return;
    }
    
    int slot = atoi(args[0]);
    if (slot < 0 || slot >= MAX_CLIENTS) {
        printf("ERROR: Invalid slot %d\n", slot);
        return;
    }

    // Esperar inicialización completa
    int client_id = -1;
    while (1) {
        wait(global_mutex);
        client_id = clients[slot].client_id;
        if (client_id >= 0 && clients[slot].active) {
            signal(global_mutex);
            break;
        }
        signal(global_mutex);
    }

    printf("Task %d started for client %d (slot %d)\n", getpid(), client_id, slot);

    // Bucle principal del cliente
    while (1) {
        wait(global_mutex);
        if (!clients[slot].active) {
            signal(global_mutex);
            break;
        }
        
        int bytes = telnet_recv(client_id, clients[slot].buffer, 
                             sizeof(clients[slot].buffer)-1);
        
        if (bytes > 0) {
            clients[slot].buffer[bytes] = '\0';
            printf("Client %d: %s", client_id, clients[slot].buffer);
            
            if (strstr(clients[slot].buffer, "help")) {
                telnet_send(client_id, "Commands: help, exit\r\n> ", 24);
            }
            else if (strstr(clients[slot].buffer, "exit")) {
                telnet_send(client_id, "Goodbye!\r\n", 10);
                signal(global_mutex);
                break;
            }
            else {
                telnet_send(client_id, clients[slot].buffer, bytes);
            }
        }
        /*else if (bytes <= 0) {
            printf("Client %d disconnected\n", client_id);
            signal(global_mutex);
            break;
        }*/
        signal(global_mutex);
    }

    // Limpieza
    wait(global_mutex);
    clients[slot].active = FALSE;
    telnet_close(client_id);
    printf("Task for client %d ended\n", client_id);
    signal(global_mutex);
}



int telnet(int nargs, char *args[]) {
    printf("Xinu Telnet Server (Ultimate Version)\n");

    // Inicialización
    global_mutex = semcreate(1);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].client_id = -1;
        clients[i].active = FALSE;
    }

    // Iniciar servidor
    wait(global_mutex);
    int server_status = telnet_start();
    signal(global_mutex);
    
    if (server_status <= 0) {
        printf("ERROR: Server start failed (%d)\n", server_status);
        semdelete(global_mutex);
        return SYSERR;
    }

    while (1) {
        //wait(global_mutex);
        int new_id = telnet_accept();
        //signal(global_mutex);

        if (new_id >= 0) {
            //wait(global_mutex);
            
            // Buscar slot libre
            int slot = -1;
            struct telnet_client *current;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i].active) {
                    slot = i;
                    clients[i].client_id = new_id;
                    clients[i].active = TRUE;
                    current =&clients[i];
                    break;
                }
            }
            //signal(global_mutex);

            if (slot == -1) {
                printf("Rejecting client %d (no slots)\n", new_id);
                //wait(global_mutex);
                telnet_close(new_id);
                //signal(global_mutex);
                continue;
            }

            // Crear tarea
            char task_name[16];
            char num[10];
            printf("client status\n");
            printf("active: %d slot: %d\n", current->active,slot);
            sprintf(task_name, "telnet_%d", slot);
            sprintf(num, "%d", slot);
            pid32 task = create((void *)client_task,
                              TELNET_STACKSIZE,
                              20+(slot*2),  // Alta prioridad
                              task_name,
                              1,
                              num);
            if (task == SYSERR) {
                printf("ERROR: pid(%d) Cannot create task for slot %d\n", task,slot);
                //wait(global_mutex);
                clients[slot].active = FALSE;
                telnet_close(new_id);
                continue;
                //signal(global_mutex);
            } else {
                resume(task);
                printf("Created task %d for client %d (slot %d)\n", 
                      task, new_id, slot);
            }
        }
    }

    //semdelete(global_mutex);
    return OK;
}


int telnet2(int nargs, char *args[]) {

    // Abrir conexión TELNET1
    int dev = open(TELNET1, NULL, NULL);
    if (dev == SYSERR) {
        printf("Failed to open TELNET1\n");
        return -1;
    }
    
    // Escribir mensaje de bienvenida
    char *welcome = "Welcome to Xinu TELNET!\r\n> ";
    write(dev, welcome, strlen(welcome));
    
    // Leer comandos
    char buffer[128];
    while (1) {
        int n = read(dev, buffer, sizeof(buffer)-1);
        if (n <= 0) break;
        
        buffer[n] = '\0';
        
        if (strstr(buffer, "exit")) {
            write(dev, "Goodbye!\r\n", 10);
            break;
        }
        
        // Eco del comando
        write(dev, buffer, n);
        write(dev, "> ", 2);
    }
    
    close(dev);
    return OK;
}

 