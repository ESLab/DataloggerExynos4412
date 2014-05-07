#include "Config.h"
#include "Measure.h"
#include "Evaluator.h"

#include "jsmn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MaximumJSONTokens 1024

static bool IsJSMNStringTokenEqualTo(jsmntok_t *token,const char *string,const char *jsonstring,const char *filename,int n);
static void CopyJSMNStringTokenToString(jsmntok_t *token,char *string,const char *jsonstring,const char *filename,int n);
static double EvaluateJSMNPrimitiveToken(jsmntok_t *token,const char *jsonstring,const char *filename,int n);
static bool ParseJSMNPrimitiveTokenAsBool(jsmntok_t *token,const char *jsonstring,const char *filename,int n);

void UseConfigurationFile(const char *filename)
{
	FILE *fh=fopen(filename,"r");
	if(!fh)
	{
		fprintf(stderr,"Couldn't open configuration file \"%s\".\n",filename);
		exit(1);
	}

	fseek(fh,0,SEEK_END);
	long length=ftell(fh);
	fseek(fh,0,SEEK_SET);

	char configbytes[length+1];
	if(fread(configbytes,1,length,fh)!=length)
	{
		fprintf(stderr,"Failed to read from configuration file \"%s\".\n",filename);
		exit(1);
	}
	configbytes[length]=0;

	jsmn_parser parser;
	jsmn_init(&parser);

	jsmntok_t tokens[MaximumJSONTokens];
	jsmnerr_t error=jsmn_parse(&parser,configbytes,tokens,MaximumJSONTokens);

	switch(error)
	{
		case JSMN_ERROR_NOMEM:
			fprintf(stderr,"Config file \"%s\" is too complex.\n",filename);
			exit(1);
		case JSMN_ERROR_INVAL:
			fprintf(stderr,"Error while parsing config file \"%s\": Invalid character.\n",filename);
			exit(1);
		case JSMN_ERROR_PART:
			fprintf(stderr,"Error while parsing config file \"%s\": File ends prematurely.\n",filename);
			exit(1);
	}

	if(tokens[0].type!=JSMN_ARRAY)
	{
		fprintf(stderr,"Error while parsing config file \"%s\": The config file must consist of a single array of objects, one for each output channel.\n",filename);
		exit(1);
	}

	int numoutputs=tokens[0].size;
	OutputChannel outputs[numoutputs];

	int numsources=0;
	DataSourceListEntry sources[numoutputs];

	char titles[numoutputs][MaximumStringLength];
	bool hastitles=false;
	memset(titles,0,sizeof(titles));

	jsmntok_t *token=&tokens[1];
	for(int i=0;i<numoutputs;i++)
	{
		if(token->type!=JSMN_OBJECT)
		{
			fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: The config file must consist of a single array of objects, one for each output channel.\n",filename,i);
			exit(1);
		}

		int numentries=token->size/2;

		token++;

		char sourcename[MaximumStringLength]={0};
		char channelname[MaximumStringLength]={0};
		char format[MaximumStringLength]={0};
		double scalefactor=1;
		bool raw=false;
		bool accumulates=false;

		for(int j=0;j<numentries;j++)
		{
			if(IsJSMNStringTokenEqualTo(token,"source",configbytes,filename,i))
			{
				CopyJSMNStringTokenToString(&token[1],sourcename,configbytes,filename,i);
			}
			else if(IsJSMNStringTokenEqualTo(token,"channel",configbytes,filename,i))
			{
				CopyJSMNStringTokenToString(&token[1],channelname,configbytes,filename,i);
			}
			else if(IsJSMNStringTokenEqualTo(token,"format",configbytes,filename,i))
			{
				CopyJSMNStringTokenToString(&token[1],format,configbytes,filename,i);
			}
			else if(IsJSMNStringTokenEqualTo(token,"title",configbytes,filename,i))
			{
				CopyJSMNStringTokenToString(&token[1],titles[i],configbytes,filename,i);
				hastitles=true;
			}
			else if(IsJSMNStringTokenEqualTo(token,"calibration",configbytes,filename,i) ||
			IsJSMNStringTokenEqualTo(token,"scalefactor",configbytes,filename,i))
			{
				scalefactor=EvaluateJSMNPrimitiveToken(&token[1],configbytes,filename,i);
			}
			else if(IsJSMNStringTokenEqualTo(token,"raw",configbytes,filename,i))
			{
				raw=ParseJSMNPrimitiveTokenAsBool(&token[1],configbytes,filename,i);
			}
			else if(IsJSMNStringTokenEqualTo(token,"accumulates",configbytes,filename,i))
			{
				accumulates=ParseJSMNPrimitiveTokenAsBool(&token[1],configbytes,filename,i);
			}
			else
			{
				char string[MaximumStringLength];
				int length=token->end-token->start;
				if(length>MaximumStringLength-1) length=MaximumStringLength-1;
				memcpy(string,&configbytes[token->start],length);
				string[length]=0;
				fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Unknown field \"%s\".\n",filename,i,string);
				exit(1);
			}

			token+=2;
		}

		DataSource *source=FindOrAllocateDataSourceNamed(sourcename,sources,&numsources);
		InitializeOutputChannel(&outputs[i],source,channelname,format,scalefactor,raw,accumulates);
	}

	if(hastitles)
	{
		for(int i=0;i<numoutputs;i++)
		{
			if(i!=0) printf(",");
			printf("%s",titles[i]);
		}
		printf("\n");
	}

	Measure(outputs,numoutputs);
}

static bool IsJSMNStringTokenEqualTo(jsmntok_t *token,const char *string,const char *jsonstring,const char *filename,int n)
{
	if(token->type!=JSMN_STRING && token->type!=JSMN_PRIMITIVE)
	{
		fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Invalid field name.\n",filename,n);
		exit(1);
	}

	int length=token->end-token->start;
	if(length!=strlen(string)) return false;
	return memcmp(&jsonstring[token->start],string,length)==0;
}

static void CopyJSMNStringTokenToString(jsmntok_t *token,char *string,const char *jsonstring,const char *filename,int n)
{
	if(token->type!=JSMN_STRING)
	{
		fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Expected a string.\n",filename,n);
		exit(1);
	}

	int length=token->end-token->start;
	if(length>MaximumStringLength-1) length=MaximumStringLength-1;
	memcpy(string,&jsonstring[token->start],length);
	string[length]=0;
}

static double EvaluateJSMNPrimitiveToken(jsmntok_t *token,const char *jsonstring,const char *filename,int n)
{
	char c=jsonstring[token->start];
	if(token->type!=JSMN_PRIMITIVE)
	{
		fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Expected an expression.\n",filename,n);
		exit(1);
	}

	return EvaluateExpression(&jsonstring[token->start],token->end-token->start);
}

static bool ParseJSMNPrimitiveTokenAsBool(jsmntok_t *token,const char *jsonstring,const char *filename,int n)
{
	if(token->type!=JSMN_PRIMITIVE)
	{
		fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Expected a boolean.\n",filename,n);
		exit(1);
	}

	int length=token->end-token->start;
	if(length==4 && memcmp(&jsonstring[token->start],"true",4)==0)
	{
		return true;
	}
	else if(length==5 && memcmp(&jsonstring[token->start],"false",5)==0)
	{
		return false;
	}
	else
	{
		fprintf(stderr,"Error while parsing config file \"%s\", output channel %d: Expected a boolean.\n",filename,n);
		exit(1);
	}
}

