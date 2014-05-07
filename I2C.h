#ifndef __I2C_H__
#define __I2C_H__

#include <stdbool.h>
#include <stdint.h>

bool InitializeI2C();
void SetI2CAddress(int address);
uint8_t ReadI2CRegister(int address);
void ReadI2CRegisters(int address,uint8_t *buffer,int length);
void WriteI2CRegister(int address,uint8_t value);
void WriteI2CRegisters(const uint8_t *addressandvalues,int length);

#endif

