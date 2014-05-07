#include "DataSource.h"

#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

static DataSource *AllocateDataSource(const char *arguments);
static void FreeDataSource(DataSource *source);
static bool IsDataAvailable(DataSource *source);
static void SampleData(DataSource *source);
static double GetData(DataSource *source,int channel);
static double GetDefaultCalibration(DataSource *source,int channel);
static int ChannelNumberForName(DataSource *source,const char *name);
static const char **AvailableChannelNamesAndUnits(DataSource *source);

DataSourceDefinition TimeSourceDefinition=
{
	.AllocateDataSource=AllocateDataSource,
	.Name="Time",
	.Arguments=NULL,
	.ChannelNamesAndUnits=(const char *[]) {
		"Elapsed","s",
		"Timestamp","s",
		NULL,
	},
};

typedef struct TimeSource
{
	DataSource source;
	struct timeval start;
	struct timeval now;
} TimeSource;

static DataSource *AllocateDataSource(const char *arguments)
{
	TimeSource *source=calloc(sizeof(TimeSource),1);
	if(!source) exit(2);

	source->source.FreeDataSource=FreeDataSource;
	source->source.IsDataAvailable=IsDataAvailable;
	source->source.SampleData=SampleData;
	source->source.GetData=GetData;
	source->source.GetDefaultCalibration=GetDefaultCalibration;
	source->source.ChannelNumberForName=ChannelNumberForName;
	source->source.IsImportant=false;

	gettimeofday(&source->start,NULL);

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
	TimeSource *source=(TimeSource *)sourcearg;

	gettimeofday(&source->now,NULL);
}

static double GetData(DataSource *sourcearg,int channel)
{
	TimeSource *source=(TimeSource *)sourcearg;

	double start=(double)source->start.tv_sec+(double)source->start.tv_usec/1000000.0;
	double now=(double)source->now.tv_sec+(double)source->now.tv_usec/1000000.0;

	switch(channel)
	{
		case 0: return now-start;
		case 1: return now;
		default: return 0;
	}
}

static double GetDefaultCalibration(DataSource *source,int channel)
{
	return 1;
}

static int ChannelNumberForName(DataSource *source,const char *name)
{
	if(!strcasecmp(name,"Elapsed")) return 0;
	else if(!strcasecmp(name,"Timestamp")) return 1;
	else return -1;
}

