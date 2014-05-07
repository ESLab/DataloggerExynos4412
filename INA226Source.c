#include "DataSource.h"
#include "INA226.h"
#include "Evaluator.h"

#include <stdlib.h>
#include <string.h>

static DataSource *AllocateDataSource(const char *arguments);
static void FreeDataSource(DataSource *source);
static bool IsDataAvailable(DataSource *source);
static void SampleData(DataSource *source);
static double GetData(DataSource *source,int channel);
static double GetDefaultCalibration(DataSource *source,int channel);
static int ChannelNumberForName(DataSource *source,const char *name);

DataSourceDefinition INA226SourceDefinition=
{
	.AllocateDataSource=AllocateDataSource,
	.Name="INA226",
	.Arguments="I2C address",
	.ChannelNamesAndUnits=(const char *[]) {
		"Voltage","V",
		"Current","A",
		"Power","W",
		NULL,
	},
};

typedef struct INA226Source
{
	DataSource source;
	int address;
	bool ready;
	int voltage,current,power;
} INA226Source;

static DataSource *AllocateDataSource(const char *arguments)
{
	int address=EvaluateExpression(arguments,strlen(arguments));
	if(!InitializeINA226(address,0x1000)) return NULL;

	INA226Source *source=calloc(sizeof(INA226Source),1);
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
	INA226Source *source=(INA226Source *)sourcearg;

	if(source->ready) return true;
	source->ready=ReadAndClearINA226ReadyFlag(source->address);
	return source->ready;
}

static void SampleData(DataSource *sourcearg)
{
	INA226Source *source=(INA226Source *)sourcearg;

	source->voltage=ReadINA226Voltage(source->address);
	source->current=ReadINA226Current(source->address);
	source->power=ReadINA226Power(source->address);
	source->ready=false;
}

static double GetData(DataSource *sourcearg,int channel)
{
	INA226Source *source=(INA226Source *)sourcearg;

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
		case 0: return 0.00125;
		case 1: return 0.00125/2;
		case 2: return 0.00125*0.00125/2*20000;
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

