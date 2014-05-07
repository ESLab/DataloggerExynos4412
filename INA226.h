#ifndef __INA226_H__
#define __INA226_H__

#include <stdbool.h>

bool InitializeINA226(int address,int calibration);

bool ReadAndClearINA226ReadyFlag(int address);
int ReadINA226Voltage(int address);
int ReadINA226Current(int address);
int ReadINA226Power(int address);

#endif

