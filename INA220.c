#include "INA220.h"
#include "I2C.h"
#include "Registers.h"

static uint16_t ReadRegister(int address);
static int16_t ReadSignedRegister(int address);
static void WriteRegister(int address,uint16_t value);

bool InitializeINA220(int address,int calibration)
{
	if(!InitializeI2C()) return false;

	SetI2CAddress(address);

	WriteRegister(0,0x8000); // Reset
	WriteRegister(0,0x1f77); // Bus voltage range 16 V, shunt voltage range 320mV,
	// average 64 (34.05 ms total conversion time), continuous voltage+current.
	WriteRegister(5,calibration);

	return true;
}

int ReadINA220VoltageAndFlags(int address)
{
	SetI2CAddress(address);
	return ReadRegister(2);
}

int ReadINA220Current(int address)
{
	SetI2CAddress(address);
	return ReadSignedRegister(4);
}

int ReadINA220Power(int address)
{
	SetI2CAddress(address);
	return ReadRegister(3);
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

