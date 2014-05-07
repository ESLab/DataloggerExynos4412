#ifndef __DATA_SOURCE_HELPERS_H__
#define __DATA_SOURCE_HELPERS_H__

#include "DataSource.h"

static inline DataSource *AllocateDataSource(DataSourceDefinition *definition,const char *arguments)
{
	DataSource *source=definition->AllocateDataSource(arguments);
	source->definition=definition;
	return source;
}

static inline const char **AvailableChannelNamesAndUnits(DataSourceDefinition *definition) { return definition->ChannelNamesAndUnits; }
static inline const char *DataSourceDefinitionName(DataSourceDefinition *definition) { return definition->Name; }
static inline const char *DataSourceDefinitionArguments(DataSourceDefinition *definition) { return definition->Arguments; }

static inline void FreeDataSource(DataSource *source) { source->FreeDataSource(source); }
static inline bool IsDataAvailable(DataSource *source) { return source->IsDataAvailable(source); }
static inline void SampleData(DataSource *source) { source->SampleData(source); }
static inline double GetData(DataSource *source,int channel) { return source->GetData(source,channel); }
static inline double GetDefaultCalibration(DataSource *source,int channel) { return source->GetDefaultCalibration(source,channel); }
static inline int ChannelNumberForName(DataSource *source,const char *name) { return source->ChannelNumberForName(source,name); }
static inline bool IsDataSourceImportant(DataSource *source) { return source->IsImportant; }
static inline const char *DataSourceName(DataSource *source) { return source->definition->Name; }
static inline const char *DataSourceArguments(DataSource *source) { return source->definition->Arguments; }

#endif

