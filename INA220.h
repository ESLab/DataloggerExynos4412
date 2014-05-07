#ifndef __INA220_H__
#define __INA220_H__

#include <stdbool.h>

bool InitializeINA220(int address,int calibration);

int ReadINA220VoltageAndFlags(int address);
int ReadINA220Current(int address);
int ReadINA220Power(int address);

#endif

