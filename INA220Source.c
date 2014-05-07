#include "DataSource.h"
#include "INA220.h"
#include "Evaluator.h"

#include <stdlib.h>
#include <string.h>

#define CurrentFactor 5
#define ShuntResistance 0.02

static DataSource *AllocateDataSource(const char *arguments);
static void FreeDataSource(DataSource *source);
static bool IsDataAvailable();
static void SampleData(DataSource *source);
static double GetData(DataSource *source,int channel);
static double GetDefaultCalibration(DataSource *source,int channel);
static int ChannelNumberForName(DataSource *source,const char *name);

DataSourceDefinition INA220SourceDefinition=
{
	.AllocateDataSource=AllocateDataSource,
	.Name="INA220",
	.Arguments="I2C address",
	.ChannelNamesAndUnits=(const char *[]) {
		"Voltage","V",
		"Current","A",
		"Power","W",
		NULL,
	},
};

typedef struct INA220Source
{
	DataSource source;
	int address;
	bool ready;
	int voltage,current,power;
} INA220Source;

static DataSource *AllocateDataSource(const char *arguments)
{
	int address=EvaluateExpression(arguments,strlen(arguments));
	if(!InitializeINA220(address,0xfff*CurrentFactor)) return NULL; // Should this be 0x1000?

	INA220Source *source=calloc(sizeof(INA220Source),1);
	if(!source) exit(2);

	source->source.FreeDataSource=FreeDataSource;
	source->source.IsDataAvailable=IsDataAvailable;
	source->source.SampleData=SampleData;
	source->source.GetData=GetData;
	source->source.GetDefaultCalibration=GetDefaultCalibration;
	source->source.ChannelNumberForName=ChannelNumberForName;
	source->source.IsImportant=true;

	source->address=address;
	source->ready=false;

	return &source->source;
}

static void FreeDataSource(DataSource *source)
{
	free(source);
}

static bool IsDataAvailable(DataSource *sourcearg)
{
	INA220Source *source=(INA220Source *)sourcearg;

	if(source->ready) return true;
	int voltageandflags=ReadINA220VoltageAndFlags(source->address);
	source->voltage=voltageandflags>>3;
	if(voltageandflags&0x0002) source->ready=true;
	return source->ready;
}

static void SampleData(DataSource *sourcearg)
{
	INA220Source *source=(INA220Source *)sourcearg;

	if(source->ready) source->voltage=ReadINA220VoltageAndFlags(source->address)>>3;
	source->current=ReadINA220Current(source->address);
	source->power=ReadINA220Power(source->address);
	source->ready=false;
}

static double GetData(DataSource *sourcearg,int channel)
{
	INA220Source *source=(INA220Source *)sourcearg;

	switch(channel)
	{
		case 0: return source->voltage;
		case 1: return source->current;
		case 2: return source->power;
		default: return 0;
	}
}

static double GetDefaultCalibration(DataSource *source,int channel)
{
	switch(channel)
	{
		case 0: return 0.004;
		case 1: return 0.00001/CurrentFactor/ShuntResistance;
		case 2: return 0.004*0.00001/CurrentFactor/ShuntResistance*5000;
		default: return 0;
	}
}

static int ChannelNumberForName(DataSource *source,const char *name)
{
	if(strcasecmp(name,"Voltage")==0) return 0;
	else if(strcasecmp(name,"Current")==0) return 1;
	else if(strcasecmp(name,"Power")==0) return 2;
	else return -1;
}

