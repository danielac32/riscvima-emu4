// Copyright 2022 Charles Lohr, you may use this file or any portions herein under any of the BSD, MIT, or CC0 licenses.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

//#include "default64mbdtc.h"

// Just default RAM amount is 64MB.
int fail_on_all_faults = 0;

Memory mem;

FILE *disk_file = NULL;
typedef struct{
	unsigned char *mem;
	unsigned int size;
}Disk;

Disk VirtualDisk;





static uint64_t GetTimeMicroseconds();
static void ResetKeyboardInput();
static void CaptureKeyboardInput();
static uint32_t HandleException( uint32_t ir, uint32_t retval );
static uint32_t HandleControlStore( uint32_t addy, uint32_t val );
static uint32_t HandleControlLoad( uint32_t addy );
static void HandleOtherCSRWrite( uint8_t * image, uint16_t csrno, uint32_t value );
static int32_t HandleOtherCSRRead( uint8_t * image, uint16_t csrno);
static uint32_t HandleOtherReturnSyscall( uint16_t csrno,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4,uint32_t a5);
static void MiniSleep();
static int IsKBHit();
static int ReadKBByte();

// This is the functionality we want to override in the emulator.
//  think of this as the way the emulator's processor is connected to the outside world.
#define MINIRV32WARN( x... ) printf( x );
#define MINI_RV32_RAM_SIZE DRAM_SIZE
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_POSTEXEC( pc, ir, retval ) { if( retval > 0 ) { if( fail_on_all_faults ) { printf( "FAULT\n" ); return 3; } else retval = HandleException( ir, retval ); } }
#define MINIRV32_HANDLE_MEM_STORE_CONTROL( addy, val ) if( HandleControlStore( addy, val ) ) return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL( addy, rval ) rval = HandleControlLoad( addy );
#define MINIRV32_OTHERCSR_WRITE( csrno, value ) HandleOtherCSRWrite( image, csrno, value );
#define MINIRV32_OTHERCSR_READ( csrno, value ) value = HandleOtherCSRRead( image, csrno);
#define MINIRV32_RETURNSYSCALL_HOST(csrno, a0,a1,a2,a3,a4,a5,value) value = HandleOtherReturnSyscall(csrno,a0,a1,a2,a3,a4,a5);



#define MINIRV32_CUSTOM_MEMORY_BUS


static inline bool check_memory_bounds(uint32_t ofs, uint32_t size)
{
    return (ofs + size <= MINI_RV32_RAM_SIZE);
}


static void MINIRV32_STORE4(uint32_t ofs, uint32_t val) {
	if (check_memory_bounds(ofs, 4)) {
	*((uint32_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,4);
}

}

static void MINIRV32_STORE2(uint32_t ofs, uint16_t val) {
    if (check_memory_bounds(ofs, 2)) {
    *((uint16_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,2);
}
}

static void MINIRV32_STORE1(uint32_t ofs, uint8_t val) {
    if (check_memory_bounds(ofs, 1)) {
	    *((uint8_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,1);
	}
}

static uint32_t MINIRV32_LOAD4(uint32_t ofs) {
    uint32_t val = 0;
    if (check_memory_bounds(ofs, 4)) {
	    val = *((uint32_t *)(mem.p + ofs));//cache_read(ofs,&val,4);
	}
    return val;
}

static uint16_t MINIRV32_LOAD2(uint32_t ofs) {
    uint16_t val = 0;
    if (check_memory_bounds(ofs, 2)) {
	    val = *((uint16_t *)(mem.p + ofs));//cache_read(ofs,&val,2);
	}
    return val;
}

static uint8_t MINIRV32_LOAD1(uint32_t ofs) {
	uint8_t val = 0;
    if (check_memory_bounds(ofs, 1)) {
    val = *((uint8_t *)(mem.p + ofs));//cache_read(ofs,&val,1);
}
    return val;
}
#include "mini-rv32ima.h"

struct MiniRV32IMAState core;
 

static void DumpState( struct MiniRV32IMAState * core/*, uint8_t * ram_image */);

int main( int argc, char ** argv )
{
    /*disk_file = fopen("disk.img", "r+b");
    if (!disk_file) {
        printf("No se encontró 'disk.img', creando uno nuevo...\n");
        disk_file = fopen("disk.img", "w+b");
        if (!disk_file) {
            perror("No se pudo crear disk.img");
            return 1;
        }
        // Crear un archivo de 64MB lleno de ceros
        const size_t DISK_SIZE = 64 * 1024 * 1024; // 64MB
        fseek(disk_file, DISK_SIZE - 1, SEEK_SET);
        fputc('\0', disk_file); // Escribir un byte para extender el archivo
        rewind(disk_file);
    } else {
        printf("Cargado 'disk.img' existente.\n");
    }
    fclose(disk_file);*/

    const size_t DISK_SIZE = 64 * 1024 * 1024; // 64MB
    VirtualDisk.mem=malloc(DISK_SIZE);
    VirtualDisk.size=DISK_SIZE;
    memset(VirtualDisk.mem,0,DISK_SIZE);

restart:
	{
        mem = create_memory(argv[1]);
	}
	core.regs[10] = 0x00; // hart ID
	core.regs[11] = 0;
	core.extraflags |= 3; // Machine-mode.

	core.pc = MINIRV32_RAM_IMAGE_OFFSET;
 
	uint64_t lastTime = GetTimeMicroseconds();
	int instrs_per_flip = 1024;
	printf("RV32IMA starting\n");
	int c;


	while(1){
	  	int ret;
		uint64_t *this_ccount = ((uint64_t*)&core.cyclel);
		uint32_t elapsedUs = GetTimeMicroseconds() / lastTime;
		lastTime += elapsedUs;

	  	ret = MiniRV32IMAStep( &core, NULL, 0, elapsedUs, instrs_per_flip ); // Execute upto 1024 cycles before breaking out.
        switch( ret )
		{
			case 0: break;
			case 1: 
				 *this_ccount += instrs_per_flip;
			break;
			case 3:  break;
			case 0x7777: goto restart;	//syscon code for restart
			case 0x5555: printf( "POWEROFF@0x%08x%08x\n", core.cycleh, core.cyclel ); return 0; //syscon code for power-off
			default: printf( "Unknown failure %d\n",ret ); break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Platform-specific functionality
//////////////////////////////////////////////////////////////////////////



#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
 
 

static uint64_t GetTimeMicroseconds()
{
	struct timeval tv;
	gettimeofday( &tv, 0 );
	return tv.tv_usec + ((uint64_t)(tv.tv_sec)) * 1000000LL;
}

static int is_eofd;

static int ReadKBByte()
{
	if( is_eofd ) return 0xffffffff;
	char rxchar = 0;
	int rread = read(fileno(stdin), (char*)&rxchar, 1);

	if( rread > 0 ) // Tricky: getchar can't be used with arrow keys.
		return rxchar;
	else
		return -1;
}

static int IsKBHit()
{
	if( is_eofd ) return -1;
	int byteswaiting;
	ioctl(0, FIONREAD, &byteswaiting);
	if( !byteswaiting && write( fileno(stdin), 0, 0 ) != 0 ) { is_eofd = 1; return -1; } // Is end-of-file for 
	return !!byteswaiting;
}





//////////////////////////////////////////////////////////////////////////
// Rest of functions functionality
//////////////////////////////////////////////////////////////////////////

static uint32_t HandleException( uint32_t ir, uint32_t code )
{
	// Weird opcode emitted by duktape on exit.
	if( code == 3 )
	{
		// Could handle other opcodes here.
	}
	return code;
}

static uint32_t HandleControlStore( uint32_t addy, uint32_t val )
{
	if( addy == 0x10000000 ) //UART 8250 / 16550 Data Buffer
	{
		printf( "%c", val );
		fflush( stdout );
	}
	return 0;
}


static uint32_t HandleControlLoad( uint32_t addy )
{
	// Emulating a 8250 / 16550 UART
	if( addy == 0x10000005 )
		return IsKBHit();
	else if( addy == 0x10000000 && IsKBHit() )
		return ReadKBByte();
	else if(addy==0x60000000){
		return 67;
	}
	return 0;
}

static void HandleOtherCSRWrite( uint8_t * image, uint16_t csrno, uint32_t value )
{

	if( csrno == 0x402 ){
		printf( "%c", value ); fflush( stdout );
	}else if( csrno == 0x136 )
	{
		printf( "%d", value ); fflush( stdout );
	}else if( csrno == 0x137 )
	{
		printf( "%08x", value ); fflush( stdout );
	}
	else if( csrno == 0x139 )
	{
		putchar( value ); fflush( stdout );
	}
}



static int32_t HandleOtherCSRRead( uint8_t * image, uint16_t csrno )
{
	char c;
    if( csrno == 0x141 )
	{  
		c=IsKBHit();
		return c;
	}else
	if( csrno == 0x140 )
	{  
		if( !IsKBHit() ) return -1;
		return ReadKBByte();
	}
	return 0;
}


enum {
    DISK_INIT = 0x0001,
    DISK_WRITE = 0x0002,
    DISK_READ = 0x0003,
    TELNET_START = 0x0009,
    TELNET_ACCEPT = 0x0010,
    TELNET_SEND = 0x0011,
    TELNET_RECV = 0x0012,
    TELNET_CLOSE = 0x0013,

};
#define MAX_CLIENTS 5

int telnet_sock = -1;
int client_socks[MAX_CLIENTS] = {0};




static uint32_t HandleOtherReturnSyscall(uint16_t csrno, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) {
    switch (csrno) {


        case TELNET_START: {
            if (telnet_sock != -1) return 1;
            
            telnet_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (telnet_sock < 0) {
                perror("socket() failed");
                return -1;
            }
            
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(23);
            addr.sin_addr.s_addr = INADDR_ANY;
            
            int opt = 1;
            setsockopt(telnet_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            
            if (bind(telnet_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("bind() failed");
                close(telnet_sock);
                telnet_sock = -1;
                return -1;
            }
            
            if (listen(telnet_sock, MAX_CLIENTS) < 0) {
                perror("listen() failed");
                close(telnet_sock);
                telnet_sock = -1;
                return -1;
            }
            
            fcntl(telnet_sock, F_SETFL, O_NONBLOCK);
          //  printf("TELNET server started on port 23\n");
            return 1;
        }
            
        case TELNET_ACCEPT: {
            if (telnet_sock == -1) return -1;
            
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int fd = accept(telnet_sock, (struct sockaddr*)&client_addr, &len);
            
            if (fd >= 0) {
                fcntl(fd, F_SETFL, O_NONBLOCK);
                
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_socks[i] == 0) {
                        client_socks[i] = fd;
                        
                        //const char *banner = "\r\nBienvenido al servidor Xinu (PC)\r\n$ ";
                        //send(fd, banner, strlen(banner), 0);
                        
                        //printf("New TELNET client (ID=%d)\n", i);
                        return i;
                    }
                }
                close(fd);
            }
            return -1;
        }
            
        case TELNET_SEND: {
            int id = a0;
            if (id < 0 || id >= MAX_CLIENTS || client_socks[id] == 0) return -1;
            
            // a1 es el puntero al buffer en memoria del emulado
            const char* buf = (const char*)(mem.p + (a1 - MINIRV32_RAM_IMAGE_OFFSET));
            size_t len = a2;
            
            return send(client_socks[id], buf, len, 0);
        }
        
        case TELNET_RECV: {
            int id = a0;
            if (id < 0 || id >= MAX_CLIENTS || client_socks[id] == 0) return -1;
            
            // a1 es el puntero al buffer de destino
            char* buf = (char*)(mem.p + (a1 - MINIRV32_RAM_IMAGE_OFFSET));
            size_t max_len = a2;
            
            memset(buf, 0, max_len);
            int received = recv(client_socks[id], buf, max_len, MSG_DONTWAIT);
            
            if (received == 0) {
                printf("Client %d disconnected\n", id);
                close(client_socks[id]);
                client_socks[id] = 0;
                return -2;
            }
            
            return received;
        }
        
        case TELNET_CLOSE: {
            int id = a0;
            if (id < 0 || id >= MAX_CLIENTS || client_socks[id] == 0) return -1;
            
           // printf("Closing client %d\n", id);
            close(client_socks[id]);
            client_socks[id] = 0;
            return 0;
        }



         case DISK_INIT:
         	{
         	/*disk_file = fopen("disk.img", "rb");
            fseek(disk_file, 0, SEEK_END);
            long size = ftell(disk_file);
            //rewind(disk_file);
            fclose(disk_file);
            return (uint32_t)size; */
         		return VirtualDisk.size;
         	}
         case DISK_READ: {
    // a0 = buffer (destino en RAM emulada)
    // a1 = start_block (sector LBA)
    // a2 = sector_count (siempre 512 en tu caso)
    
    uint32_t ram_addr = a0 - MINIRV32_RAM_IMAGE_OFFSET;
    uint32_t disk_offset = a1 * 512; // Convertir LBA a offset de bytes
    
    // Verificación de límites
    if (disk_offset + 512 > VirtualDisk.size) {
        return 0; // Error: fuera de límites
    }
    
    // Copia directa sector a RAM emulada
    for (int i = 0; i < 512; i++) {
        MINIRV32_STORE1(ram_addr + i, VirtualDisk.mem[disk_offset + i]);
    }
    
    return 1; // Éxito
}

case DISK_WRITE: {
    // a0 = buffer (origen en RAM emulada)
    // a1 = start_block (sector LBA)
    // a2 = sector_count (siempre 512)
    
    uint32_t ram_addr = a0 - MINIRV32_RAM_IMAGE_OFFSET;
    uint32_t disk_offset = a1 * 512;
    
    if (disk_offset + 512 > VirtualDisk.size) {
        return 0; // Error: fuera de límites
    }
    
    // Copia directa RAM emulada a sector
    for (int i = 0; i < 512; i++) {
        VirtualDisk.mem[disk_offset + i] = MINIRV32_LOAD1(ram_addr + i);
    }
    
    return 1; // Éxito
}
    }
}
static void DumpState( struct MiniRV32IMAState * core/*, uint8_t * ram_image */)
{
	uint32_t pc = core->pc;
	uint32_t pc_offset = pc - MINIRV32_RAM_IMAGE_OFFSET;
	uint32_t ir = 0;

	printf( "PC: %08x ", pc );
	if( pc_offset >= 0 && pc_offset < DRAM_SIZE - 3 )
	{
		//ir = *((uint32_t*)(&((uint8_t*)ram_image)[pc_offset]));
		//printf( "[0x%08x] ", ir ); 
	}
	else
		printf( "[xxxxxxxxxx] " ); 
	uint32_t * regs = core->regs;
	printf( "Z:%08x ra:%08x sp:%08x gp:%08x tp:%08x t0:%08x t1:%08x t2:%08x s0:%08x s1:%08x a0:%08x a1:%08x a2:%08x a3:%08x a4:%08x a5:%08x ",
		regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7],
		regs[8], regs[9], regs[10], regs[11], regs[12], regs[13], regs[14], regs[15] );
	printf( "a6:%08x a7:%08x s2:%08x s3:%08x s4:%08x s5:%08x s6:%08x s7:%08x s8:%08x s9:%08x s10:%08x s11:%08x t3:%08x t4:%08x t5:%08x t6:%08x\n",
		regs[16], regs[17], regs[18], regs[19], regs[20], regs[21], regs[22], regs[23],
		regs[24], regs[25], regs[26], regs[27], regs[28], regs[29], regs[30], regs[31] );
}

