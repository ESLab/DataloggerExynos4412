#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdint.h>
#include <stdbool.h>

extern volatile uint32_t PeripheralBase;

#define CM_GP0CTL (*(volatile uint32_t *)(PeripheralBase+0x101070))
#define CM_GP0DIV (*(volatile uint32_t *)(PeripheralBase+0x101074))
#define CM_GP1CTL (*(volatile uint32_t *)(PeripheralBase+0x101078))
#define CM_GP1DIV (*(volatile uint32_t *)(PeripheralBase+0x10107c))
#define CM_GP2CTL (*(volatile uint32_t *)(PeripheralBase+0x101080))
#define CM_GP2DIV (*(volatile uint32_t *)(PeripheralBase+0x101084))

#define CM_PWMCTL (*(volatile uint32_t *)(PeripheralBase+0x1010a0))
#define CM_PWMDIV (*(volatile uint32_t *)(PeripheralBase+0x1010a4))

#define GPFSEL(n) (*(volatile uint32_t *)(PeripheralBase+0x200000+(n)*4))
#define GPSET0 (*(volatile uint32_t *)(PeripheralBase+0x20001c))
#define GPSET1 (*(volatile uint32_t *)(PeripheralBase+0x200020))
#define GPSET2 (*(volatile uint32_t *)(PeripheralBase+0x200024))
#define GPCLR0 (*(volatile uint32_t *)(PeripheralBase+0x200028))
#define GPCLR1 (*(volatile uint32_t *)(PeripheralBase+0x20002c))
#define GPCLR2 (*(volatile uint32_t *)(PeripheralBase+0x200030))
#define GPLEV0 (*(volatile uint32_t *)(PeripheralBase+0x200034))
#define GPLEV1 (*(volatile uint32_t *)(PeripheralBase+0x200038))
#define GPLEV2 (*(volatile uint32_t *)(PeripheralBase+0x20003c))
#define GPPUD (*(volatile uint32_t *)(PeripheralBase+0x200094))
#define GPPUDCLK0 (*(volatile uint32_t *)(PeripheralBase+0x200098))
#define GPPUDCLK1 (*(volatile uint32_t *)(PeripheralBase+0x20009c))

#define PWMCTL (*(volatile uint32_t *)(PeripheralBase+0x20c000))
#define PWMSTA (*(volatile uint32_t *)(PeripheralBase+0x20c004))
#define PWMDMAC (*(volatile uint32_t *)(PeripheralBase+0x20c008))
#define PWMRNG1 (*(volatile uint32_t *)(PeripheralBase+0x20c010))
#define PWMDAT1 (*(volatile uint32_t *)(PeripheralBase+0x20c014))
#define PWMFIF1 (*(volatile uint32_t *)(PeripheralBase+0x20c018))
#define PWMRNG2 (*(volatile uint32_t *)(PeripheralBase+0x20c020))
#define PWMDAT2 (*(volatile uint32_t *)(PeripheralBase+0x20c024))

bool InitializeRegisterAccess();

static inline void SetGPIOMode(int pin,int mode);

static inline void SetGPIOInputMode(int pin) { SetGPIOMode(pin,0); }

static inline void SetGPIOOutputMode(int pin) { SetGPIOMode(pin,1); }

static inline void SetGPIOAlternateMode(int pin,int mode)
{
	if(mode<4) SetGPIOMode(pin,mode+4);
	else if(mode==4) SetGPIOMode(pin,3);
	else SetGPIOMode(pin,2);
}

static inline void SetGPIOMode(int pin,int mode)
{
	int regindex=pin/10;
	int bitindex=(pin%10)*3;

	uint32_t sel=GPFSEL(regindex);
	sel&=~(0x7<<bitindex);
	sel|=mode<<bitindex;
	GPFSEL(regindex)=sel;
}

static inline int GPIOMode(int pin)
{
	int regindex=pin/10;
	int bitindex=(pin%10)*3;

	return (GPFSEL(regindex)>>bitindex)&7;
}

#endif

