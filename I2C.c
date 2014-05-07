#include "I2C.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define Device "/dev/i2c-0"

static int fd=0;
static int currentaddress;

bool InitializeI2C()
{
	if(fd) return true;

	fd=open(Device,O_RDWR);
	if(fd<0)
	{
		fprintf(stderr,"Error opening %s\n",Device);
		return false;
	}

	currentaddress=-1;

	return true;
}

void SetI2CAddress(int address)
{
	if(address!=currentaddress)
	{
		if(ioctl(fd,I2C_SLAVE,address)<0)
		{
			fprintf(stderr,"Unable to configure I2C address while accessing LTC2991.\n");
			exit(1);
	    }

		currentaddress=address;
	}
}

uint8_t ReadI2CRegister(int address)
{
	uint8_t value;
	ReadI2CRegisters(address,&value,1);
	return value;
}

void ReadI2CRegisters(int address,uint8_t *buffer,int length)
{
	if(write(fd,(uint8_t[1]){ address },1)!=1)
	{
		fprintf(stderr,"Error writing to I2C.\n");
		exit(1);
	}

	if(read(fd,buffer,length)!=length)
	{
		fprintf(stderr,"Error reading from I2C.\n");
		exit(1);
	}
}

void WriteI2CRegister(int address,uint8_t value)
{
	WriteI2CRegisters((const uint8_t[2]){ address,value },2);
}

void WriteI2CRegisters(const uint8_t *addressandvalues,int length)
{
	if(write(fd,addressandvalues,length)!=length)
	{
		fprintf(stderr,"Error writing to I2C.\n");
		exit(1);
	}
}

