
#include <os.h>
#include <aes.h>


 

shellcmd xsh_test(int nargs, char *args[]){
  
  Byte block[16]={1, 2, 3, 4, 5, 6, 7, 8, 9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16}; 
  Byte block2[16];
  memcpy(block2, block, 16);

  Byte key[32]={1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0,
          0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20,
          0x21, 0x22, 0x23, 0x24, 0x25, 0x26};  
  
 
  AES_setKey(key);


  AES256_encrypt(block);  
  for(int i=0;i<4;++i){
    fprintf(stdout," %2x %2x %2x %2x\n", block[i+0], block[i+4], block[i+8], block[i+12]);
  }
  fprintf(stdout,"\n");

  AES256_decrypt(block);  
  for(int i=0;i<16;++i){
    if(block[i]!=block2[i]){
      fprintf(stdout,"ERROR.\n");
      return 1;
    }
  }
  fprintf(stdout,"OK.\n");


     

    
 
    
    // Debug: Mostrar información de los descriptores
    fprintf(stdout, "\n=== Debug de descriptores ===\n");
    fprintf(stdout, "PID: %d\n", getpid());
    fprintf(stdout, "stdin: %d\n", proctab[getpid()].prdesc[0]);
    fprintf(stdout, "stdout: %d\n", proctab[getpid()].prdesc[1]);
    fprintf(stdout, "stderr: %d\n", proctab[getpid()].prdesc[2]);
    fprintf(stdout, "extra: %d\n", proctab[getpid()].prdesc[3]);
    fprintf(stdout, "===========================\n\n");

    // Prueba de salida continua
    for(int i = 0; i < 100; i++) {
        fprintf(stdout, "Mensaje %d desde test (PID: %d)\n", i, getpid());
        sleepms(5);
    }

    return 0;
}

