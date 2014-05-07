#ifndef __DATA_SOURCE_H__
#define __DATA_SOURCE_H__

#include <stdbool.h>

typedef struct DataSourceDefinition DataSourceDefinition;
typedef struct DataSource DataSource;

typedef DataSource *AllocateDataSourceFunction(const char *arguments);
typedef void FreeDataSourceFunction(DataSource *source);
typedef bool IsDataAvailableFunction(DataSource *source);
typedef void SampleDataFunction(DataSource *source);
typedef double GetDataFunction(DataSource *source,int channel);
typedef double GetDefaultCalibrationFunction(DataSource *source,int channel);
typedef int ChannelNumberForNameFunction(DataSource *source,const char *name);

struct DataSourceDefinition
{
	AllocateDataSourceFunction *AllocateDataSource;
	const char *Name,*Arguments;
	const char **ChannelNamesAndUnits;
};

struct DataSource
{
	FreeDataSourceFunction *FreeDataSource;
	IsDataAvailableFunction *IsDataAvailable;
	SampleDataFunction *SampleData;
	GetDataFunction *GetData;
	GetDefaultCalibrationFunction *GetDefaultCalibration;
	ChannelNumberForNameFunction *ChannelNumberForName;
	bool IsImportant;
	DataSourceDefinition *definition;
};

#endif

