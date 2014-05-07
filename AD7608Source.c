#include "DataSource.h"
#include "AD7608.h"

#include <stdlib.h>
#include <string.h>

#define NumberOfSamples 128

static DataSource *AllocateDataSource(const char *arguments);
static void FreeDataSource(DataSource *source);
static bool IsDataAvailable(DataSource *source);
static void SampleData(DataSource *source);
static double GetData(DataSource *source,int channel);
static double GetDefaultCalibration(DataSource *source,int channel);
static int ChannelNumberForName(DataSource *source,const char *name);

DataSourceDefinition AD7608SourceDefinition=
{
	.AllocateDataSource=AllocateDataSource,
	.Name="AD7608",
	.Arguments=NULL,
	.ChannelNamesAndUnits=(const char *[]) {
		"0","V",
		"1","V",
		"2","V",
		"3","V",
		"4","V",
		"5","V",
		"6","V",
		"7","V",
		"N*M","Product of two channels. N,M=0..7",
		NULL,
	},
};

typedef struct AD7608Source
{
	DataSource source;
	int samples[8];
} AD7608Source;

static DataSource *AllocateDataSource(const char *arguments)
{
	if(!InitializeAD7608()) return NULL;

	AD7608Source *source=calloc(sizeof(AD7608Source),1);
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
	return true;
}

static void SampleData(DataSource *sourcearg)
{
	AD7608Source *source=(AD7608Source *)sourcearg;

	#if NumberOfSamples==1
	SampleAD7608(source->samples);
	#else
	for(int i=0;i<8;i++) source->samples[i]=0;
	for(int i=0;i<NumberOfSamples;i++)
	{
		int samples[8];
		SampleAD7608(samples);
		for(int j=0;j<8;j++) source->samples[j]+=samples[j];
	}
	#endif
}

static double GetData(DataSource *sourcearg,int channel)
{
	AD7608Source *source=(AD7608Source *)sourcearg;

	if(channel<0||channel>=8+8*8) return 0;
	if(channel<8)
	{
		return source->samples[channel];
	}
	else
	{
		int a=channel%8;
		int b=(channel-8)/8;
		return (double)source->samples[a]*(double)source->samples[b];
	}
}

static double GetDefaultCalibration(DataSource *source,int channel)
{
	if(channel<8) return 10.0/(65536.0*2.0)/NumberOfSamples;
	else return 10.0*10.0/(65536.0*65536.0*2.0*2.0)/(NumberOfSamples*NumberOfSamples);
}

static int ChannelNumberForName(DataSource *source,const char *name)
{
	size_t length=strlen(name);
	if(length==1)
	{
		if(name[0]<'0' || name[0]>'7') return -1;
		return name[0]-'0';
	}
	else if(length==3)
	{
		if(name[0]<'0' || name[0]>'7') return -1;
		if(name[1]!='*') return -1;
		if(name[2]<'0' || name[2]>'7') return -1;

		return (name[0]-'0')*8+name[2]-'0'+8;
	}
	else
	{
		return -1;
	}
}

