
#include <stdio.h>
#include <kernel.h>
#include <delay.h>


int main(int argc, char const *argv[])
{
	while(1){
		kprintf("main\n");
		MDELAY(500);
	}
	return 0;
}