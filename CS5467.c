#include "CS5467.h"
#include "Registers.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define Device "/dev/spidev0.1"

#define VoltageDCOffset 0x073b3f
//#define CurrentDCOffset 0xfe7de1
#define CurrentDCOffset 0xfe7e23
#define VoltageACOffset 0
#define CurrentACOffset 0xffe9bc


#define REG(page,address) (((page)<<5)|(address))

#define CONFIG REG(0,0)
#define INSTANTENOUS_CURRENT_1 REG(0,1)
#define INSTANTENOUS_VOLTAGE_1 REG(0,2)
#define INSTANTENOUS_POWER_1 REG(0,3)
#define ACTIVE_POWER_1 REG(0,4)
#define RMS_CURRENT_1 REG(0,5)
#define RMS_VOLTAGE_1 REG(0,6)
#define INSTANTENOUS_CURRENT_2 REG(0,7)
#define INSTANTENOUS_VOLTAGE_2 REG(0,8)
#define INSTANTENOUS_POWER_2 REG(0,9)
#define ACTIVE_POWER_2 REG(0,10)
#define RMS_CURRENT_2 REG(0,11)
#define RMS_VOLTAGE_2 REG(0,12)
#define REACTIVE_POWER_1 REG(0,13)
#define INSTANTENOUS_QUADRATURE_POWER_1 REG(0,14)
#define STATUS REG(0,15)
#define REACTIVE_POWER_2 REG(0,16)
#define INSTANTENOUS_QUADRATURE_POWER_2 REG(0,17)
#define PEAK_CURRENT_1 REG(0,18)
#define PEAK_VOLTAGE_1 REG(0,19)
#define APPARENT_POWER_1 REG(0,20)
#define PEAK_CURRENT_1 REG(0,18)
#define PEAK_VOLTAGE_1 REG(0,19)
#define APPARENT_POWER_1 REG(0,20)
#define POWER_FACTOR_1 REG(0,21)
#define PEAK_CURRENT_2 REG(0,22)
#define PEAK_VOLTAGE_2 REG(0,23)
#define APPARENT_POWER_2 REG(0,24)
#define POWER_FACTOR_2 REG(0,25)
#define INTERRUPT_MASK REG(0,26)
#define TEMPERATURE REG(0,27)
#define CONTROL REG(0,28)
#define ACTIVE_ENERGY_PULSE_OUTPUT REG(0,29)
#define APPARENT_ENERGY_PULSE_OUTPUT REG(0,30)
#define REACTIVE_ENERGY_PULSE_OUTPUT REG(0,31)

#define CURRENT_DC_OFFSET_1 REG(1,0)
#define CURRENT_GAIN_1 REG(1,1)
#define VOLTAGE_DC_OFFSET_1 REG(1,2)
#define VOLTAGE_GAIN_1 REG(1,3)
#define POWER_OFFSET_1 REG(1,4)
#define CURRENT_AC_RMS_OFFSET_1 REG(1,5)
#define VOLTAGE_AC_RMS_OFFSET_1 REG(1,6)
#define CURRENT_DC_OFFSET_2 REG(1,7)
#define CURRENT_GAIN_2 REG(1,8)
#define VOLTAGE_DC_OFFSET_2 REG(1,9)
#define VOLTAGE_GAIN_2 REG(1,10)
#define POWER_OFFSET_2 REG(1,11)
#define CURRENT_AC_RMS_OFFSET_2 REG(1,12)
#define VOLTAGE_AC_RMS_OFFSET_2 REG(1,13)
#define PULSE_OUTPUT_WIDTH REG(1,14)
#define PULSE_OUTPUT_RATE REG(1,15)
#define MODE_CONTROL REG(1,16)
#define RATIO_OF_LINE_TO_SAMPLE_FREQUENCY REG(1,17)
#define CYCLE_COUNT REG(1,19)
#define WIDEBAND_REACTIVE_POWER_1 REG(1,20)
#define WIDEBAND_REACTIVE_POWER_2 REG(1,21)
#define TEMPERATURE_SENSOR_GAIN REG(1,22)
#define TEMPERATURE_SENSOR_OFFSET REG(1,23)
#define FILTER_SETTLING_TIME REG(1,25)
#define NO_LOAD_THRESHOLD REG(1,26)
#define VOLTAGE_RMS_FIXED_REFERENCE REG(1,27)
#define SYSTEM_GAIN REG(1,28)
#define SYSTEM_TIME REG(1,29)

#define V_SAG_DURATION_1 REG(2,0)
#define V_SAG_LEVEL_1 REG(2,1)
#define I_FAULT_DURATION_1 REG(2,4)
#define I_FAULT_LEVEL_1 REG(2,5)
#define V_SAG_DURATION_2 REG(2,8)
#define V_SAG_LEVEL_2 REG(2,9)
#define I_FAULT_DURATION_2 REG(2,12)
#define I_FAULT_LEVEL_2 REG(2,13)

#define TEMPERATURE_MEASUREMENT REG(5,26)

static void WaitForAndClearDataReady();
static void WaitForDataReady();
static void ClearDataReady();

static void Reset();
static void StartContinuousConversion();
static void CalibratePowerDCOffset();
static void CalibratePowerDCGain();
static void CalibratePowerACOffset();
static void CalibratePowerACGain();
static int ReadRegister(int reg);
static int ReadSignedRegister(int reg);
static void WriteRegister(int reg,int value);
static void SetPageForRegister(int reg);
static void Send4ByteCommandWithoutInput(const uint8_t output[4]);
static void Send4ByteCommand(const uint8_t output[4],uint8_t input[4]);
static void Send1ByteCommand(uint8_t command);

static int fd,currentpage;

bool InitializeCS5467()
{
	if(!InitializeRegisterAccess()) return false;

	currentpage=-1;

	fd=open(Device,O_RDWR);
	if(fd<0)
	{
		fprintf(stderr,"CS5467: Error opening %s\n",Device);
		return false;
	}

	int mode=SPI_MODE_0;
	if(ioctl(fd,SPI_IOC_RD_MODE,&mode)<0) { fprintf(stderr,"CS5467: Error configuring %s\n",Device); return false; }
	if(ioctl(fd,SPI_IOC_WR_MODE,&mode)<0) { fprintf(stderr,"CS5467: Error configuring %s\n",Device); return false; }

	SetGPIOAlternateMode(18,5);

	PWMCTL=0; // Turn off PWM.

	PWMCTL=0x0081; // Channel 1 M/S mode, no FIFO, PWM mode, enabled.

	CM_PWMCTL=(CM_PWMCTL&~0x10)|0x5a000000; // Turn off enable flag.
	while(CM_PWMCTL&0x80); // Wait for busy flag to turn off.
	CM_PWMDIV=0x5a000000|(5<<12); // Configure divider.
	CM_PWMCTL=0x5a000206; // Source=PLLD (500 MHz), 1-stage MASH.
	CM_PWMCTL=0x5a000216; // Enable clock.
	while(!(CM_PWMCTL&0x80)); // Wait for busy flag to turn on.

	PWMRNG1=494;
	PWMDAT1=494*36/91;

	PWMCTL=0x0081; // Channel 1 M/S mode, no FIFO, PWM mode, enabled.

	Reset();

	WaitForDataReady();

	WriteRegister(CONFIG,0x000003); // K=3 (12,788 Mhz)
	WriteRegister(CONTROL,0x001034); // Phase == 0, I2GAIN  50 mV, I1GAIN  50 mV
	//WriteRegister(MODE_CONTROL,0x000003); // All HPF disabled.
	WriteRegister(MODE_CONTROL,0x0001e3); // All HPF enabled.

	WriteRegister(VOLTAGE_DC_OFFSET_2,VoltageDCOffset);
	WriteRegister(CURRENT_DC_OFFSET_2,CurrentDCOffset);
	WriteRegister(VOLTAGE_AC_RMS_OFFSET_2,VoltageACOffset);
	WriteRegister(CURRENT_AC_RMS_OFFSET_2,CurrentACOffset);

	return true;
}

void StartSamplingCS5467()
{
/*	WriteRegister(CYCLE_COUNT,40000);

	printf("Channel 2 DC voltage offset: %06x\n",ReadRegister(VOLTAGE_DC_OFFSET_2));
	printf("Channel 2 DC current offset: %06x\n",ReadRegister(CURRENT_DC_OFFSET_2));

	CalibratePowerDCOffset();

	printf("Channel 2 DC voltage offset: %06x\n",ReadRegister(VOLTAGE_DC_OFFSET_2));
	printf("Channel 2 DC current offset: %06x\n",ReadRegister(CURRENT_DC_OFFSET_2));
*/
/*	printf("Channel 2 AC RMS voltage offset: %06x\n",ReadRegister(VOLTAGE_AC_RMS_OFFSET_2));
	printf("Channel 2 AC RMS current offset: %06x\n",ReadRegister(CURRENT_AC_RMS_OFFSET_2));

	CalibratePowerACOffset();

	printf("Channel 2 AC RMS voltage offset: %06x\n",ReadRegister(VOLTAGE_AC_RMS_OFFSET_2));
	printf("Channel 2 AC RMS current offset: %06x\n",ReadRegister(CURRENT_AC_RMS_OFFSET_2));

	printf("Channel 2 DC voltage gain: %06x\n",ReadRegister(VOLTAGE_GAIN_2));
*/
/*	printf("Channel 2 voltage gain: %06x\n",ReadRegister(VOLTAGE_GAIN_2));
	printf("Channel 2 current gain: %06x\n",ReadRegister(CURRENT_GAIN_2));

	CalibratePowerACGain();

	printf("Channel 2 voltage gain: %06x\n",ReadRegister(VOLTAGE_GAIN_2));
	printf("Channel 2 current gain: %06x\n",ReadRegister(CURRENT_GAIN_2));
*/

/*	printf("System gain: %06x\n",ReadRegister(SYSTEM_GAIN));
	printf("Config: %06x\n",ReadRegister(CONFIG));
	printf("Status: %06x\n",ReadRegister(STATUS));
	printf("Cycle count: %06x\n",ReadRegister(CYCLE_COUNT));
	printf("Temperature: %06x\n",ReadRegister(TEMPERATURE));
	printf("Temperature: %06x\n",ReadRegister(TEMPERATURE_MEASUREMENT));
	printf("System time: %06x\n",ReadRegister(SYSTEM_TIME));*/

	StartContinuousConversion();
}

bool IsCS5467DataReady()
{
	return ReadRegister(STATUS)&0x800000?true:false;
}

RawCS5467Sample NextRawCS5467Sample()
{
	WaitForAndClearDataReady();

	return (RawCS5467Sample){
		.rms_voltage=ReadRegister(RMS_VOLTAGE_2),
		.rms_current=ReadRegister(RMS_CURRENT_2),
		.active_power=ReadSignedRegister(ACTIVE_POWER_2),
		.reactive_power=ReadSignedRegister(REACTIVE_POWER_2),
		.apparent_power=ReadSignedRegister(APPARENT_POWER_2),
		.power_factor=ReadSignedRegister(POWER_FACTOR_2),
		.wideband_reactive_power=ReadSignedRegister(WIDEBAND_REACTIVE_POWER_2),
		.active_energy_pulse=ReadSignedRegister(ACTIVE_ENERGY_PULSE_OUTPUT),
		.reactive_energy_pulse=ReadSignedRegister(REACTIVE_ENERGY_PULSE_OUTPUT),
		.apparent_energy_pulse=ReadSignedRegister(APPARENT_ENERGY_PULSE_OUTPUT),
		.status=ReadRegister(STATUS),
	};
}

CS5467Sample NextCS5467Sample()
{
	return CalibrateRawCS5467Sample(NextRawCS5467Sample());
}

CS5467Sample CalibrateRawCS5467Sample(RawCS5467Sample raw)
{
	return (CS5467Sample){
		.rms_voltage=(float)raw.rms_voltage*CS5467VoltageCalibration,
		.rms_current=(float)raw.rms_current*CS5467CurrentCalibration,
		.active_power=(float)raw.active_power*CS5467PowerCalibration,
		.reactive_power=(float)raw.reactive_power*CS5467PowerCalibration,
		.apparent_power=(float)raw.apparent_power*CS5467PowerCalibration,
		.power_factor=(float)raw.power_factor*CS5467PowerFactorCalibration,
		.wideband_reactive_power=(float)raw.wideband_reactive_power*CS5467PowerCalibration,
		.active_energy_pulse=(float)raw.active_energy_pulse*CS5467EnergyCalibration,
		.reactive_energy_pulse=(float)raw.reactive_energy_pulse*CS5467EnergyCalibration,
		.apparent_energy_pulse=(float)raw.apparent_energy_pulse*CS5467EnergyCalibration,
		.status=raw.status,
	};
}

static void WaitForAndClearDataReady()
{
	WaitForDataReady();
	ClearDataReady();
}

static void WaitForDataReady()
{
	while(!(ReadRegister(STATUS)&0x800000));
}

static void ClearDataReady()
{
	WriteRegister(STATUS,0x800000);
}

static void Reset()
{
	Send1ByteCommand(0x80);
}

static void StartContinuousConversion()
{
	Send1ByteCommand(0xe8);
}

static void CalibratePowerDCOffset()
{
	ClearDataReady();
	Send1ByteCommand(0x8c);
	WaitForAndClearDataReady();
}

static void CalibratePowerDCGain()
{
	ClearDataReady();
	Send1ByteCommand(0x9c);
	WaitForAndClearDataReady();
}

static void CalibratePowerACOffset()
{
	ClearDataReady();
	Send1ByteCommand(0xac);
	WaitForAndClearDataReady();
}

static void CalibratePowerACGain()
{
	ClearDataReady();
	Send1ByteCommand(0xbc);
	WaitForAndClearDataReady();
}

static int ReadRegister(int reg)
{
	SetPageForRegister(reg);
	uint8_t output[4]={ (reg&0x1f)<<1,0xfe,0xfe,0xfe };
	uint8_t input[4];
	Send4ByteCommand(output,input);

	return (input[1]<<16)|(input[2]<<8)|input[3];
}

static int ReadSignedRegister(int reg)
{
	SetPageForRegister(reg);
	uint8_t output[4]={ (reg&0x1f)<<1,0xfe,0xfe,0xfe };
	uint8_t input[4];
	Send4ByteCommand(output,input);

	int32_t value=(input[1]<<16)|(input[2]<<8)|input[3];
	return (value<<8)>>8;
}

static void WriteRegister(int reg,int value)
{
	SetPageForRegister(reg);
	uint8_t command[4]={ 0x40|((reg&0x1f)<<1),value>>16,value>>8,value };
	Send4ByteCommandWithoutInput(command);
}

static void SetPageForRegister(int reg)
{
	int page=reg>>5;
	if(page!=currentpage)
	{
		uint8_t command[4]={ 0x7e,0,0,page };
		Send4ByteCommandWithoutInput(command);
		currentpage=page;
	}
}

static void Send4ByteCommandWithoutInput(const uint8_t output[4])
{
	uint8_t input[4];
	Send4ByteCommand(output,input);
}

static void Send4ByteCommand(const uint8_t output[4],uint8_t input[4])
{
	ioctl(fd,SPI_IOC_MESSAGE(1),&(struct spi_ioc_transfer){
		.tx_buf=(unsigned long)output,
		.rx_buf=(unsigned long)input,
		.len=4,
		.delay_usecs=0,
		.speed_hz=1000000,
		.bits_per_word=8,
	});

	usleep(25);
}

static void Send1ByteCommand(uint8_t command)
{
	uint8_t input;
	ioctl(fd,SPI_IOC_MESSAGE(1),&(struct spi_ioc_transfer){
		.tx_buf=(unsigned long)&command,
		.rx_buf=(unsigned long)&input,
		.len=1,
		.delay_usecs=0,
		.speed_hz=1000000,
		.bits_per_word=8,
	});

	usleep(25);
}


