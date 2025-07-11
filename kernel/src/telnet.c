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



static int assign_client_to_device(int client_id);
 

void telnet_client_task(int nargs, char *args[]);


void telnet2(int nargs, char *args[]) {
    // Inicializar estructuras
    server_mutex = semcreate(1);
    
    /*for (int i = 0; i < MAX_TELNET_DEVICES; i++) {
        ttytel[i].sem = semcreate(1);
        ttytel[i].client_id = -1;
        ttytel[i].active = FALSE;
        ttytel[i].owner = -1;
        ttytel[i].assigned = FALSE;
    }*/

    // Inicializar solo dispositivos TELNET
    for (int i = 0; i < NDEVS; i++) {
        if (strncmp(devtab[i].dvname, "TELNET", 6) == 0) {
            int index = devtab[i].dvminor;
            if (index >= 0 && index < MAX_TELNET_DEVICES) {
                ttytel[index].sem = semcreate(1);
                ttytel[index].client_id = -1;
                ttytel[index].active = FALSE;
                ttytel[index].owner = -1;
                ttytel[index].assigned = FALSE;
            }
        }
    }
    
    // Iniciar servidor TELNET
    if (telnet_start() <= 0) {
        printf("ERROR: Failed to start TELNET server\n");
        return;
    }
    
    printf("TELNET server started, waiting for connections...\n");
    
    while (1) {
        int client_id = telnet_accept();
        if (client_id >= 0) {
            printf("New client connected: %d\n", client_id);
            
            // Asignar a un dispositivo (devuelve número de dispositivo real)
            int devnum = assign_client_to_device(client_id);
            
            if (devnum < 0) {
                printf("No available devices, rejecting client %d\n", client_id);
                telnet_close(client_id);
                continue;
            }
            
            // Obtener nombre del dispositivo
            char* devname = devtab[devnum].dvname;
            printf("Client %d assigned to %s\n", client_id, devname);
            
            // Crear tarea para este cliente
            char task_name[16];
            sprintf(task_name, "Client_%s", devname);
            
            char devnum_str[4];
            sprintf(devnum_str, "%d", devnum);
            
            pid32 task = create((void *)telnet_client_task,
                              4096,
                              40,
                              task_name,
                              1,
                              devnum_str);
            
            if (task != SYSERR) {
                resume(task);
               // printf("Created task %d for %s\n", task, devname);
            } else {
                printf("ERROR: Could not create task for %s\n", devname);
                // Liberar el dispositivo
                int index = devtab[devnum].dvminor;
                wait(ttytel[index].sem);
                ttytel[index].assigned = FALSE;
                ttytel[index].active = FALSE;
                ttytel[index].client_id = -1;
                signal(ttytel[index].sem);
                telnet_close(client_id);
            }
        }
    }
}





int assign_client_to_device(int client_id) {
    wait(server_mutex);
    
    // Buscar por número de dispositivo (no por índice)
    for (int i = 0; i < NDEVS; i++) {
        // Solo considerar dispositivos TELNET
        if (strncmp(devtab[i].dvname, "TELNET", 6) != 0) {
            continue;
        }
        
        // Calcular índice basado en dvminor
        int index = devtab[i].dvminor;
        if (index < 0 || index >= MAX_TELNET_DEVICES) {
            continue;
        }
        
        wait(ttytel[index].sem);
        
        if (!ttytel[index].assigned) {
            ttytel[index].client_id = client_id;
            ttytel[index].assigned = TRUE;
            ttytel[index].active = TRUE;
            
            signal(ttytel[index].sem);
            signal(server_mutex);
            return i; // Devuelve el número de dispositivo real
        }
        
        signal(ttytel[index].sem);
    }
    
    signal(server_mutex);
    return -1; // No hay dispositivos disponibles
}

void telnet_client_task(int nargs, char *args[]) {
    // Verificar que el dispositivo es válido
    int devnum = atoi(args[0]);
    //printf("dev: %d\n",devnum );
    if (devnum < 0 || devnum >= NDEVS || 
        strncmp(devtab[devnum].dvname, "TELNET", 6) != 0) {
        printf("ERROR: Invalid device number %d\n", devnum);
        return;
    }
    
    int index = devtab[devnum].dvminor;
    char* devname = devtab[devnum].dvname;
    char buff[256];

    // Abrir el dispositivo
    if (open(devnum, NULL, NULL) == SYSERR) {
        printf("ERROR: Could not open %s\n", devname);
        return;
    }
    
    // Mensaje de bienvenida
    //char welcome[64];
    //sprintf(welcome, "\r\nConnected to %s\r\n> ", devname);
    //write(devnum, welcome, strlen(welcome));
    fprintf(devnum,"\r\nConnected to %s\r\n> ",devname);
    // Bucle principal de la tarea del cliente
    while (1) {
        /*wait(ttytel[index].sem);
        if (!ttytel[index].active) {
            signal(ttytel[index].sem);
            break;
        }*/
        
       // wait(ttytel[index].sem);

        if (control(devnum, TELNET_CTRL_GET_STATUS, 0, 0) != 1) {
            printf("%s: Connection lost\n", devname);
            break;
        }
        // Leer comando del cliente
        int n = read(devnum, buff, sizeof(buff) - 1);
        
        if(n>0){
            buff[n] = '\0';
            printf("%s received: %s", devname, buff);
            
            // Procesar comando
            if (strstr(buff, "help")) {
                //char help_msg[] = "Available commands:\r\nhelp - Show this help\r\nexit - Disconnect\r\n> ";
                //write(devnum, help_msg, sizeof(help_msg) - 1);
                fprintf(devnum,"Available commands:\r\nhelp - Show this help\r\nexit - Disconnect\r\n> ");
            }
            else if (strstr(buff, "exit")) {
                //char goodbye[] = "Goodbye!\r\n";
                //write(devnum, goodbye, sizeof(goodbye) - 1);
                fprintf(devnum,"Goodbye\n");
                break;
            }
            else {
                fprintf(devnum, "You wrote %s\n", buff);
                // Eco del mensaje
                //write(devnum, "You wrote: ", 11);
                //write(devnum, ttytel[index].input_buffer, n);
                //write(devnum, "\r\n> ", 4);
            }
        }else if (n == -2) {
            printf("Client disconnected from %s\n", devname);
            break;
        }
    }
    
    // Limpieza antes de terminar
    close(devnum);
    
    wait(ttytel[index].sem);
    if (ttytel[index].active) {
        telnet_close(ttytel[index].client_id);
        ttytel[index].active = FALSE;
        ttytel[index].assigned = FALSE;
        ttytel[index].client_id = -1;
    }
    signal(ttytel[index].sem);
    
    printf("%s client task ending\n", devname);
}


 