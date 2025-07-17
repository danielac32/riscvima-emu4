//#include "syscall.h"




int main(int argc, char *argv[])
{
	while(1){
		for (int i = 0; i < 1000; ++i)
		{
			asm("nop");
		}
	}

	return 0;
}