// Copyright 2022 Charles Lohr, you may use this file or any portions herein under any of the BSD, MIT, or CC0 licenses.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mem.h>

//#include "default64mbdtc.h"

 
static uint8_t * ram_image;
int fail_on_all_faults = 0;

Memory mem;


static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );
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
#define MINIRV32_DECORATE  static
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
   // write_memory_4("dram", ofs, val);
	//psram_write(ofs,&val,4);
	*((uint32_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,4);
}

}

static void MINIRV32_STORE2(uint32_t ofs, uint16_t val) {
    //write_memory_2("dram", ofs, val);
    //psram_write(ofs,&val,2);
    if (check_memory_bounds(ofs, 2)) {
    *((uint16_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,2);
}
}

static void MINIRV32_STORE1(uint32_t ofs, uint8_t val) {
    //write_memory_1("dram", ofs, val);
    //psram_write(ofs,&val,1);
    if (check_memory_bounds(ofs, 1)) {
    *((uint8_t *)(mem.p + ofs)) = val;//cache_write(ofs,&val,1);
}
}

static uint32_t MINIRV32_LOAD4(uint32_t ofs) {
    uint32_t val = 0;
    //return read_memory_4("dram", ofs);
    //psram_read(ofs,&val,4);
    if (check_memory_bounds(ofs, 4)) {
    val = *((uint32_t *)(mem.p + ofs));//cache_read(ofs,&val,4);

}
    return val;
}

static uint16_t MINIRV32_LOAD2(uint32_t ofs) {
    uint16_t val = 0;
    //return read_memory_2("dram", ofs);
    //psram_read(ofs,&val,2);
    if (check_memory_bounds(ofs, 2)) {
    val = *((uint16_t *)(mem.p + ofs));//cache_read(ofs,&val,2);
}
    return val;
}

static uint8_t MINIRV32_LOAD1(uint32_t ofs) {
	uint8_t val = 0;
      //return read_memory_1("dram", ofs);//(uint8_t)ram_image[ofs];
    //psram_read(ofs,&val,1);
    if (check_memory_bounds(ofs, 1)) {
    val = *((uint8_t *)(mem.p + ofs));//cache_read(ofs,&val,1);
}
    return val;
}
#include "mini-rv32ima.h"

//uint8_t * ram_image = 0;
struct MiniRV32IMAState core;
const char * kernel_command_line = 0;

static void DumpState( struct MiniRV32IMAState * core, uint8_t * ram_image );

int main( int argc, char ** argv )
{

restart:
	{
		/*FILE * f = fopen( argv[1], "rb" );
		if( !f || ferror( f ) )
		{
			fprintf( stderr, "Error: \"%s\" not found\n", argv[1] );
			return -5;
		}
		fseek( f, 0, SEEK_END );
		long flen = ftell( f );
		fseek( f, 0, SEEK_SET );
		if( flen > DRAM_SIZE )
		{
			fprintf( stderr, "Error: Could not fit RAM image (%ld bytes) into %d\n", flen, DRAM_SIZE );
			return -6;
		}

		memset( mem.data, 0, DRAM_SIZE );
		if( fread( mem.data, flen, 1, f ) != 1)
		{
			fprintf( stderr, "Error: Could not load image.\n" );
			return -7;
		}
		fclose( f );*/
        mem = create_memory(argv[1]);

	}

	//CaptureKeyboardInput();

	// The core lives at the end of RAM.
	/*core = (struct MiniRV32IMAState *)(ram_image + DRAM_SIZE - sizeof( struct MiniRV32IMAState ));
	core->pc = MINIRV32_RAM_IMAGE_OFFSET;
	core->regs[10] = 0x00; //hart ID
	core->regs[11] = 0; //dtb_pa (Must be valid pointer) (Should be pointer to dtb)
	core->extraflags |= 3; // Machine-mode.*/

	core.regs[10] = 0x00; // hart ID
	core.regs[11] = 0;
	core.extraflags |= 3; // Machine-mode.

	core.pc = MINIRV32_RAM_IMAGE_OFFSET;
	// Image is loaded.
	
	//int y;
	//char buff[100];

#if 1


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
    #else
    uint64_t rt;
	uint64_t lastTime = (fixed_update)?0:(GetTimeMicroseconds()/time_divisor);
	int instrs_per_flip = 1024;
	for( rt = 0; rt < instct+1 || instct < 0; rt += instrs_per_flip )
	{
		uint64_t * this_ccount = ((uint64_t*)&core.cyclel);
		uint32_t elapsedUs = 0;
		if( fixed_update )
			elapsedUs = *this_ccount / time_divisor - lastTime;
		else
			elapsedUs = GetTimeMicroseconds()/time_divisor - lastTime;
		lastTime += elapsedUs;

		 
		int ret = MiniRV32IMAStep( &core, ram_image, 0, elapsedUs, instrs_per_flip ); // Execute upto 1024 cycles before breaking out.
		switch( ret )
		{
			case 0: break;
			case 1: //if( do_sleep ) MiniSleep(); *this_ccount += instrs_per_flip; 
				break;
			case 3: instct = 0; break;
			case 0x7777: goto restart;	//syscon code for restart
			case 0x5555: printf( "POWEROFF@0x%08x%08x\n", core.cycleh, core.cyclel ); return 0; //syscon code for power-off
			default: printf( "Unknown failure\n" ); break;
		}
	}
	#endif
	

	DumpState( &core, ram_image);
}


//////////////////////////////////////////////////////////////////////////
// Platform-specific functionality
//////////////////////////////////////////////////////////////////////////



#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

static void CtrlC()
{
	DumpState( &core, ram_image);
	exit( 0 );
}

// Override keyboard, so we can capture all keyboard input for the VM.
static void CaptureKeyboardInput()
{
	// Hook exit, because we want to re-enable keyboard.
	atexit(ResetKeyboardInput);
	signal(SIGINT, CtrlC);

	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag &= ~(ICANON | ECHO); // Disable echo as well
	tcsetattr(0, TCSANOW, &term);
}

static void ResetKeyboardInput()
{
	// Re-enable echo, etc. on keyboard.
	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag |= ICANON | ECHO;
	tcsetattr(0, TCSANOW, &term);
}

static void MiniSleep()
{
	usleep(500);
}

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
	else if( csrno == 0x138 )
	{
		//Print "string"
		uint32_t ptrstart = value - MINIRV32_RAM_IMAGE_OFFSET;
		uint32_t ptrend = ptrstart;
		if( ptrstart >= DRAM_SIZE )
			printf( "DEBUG PASSED INVALID PTR (%08x)\n", value );
		while( ptrend < DRAM_SIZE )
		{
			if( image[ptrend] == 0 ) break;
			ptrend++;
		}
		if( ptrend != ptrstart )
			fwrite( image + ptrstart, ptrend - ptrstart, 1, stdout );
	}
	else if( csrno == 0x139 )
	{
		putchar( value ); fflush( stdout );
	}else if (csrno == 0x400){//download
		uint32_t start = value - MINIRV32_RAM_IMAGE_OFFSET;
		printf("%x\n",start );
	    FILE *fptr;
	    fptr = fopen("fs2.img","w");

	    if(fptr == NULL)
	    {
	      printf("Error!");   
	      exit(1);             
	    }
	    fwrite((image + start), 500000, 1, fptr); 
	    fclose(fptr);
	}else if (csrno == 0x401){//upload
		uint32_t start = value - MINIRV32_RAM_IMAGE_OFFSET;
		printf("%x\n",start );
		FILE *fptr;
	    fptr = fopen("fs2.img","r");

	    if(fptr == NULL)
	    {
	      printf("Error!");   
	      exit(1);             
	    }
	    fread((image + start), 500000, 1, fptr); 
	    fclose(fptr);
	}
}


enum {
    SYS_FOPEN = 0x0001,
    SYS_FCLOSE = 0x0002,
    SYS_FFLUSH = 0x0003,
    SYS_FGETC = 0x0004,
    SYS_FGETS = 0x0005,
    SYS_FPUTC = 0x0006,
    SYS_FPUTS = 0x0007,
    SYS_FWRITE = 0x0008,
    SYS_FREAD = 0x0009,
    SYS_FSEEK = 0x0010,
    SYS_FGETPOS = 0x0011,
    SYS_FTELL = 0x0012,
    SYS_FEOF = 0x0013,
    SYS_REMOVE = 0x0014,
    SYS_MKDIR = 0x0015
};




// Función para obtener el puntero a una cadena desde la memoria
char* getString(uint32_t t) {
    uint32_t pc_offset = t - MINIRV32_RAM_IMAGE_OFFSET;  // Calculamos la dirección real en la memoria

    uint32_t i = 0;
    //static char str[256];  // Almacenamos la cadena en un arreglo estático (puedes ajustar el tamaño según sea necesario)
    char *str=malloc(101);
    // Leer byte por byte hasta encontrar el carácter nulo '\0' o alcanzar el tamaño máximo
    while (1) {
        char c = MINIRV32_LOAD1(pc_offset + i);  // Leer el byte de la memoria
        if (c == '\0' || i >= 100) {  // Fin de la cadena o límite de tamaño
            str[i] = '\0';  // Aseguramos que la cadena esté correctamente terminada
            break;
        }
        str[i] = c;  // Copiar el carácter al arreglo
        i++;
    }

    return str;  // Retornar el puntero a la cadena almacenada en 'str'
}


static uint32_t HandleOtherReturnSyscall(uint16_t csrno,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4,uint32_t a5) {
    uint32_t x = 0;

    // Usamos un switch para manejar las syscalls
    switch (csrno) {
        case SYS_FOPEN:
            // Lógica específica para la syscall SYS_FOPEN (0x0001)
            //x = 123; // Este es un valor de ejemplo, cámbialo según lo que necesites
            //printf("%x %x\n", a0,a1);
            //printf("%s %s\n",getString(a0) ,getString(a1));

            char *filename=getString(a0);
            char *mode=getString(a1);
            printf("%s %s\n",filename , mode);
            free(filename);
            free(mode);
            x=0;
            break;
        case SYS_FCLOSE:
            // Lógica específica para la syscall SYS_FCLOSE (0x0002)
            x = 555; // Otro valor de ejemplo
            break;
        case SYS_FFLUSH:
            // Lógica para SYS_FFLUSH (0x0003)
            x = 789;
            break;
        case SYS_FGETC:
            // Lógica para SYS_FGETC (0x0004)
            x = 456;
            break;
        case SYS_FGETS:
            // Lógica para SYS_FGETS (0x0005)
            x = 321;
            break;
        case SYS_FPUTC:
            // Lógica para SYS_FPUTC (0x0006)
            x = 654;
            break;
        case SYS_FPUTS:
            // Lógica para SYS_FPUTS (0x0007)
            x = 987;
            break;
        case SYS_FWRITE:
            // Lógica para SYS_FWRITE (0x0008)
            x = 1001;
            break;
        case SYS_FREAD:
            // Lógica para SYS_FREAD (0x0009)
            x = 2002;
            break;
        case SYS_FSEEK:
            // Lógica para SYS_FSEEK (0x0010)
            x = 3030;
            break;
        case SYS_FGETPOS:
            // Lógica para SYS_FGETPOS (0x0011)
            x = 4040;
            break;
        case SYS_FTELL:
            // Lógica para SYS_FTELL (0x0012)
            x = 5050;
            break;
        case SYS_FEOF:
            // Lógica para SYS_FEOF (0x0013)
            x = 6060;
            break;
        case SYS_REMOVE:
            // Lógica para SYS_REMOVE (0x0014)
            x = 7070;
            break;
        case SYS_MKDIR:
            // Lógica para SYS_MKDIR (0x0015)
            x = 8080;
            break;
        default:
            // Si la syscall no se encuentra en el enum, manejamos un caso por defecto
            x = 0; // Valor predeterminado si no se encuentra la syscall
            break;
    }

    return x;
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
        c=IsKBHit();
        if(!c){
        	return -1;
        }
        c = ReadKBByte();
        
        //printf("csr %c\n",c );
        return c;
		//if( !IsKBHit() ) return -1;
		//return ReadKBByte();
	}else if(csrno == 0x333){
		//struct data *s=(struct data*)a;
		//printf("%s\n",s->a);
		//printf("%s\n",(uint32_t  *)a );
		//char *p=(char *)a;
		//printf("aquiiii %s\n",*(char *)a );
		return 0;
	}
	return 0;
}

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber )
{
	if( !number || !number[0] ) return defaultNumber;
	int radix = 10;
	if( number[0] == '0' )
	{
		char nc = number[1];
		number+=2;
		if( nc == 0 ) return 0;
		else if( nc == 'x' ) radix = 16;
		else if( nc == 'b' ) radix = 2;
		else { number--; radix = 8; }
	}
	char * endptr;
	uint64_t ret = strtoll( number, &endptr, radix );
	if( endptr == number )
	{
		return defaultNumber;
	}
	else
	{
		return ret;
	}
}

static void DumpState( struct MiniRV32IMAState * core, uint8_t * ram_image )
{
	uint32_t pc = core->pc;
	uint32_t pc_offset = pc - MINIRV32_RAM_IMAGE_OFFSET;
	uint32_t ir = 0;

	printf( "PC: %08x ", pc );
	if( pc_offset >= 0 && pc_offset < DRAM_SIZE - 3 )
	{
		ir = *((uint32_t*)(&((uint8_t*)ram_image)[pc_offset]));
		printf( "[0x%08x] ", ir ); 
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

