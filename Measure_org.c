#include "Measure.h"
#include "AD7608Source.h"
#include "CS5467Source.h"
#include "INA226Source.h"
#include "INA220Source.h"
#include "TimeSource.h"
#include "DataSourceHelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DataSourceDefinition *AvailableDataSourceDefinitions[NumberOfDataSourceDefinitions]=
{
	&AD7608SourceDefinition,
	&CS5467SourceDefinition,
	&INA226SourceDefinition,
	&INA220SourceDefinition,
	&TimeSourceDefinition,
};

DataSource *FindOrAllocateDataSourceNamed(const char *sourcename,
DataSourceListEntry *sources,int *numsources)
{
	for(int i=0;i<*numsources;i++)
	{
		if(strcasecmp(sourcename,sources[i].name)==0) return sources[i].source;
	}

	char namepart[MaximumStringLength];
	const char *argumentspart;

	const char *colon=strchr(sourcename,':');
	if(colon)
	{
		size_t namelength=colon-sourcename;
		memcpy(namepart,sourcename,namelength);
		namepart[namelength]=0;
		argumentspart=&sourcename[namelength+1];
	}
	else
	{
		strcpy(namepart,sourcename);
		argumentspart=NULL;
	}

	DataSourceDefinition *definition=NULL;
	for(int i=0;i<NumberOfDataSourceDefinitions;i++)
	{
		if(strcasecmp(namepart,DataSourceDefinitionName(AvailableDataSourceDefinitions[i]))==0)
		{
			definition=AvailableDataSourceDefinitions[i];
			break;
		}
	}

	if(!definition)
	{
		fprintf(stderr,"There is no data source named \"%s\" available.\n",namepart);
		exit(1);
	}

	DataSource *source=AllocateDataSource(definition,argumentspart);
	if(!source)
	{
		fprintf(stderr,"Failed to initialize data source \"%s\" with arguments \"%s\".\n",namepart,argumentspart);
		exit(1);
	}

	sources[*numsources].source=source;
	strcpy(sources[*numsources].name,sourcename);
	(*numsources)++;

	return source;
}

void InitializeOutputChannel(OutputChannel *output,DataSource *source,
const char *channelname,const char *format,double scalefactor,bool raw,bool accumulates)
{
	int channelnumber=ChannelNumberForName(source,channelname);
	if(channelnumber<0)
	{
		fprintf(stderr,"The data source \"%s\" has no channel named \"%s\".\n",DataSourceName(source),channelname);
		exit(1);
	}

	if(!raw) scalefactor*=GetDefaultCalibration(source,channelnumber);

	output->source=source;
	output->channel=channelnumber;
	output->scalefactor=scalefactor;
	output->accumulator=0;
	output->accumulates=accumulates;
	strcpy(output->format,format);
}

void Measure(OutputChannel *outputs,int numoutputs)
{
	DataSource *sources[numoutputs];
	int numsources=0;

	for(int i=0;i<numoutputs;i++)
	{
		bool found=false;
		for(int j=0;j<numsources;j++)
		{
			if(outputs[i].source==sources[j]) { found=true; break; }
		}

		if(!found)
		{
			sources[numsources]=outputs[i].source;
			numsources++;
		}
	}

	for(;;)
	{
		bool didupdate=false;
		while(!didupdate)
		{
			for(int i=0;i<numsources;i++)
			{
				if(IsDataAvailable(sources[i]))
				{
					SampleData(sources[i]);
					if(IsDataSourceImportant(sources[i])) didupdate=true;

					for(int j=0;j<numoutputs;j++)
					{
						if(outputs[j].source==sources[i] && outputs[j].accumulates)
						{
							double value=GetData(outputs[j].source,outputs[j].channel);
							outputs[j].accumulator+=value;
						}
					}
				}
			}
		}

		for(int i=0;i<numoutputs;i++)
		{
			if(i!=0) printf(",");

			if(outputs[i].accumulates)
			{
				printf(outputs[i].format,outputs[i].accumulator*outputs[i].scalefactor);
			}
			else
			{
				double value=GetData(outputs[i].source,outputs[i].channel);
				printf(outputs[i].format,value*outputs[i].scalefactor);
			}
		}

		printf("\n");
	}
}

