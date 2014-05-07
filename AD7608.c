#include "AD7608.h"
#include "Registers.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define Device "/dev/spidev0.0"

static int fd;

bool InitializeAD7608()
{
	if(!InitializeRegisterAccess()) return false;

	fd=open(Device,O_RDWR);
	if(fd<0)
	{
		fprintf(stderr,"Error opening %s\n",Device);
		return false;
	}

	int mode=SPI_MODE_2;
	if(ioctl(fd,SPI_IOC_RD_MODE,&mode)<0) { fprintf(stderr,"Error configuring %s\n",Device); exit(1); }

	SetGPIOOutputMode(25);

	GPSET0=1<<25;

	return true;
}

void SampleAD7608(int channels[static 8])
{
	uint8_t output[18]={0};
	uint8_t input[18];

	GPCLR0=1<<25;
	usleep(1);
	GPSET0=1<<25;

	usleep(50);

    ioctl(fd,SPI_IOC_MESSAGE(1),&(struct spi_ioc_transfer){
		.tx_buf=(unsigned long)output,
		.rx_buf=(unsigned long)input,
		.len=18,
		.delay_usecs=0,
		.speed_hz=1000000,
		.bits_per_word=8,
	});

	channels[0]=((input[0]<<10)|(input[1]<<2)|(input[2]>>6))&0x3ffff;
	channels[1]=((input[2]<<12)|(input[3]<<4)|(input[4]>>4))&0x3ffff;
	channels[2]=((input[4]<<14)|(input[5]<<6)|(input[6]>>2))&0x3ffff;
	channels[3]=((input[6]<<16)|(input[7]<<8)|(input[8]>>0))&0x3ffff;

	channels[4]=((input[ 9]<<10)|(input[10]<<2)|(input[11]>>6))&0x3ffff;
	channels[5]=((input[11]<<12)|(input[12]<<4)|(input[13]>>4))&0x3ffff;
	channels[6]=((input[13]<<14)|(input[14]<<6)|(input[15]>>2))&0x3ffff;
	channels[7]=((input[15]<<16)|(input[16]<<8)|(input[17]>>0))&0x3ffff;

	for(int i=0;i<8;i++)
	{
		if(channels[i]>=0x20000) channels[i]=0x40000-channels[i];
	}
}

