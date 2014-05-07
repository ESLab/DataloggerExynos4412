#include "INA226.h"
#include "I2C.h"
#include "Registers.h"

static uint16_t ReadRegister(int address);
static int16_t ReadSignedRegister(int address);
static void WriteRegister(int address,uint16_t value);

bool InitializeINA226(int address,int calibration)
{
	if(!InitializeI2C()) return false;

	SetI2CAddress(address);

	WriteRegister(0,0x8000); // Reset
	WriteRegister(0,0x4c07); // Average 256, 140 us conversion times, continuous voltage+current.
	WriteRegister(5,calibration);

	return true;
}

bool ReadAndClearINA226ReadyFlag(int address)
{
	SetI2CAddress(address);
	if(ReadRegister(6)&0x0008) return true;
	else return false;
}

int ReadINA226Voltage(int address)
{
	SetI2CAddress(address);
	return ReadRegister(2);
}

int ReadINA226Current(int address)
{
	SetI2CAddress(address);
	return ReadSignedRegister(4);
}

int ReadINA226Power(int address)
{
	SetI2CAddress(address);
	return ReadSignedRegister(3);
}

static uint16_t ReadRegister(int address)
{
	uint8_t value[2];
	ReadI2CRegisters(address,value,2);
	return (value[0]<<8)|value[1];
}

static int16_t ReadSignedRegister(int address)
{
	return (int16_t)ReadRegister(address);
}

static void WriteRegister(int address,uint16_t value)
{
	WriteI2CRegisters((const uint8_t [3]){ address,value>>8,value&0xff },3);
}

