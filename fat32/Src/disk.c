#include <disk.h>
#include <stdio.h>
#include <os.h>

 


int sd_init(void)
{   
   /* char *buff=malloc(512);
    uint32 y =__syscall(0xcafe0001,buff,512);
    hexDump2(0,buff,512);
    printf("%d\n",y );*/
    return (int)__syscall(0xcafe0001);//return control(FLASHSPI,0,0,0);//flash_info->card_size;
}
//-----------------------------------------------------------------
// sd_readsector: Read a number of blocks from SD card
//-----------------------------------------------------------------
int sd_readsector(unsigned int start_block, unsigned char *buffer, unsigned int sector_count)
{
    __syscall(0xcafe0003,buffer,start_block,sector_count);
    return 1;
}
//-----------------------------------------------------------------
// sd_writesector: Write a number of blocks to SD card
//-----------------------------------------------------------------
int sd_writesector(unsigned int start_block, unsigned char *buffer, unsigned int sector_count)
{
    __syscall(0xcafe0002,buffer,start_block,sector_count);
    return 1;
}




