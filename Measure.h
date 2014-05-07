#ifndef __MEASURE_H__
#define __MEASURE_H__

#include "DataSource.h"

#define MaximumStringLength 256
#define NumberOfDataSourceDefinitions 5

extern DataSourceDefinition *AvailableDataSourceDefinitions[NumberOfDataSourceDefinitions];

typedef struct DataSourceListEntry
{
	DataSource *source;
	char name[MaximumStringLength];
} DataSourceListEntry;

typedef struct OutputChannel
{
	DataSource *source;
	int channel;
	double scalefactor;
	double accumulator;
	bool accumulates;
	char format[256];
} OutputChannel;

DataSource *FindOrAllocateDataSourceNamed(const char *sourcename,
DataSourceListEntry *sources,int *numsources);

void InitializeOutputChannel(OutputChannel *output,DataSource *source,
const char *channelname,const char *format,double scalefactor,bool raw,bool accumulates);

void Measure(OutputChannel *channels,int numchannels);

#endif

