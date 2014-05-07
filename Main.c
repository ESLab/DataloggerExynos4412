#include "Measure.h"
#include "Config.h"
#include "DataSourceHelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void PrintUsage(const char *name);
static void DoSimpleMeasurement(int argc,const char **argv);

int main(int argc,const char **argv)
{
	if(argc<=1) PrintUsage(argv[0]);
	else if(argc==2) UseConfigurationFile(argv[1]);
	else DoSimpleMeasurement(argc,argv);
}

static void PrintUsage(const char *name)
{
	fprintf(stderr,"Usage: %s configurationfile\n",name);
	fprintf(stderr,"       %s source1 channel1 [source2 channel2 ...]\n",name);
	fprintf(stderr,"\n");
	fprintf(stderr,"Available sources and channels:\n");

	for(int i=0;i<NumberOfDataSourceDefinitions;i++)
	{
		fprintf(stderr,"  * %s",DataSourceDefinitionName(AvailableDataSourceDefinitions[i]));
		const char *arguments=DataSourceDefinitionArguments(AvailableDataSourceDefinitions[i]);
		if(arguments) fprintf(stderr," (Arguments: %s)",arguments);
		fprintf(stderr,"\n");

		const char **channels=AvailableChannelNamesAndUnits(AvailableDataSourceDefinitions[i]);
		while(channels[0])
		{
			fprintf(stderr,"    - %s",channels[0]);
			if(channels[1] && channels[1][0]) fprintf(stderr," (%s)",channels[1]);
			fprintf(stderr,"\n");

			channels+=2;
		}
	}

	exit(1);
}

static void DoSimpleMeasurement(int argc,const char **argv)
{
	int numoutputs=(argc-1)/2;
	OutputChannel outputs[numoutputs];
	int numsources=0;
	DataSourceListEntry sources[numoutputs];

	for(int i=0;i<numoutputs;i++)
	{
		DataSource *source=FindOrAllocateDataSourceNamed(argv[2*i+1],sources,&numsources);
		InitializeOutputChannel(&outputs[i],source,argv[2*i+2],"%f",1,false,false);
	}

	Measure(outputs,numoutputs);
}

