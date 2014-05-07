#ifndef __CS5467_H__
#define __CS5467_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct RawCS5467Sample
{
	int rms_voltage,rms_current;
	int active_power,reactive_power,apparent_power;
	int power_factor;
	int wideband_reactive_power;
	int active_energy_pulse,reactive_energy_pulse,apparent_energy_pulse;
	int status;
} RawCS5467Sample;

typedef struct CS5467Sample
{
	float rms_voltage,rms_current;
	float active_power,reactive_power,apparent_power;
	float power_factor;
	float wideband_reactive_power;
	float active_energy_pulse,reactive_energy_pulse,apparent_energy_pulse;
	int status;
} CS5467Sample;

bool InitializeCS5467();

void StartSamplingCS5467();
bool IsCS5467DataReady();
RawCS5467Sample NextRawCS5467Sample();
CS5467Sample NextCS5467Sample();
CS5467Sample CalibrateRawCS5467Sample(RawCS5467Sample rawsample);

#define CS5467VoltageCalibration (7.071/(float)0x04a71c*231.9/229.3)
#define CS5467CurrentCalibration (10.0/(float)0x04a71c*6.05/318.981)
#define CS5467PowerCalibration (CS5467VoltageCalibration*CS5467CurrentCalibration*(float)0x2000000)
#define CS5467PowerFactorCalibration (1.0/(float)0x800000)
#define CS5467EnergyCalibration (CS5467PowerCalibration*2)

#endif

