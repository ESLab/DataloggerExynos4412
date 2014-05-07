#include "Registers.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

volatile uint32_t PeripheralBase=0;

bool InitializeRegisterAccess()
{
	if(PeripheralBase) return true;

	int fd=open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0)
	{
		fprintf(stderr,"Failed to open /dev/mem.\n");
		return false;
	}

	PeripheralBase=(uint32_t)mmap(NULL,0x1000000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x20000000);

	close(fd);

	if(PeripheralBase==(uint32_t)MAP_FAILED)
	{
		fprintf(stderr,"Failed to mmap peripheral registers: %d.\n",errno);
		exit(1);
	}

	return true;
}


