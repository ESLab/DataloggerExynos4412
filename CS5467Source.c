#include "DataSource.h"
#include "CS5467.h"

#include <stdlib.h>
#include <string.h>

static DataSource *AllocateDataSource(const char *arguments);
static void FreeDataSource(DataSource *source);
static bool IsDataAvailable(DataSource *source);
static void SampleData(DataSource *source);
static double GetData(DataSource *source,int channel);
static double GetDefaultCalibration(DataSource *source,int channel);
static int ChannelNumberForName(DataSource *source,const char *name);

DataSourceDefinition CS5467SourceDefinition=
{
	.AllocateDataSource=AllocateDataSource,
	.Name="CS5467",
	.Arguments=NULL,
	.ChannelNamesAndUnits=(const char *[]) {
		"RMS voltage","V",
		"RMS current","A",
		"Active power","W",
		"Reactive power","W",
		"Apparent power","W",
		"Power factor","",
		"Wideband reactive power","W",
		"Active energy pulse","J",
		"Reactive energy pulse","J",
		"Apparent energy pulse","J",
		NULL,
	},
};

typedef struct CS5467Source
{
	DataSource source;
	RawCS5467Sample rawsample;
	CS5467Sample sample;
} CS5467Source;

static DataSource *AllocateDataSource(const char *arguments)
{
	if(!InitializeCS5467()) return NULL;
	StartSamplingCS5467();

	CS5467Source *source=calloc(sizeof(CS5467Source),1);
	if(!source) exit(2);

	source->source.FreeDataSource=FreeDataSource;
	source->source.IsDataAvailable=IsDataAvailable;
	source->source.SampleData=SampleData;
	source->source.GetData=GetData;
	source->source.GetDefaultCalibration=GetDefaultCalibration;
	source->source.ChannelNumberForName=ChannelNumberForName;
	source->source.IsImportant=true;

	return &source->source;
}

static void FreeDataSource(DataSource *source)
{
	free(source);
}

static bool IsDataAvailable(DataSource *source)
{
	return IsCS5467DataReady();
}

static void SampleData(DataSource *sourcearg)
{
	CS5467Source *source=(CS5467Source *)sourcearg;

	source->rawsample=NextRawCS5467Sample();
	source->sample=CalibrateRawCS5467Sample(source->rawsample);
}

static double GetData(DataSource *sourcearg,int channel)
{
	CS5467Source *source=(CS5467Source *)sourcearg;

	switch(channel)
	{
		case 0: return source->rawsample.rms_voltage;
		case 1: return source->rawsample.rms_current;
		case 2: return source->rawsample.active_power;
		case 3: return source->rawsample.reactive_power;
		case 4: return source->rawsample.apparent_power;
		case 5: return source->rawsample.power_factor;
		case 6: return source->rawsample.wideband_reactive_power;
		case 7: return source->rawsample.active_energy_pulse;
		case 8: return source->rawsample.reactive_energy_pulse;
		case 9: return source->rawsample.apparent_energy_pulse;
		default: return 0;
	}
}

static double GetDefaultCalibration(DataSource *source,int channel)
{
	switch(channel)
	{
		case 0: return CS5467VoltageCalibration;
		case 1: return CS5467CurrentCalibration;
		case 2:
		case 3:
		case 4: return CS5467PowerCalibration;
		case 5: return CS5467PowerFactorCalibration;
		case 6: return CS5467PowerCalibration;
		case 7:
		case 8:
		case 9: return CS5467EnergyCalibration;
		default: return 0;
	}
}

static int ChannelNumberForName(DataSource *source,const char *name)
{
	if(!strcasecmp(name,"RMS voltage")) return 0;
	else if(!strcasecmp(name,"RMS current")) return 1;
	else if(!strcasecmp(name,"Active power")) return 2;
	else if(!strcasecmp(name,"Reactive power")) return 3;
	else if(!strcasecmp(name,"Apparent power")) return 4;
	else if(!strcasecmp(name,"Power factor")) return 5;
	else if(!strcasecmp(name,"Wideband reactive power")) return 6;
	else if(!strcasecmp(name,"Active energy pulse")) return 7;
	else if(!strcasecmp(name,"Reactive energy pulse")) return 8;
	else if(!strcasecmp(name,"Apparent energy pulse")) return 9;
	else return -1;
}

