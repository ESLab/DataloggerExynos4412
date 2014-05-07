#include "Evaluator.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <math.h>
#include <stdarg.h>

typedef struct Tokenizer
{
	const char *nextcharacter;
	int currenttoken;
	double currentvalue;

	char errorstring[1024];
	jmp_buf jmp;
} Tokenizer;

typedef struct ExpressionParser ExpressionParser;

typedef double NullDenotationFunction(ExpressionParser *self);
typedef double LeftDenotationFunction(ExpressionParser *self,double left);

struct ExpressionParser
{
	Tokenizer *tokenizer;
	NullDenotationFunction *nulldenotationfunctions[256];
	LeftDenotationFunction *leftdenotationfunctions[256];
	int leftbindingpower[256];
};

static double InitializeExpressionParser(ExpressionParser *self,Tokenizer *tokenizer);
static void AddNullDenotation(ExpressionParser *self,int token,NullDenotationFunction *func);
static void AddLeftDenotation(ExpressionParser *self,int token,LeftDenotationFunction *func,int bindingpower);
static double ParseExpression(ExpressionParser *self,int rightbindingpower);

static double ParseParenthesis(ExpressionParser *self);
static double ParseNumber(ExpressionParser *self);
static double ParseNegationOperator(ExpressionParser *self);
static double ParseAdditionOperator(ExpressionParser *self,double left);
static double ParseSubtractionOperator(ExpressionParser *self,double left);
static double ParseMultiplicationOperator(ExpressionParser *self,double left);
static double ParseDivisionOperator(ExpressionParser *self,double left);
static double ParseModuloOperator(ExpressionParser *self,double left);
static double ParseExponentiationOperator(ExpressionParser *self,double left);

static void InitializeTokenizer(Tokenizer *self,const char *string);
static int NextToken(Tokenizer *self);
static int NextTokenAssuming(Tokenizer *self,int assumedcurrent);
static int CurrentToken(Tokenizer *self);
static double CurrentTokenValue(Tokenizer *self);

#define SetTokenizerErrorJumpPoint(self) (setjmp((self)->jmp))
static void EmitTokenizerError(Tokenizer *self,const char *string,...);
static const char *TokenizerErrorString(Tokenizer *self);



double EvaluateExpression(const char *expression,size_t length)
{
	char *string=malloc(length+1);
	if(!string) { fprintf(stderr,"Out of memory.\n"); exit(2); }

	memcpy(string,expression,length);
	string[length]=0;

	Tokenizer tokenizer;
	InitializeTokenizer(&tokenizer,string);
	NextToken(&tokenizer);

	ExpressionParser parser;
	InitializeExpressionParser(&parser,&tokenizer);

	AddNullDenotation(&parser,'N',ParseNumber);
	AddNullDenotation(&parser,'(',ParseParenthesis);
	AddNullDenotation(&parser,'-',ParseNegationOperator);
	AddLeftDenotation(&parser,'+',ParseAdditionOperator,1);
	AddLeftDenotation(&parser,'-',ParseSubtractionOperator,2);
	AddLeftDenotation(&parser,'*',ParseMultiplicationOperator,2);
	AddLeftDenotation(&parser,'/',ParseDivisionOperator,2);
	AddLeftDenotation(&parser,'%',ParseModuloOperator,2);
	AddLeftDenotation(&parser,'^',ParseExponentiationOperator,3);

	if(SetTokenizerErrorJumpPoint(&tokenizer))
	{
		fprintf(stderr,"Parse error: %s\n",TokenizerErrorString(&tokenizer));
		exit(1);
	}

	double value=ParseExpression(&parser,0);

	//printf("%s = %f\n",string,value);

	free(string);

	return value;
}




static double InitializeExpressionParser(ExpressionParser *self,Tokenizer *tokenizer)
{
	self->tokenizer=tokenizer;
}

static void AddNullDenotation(ExpressionParser *self,int token,NullDenotationFunction *func)
{
	self->nulldenotationfunctions[token]=func;
}

static void AddLeftDenotation(ExpressionParser *self,int token,LeftDenotationFunction *func,int bindingpower)
{
	self->leftdenotationfunctions[token]=func;
	self->leftbindingpower[token]=bindingpower;
}

static double ParseExpression(ExpressionParser *self,int rightbindingpower)
{
	int token=CurrentToken(self->tokenizer);
	NextToken(self->tokenizer);

	if(token<0) EmitTokenizerError(self->tokenizer,"No input.");
	if(!self->nulldenotationfunctions[token]) EmitTokenizerError(self->tokenizer,"Unexpected token '%c'.",token);
	double left=self->nulldenotationfunctions[token](self);

	for(;;)
	{
		int token=CurrentToken(self->tokenizer);

		if(token<0) break;
		if(rightbindingpower>=self->leftbindingpower[token]) break;

		NextToken(self->tokenizer);

		if(!self->leftdenotationfunctions[token]) EmitTokenizerError(self->tokenizer,"Unexpected token '%c'.",token);
		left=self->leftdenotationfunctions[token](self,left);
	}

	return left;
}




static double ParseParenthesis(ExpressionParser *self)
{
	double value=ParseExpression(self,0);
	NextTokenAssuming(self->tokenizer,')');
	return value;
}

static double ParseNumber(ExpressionParser *self)
{
	double value=CurrentTokenValue(self->tokenizer);
	return value;
}

static double ParseNegationOperator(ExpressionParser *self)
{
	double value=ParseExpression(self,100);
	return -value;
}

static double ParseAdditionOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,1);
	return left+right;
}

static double ParseSubtractionOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,1);
	return left-right;
}

static double ParseMultiplicationOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,2);
	return left*right;
}

static double ParseDivisionOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,2);
	return left/right;
}

static double ParseModuloOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,2);
	return fmod(left,right);
}

static double ParseExponentiationOperator(ExpressionParser *self,double left)
{
	double right=ParseExpression(self,3-1);
	return pow(left,right);
}





static void InitializeTokenizer(Tokenizer *self,const char *string)
{
	self->nextcharacter=string;
	self->currenttoken=0;
	self->currentvalue=0;
}

static int NextToken(Tokenizer *self)
{
	while(*self->nextcharacter!=0 && isspace(*self->nextcharacter)) self->nextcharacter++;

	switch(*self->nextcharacter)
	{
		case 0: self->currenttoken=0; break;

		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '^':
		case '(':
		case ')':
			self->currenttoken=*self->nextcharacter;
			self->nextcharacter++;
		break;

		case '.':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		{
			char *end;
			self->currentvalue=strtod(self->nextcharacter,&end);
			self->currenttoken='N';
			self->nextcharacter=end;
		}
		break;

		default:
			EmitTokenizerError(self,"Unexpected character '%c'.",*self->nextcharacter);
		break;
	}

	return self->currenttoken;
}

static int NextTokenAssuming(Tokenizer *self,int assumedcurrent)
{
	if(self->currenttoken!=assumedcurrent) EmitTokenizerError(self,"Expected token '%c', found token '%c'.",assumedcurrent,self->currenttoken);
	return NextToken(self);
}

static int CurrentToken(Tokenizer *self)
{
	return self->currenttoken;
}

static double CurrentTokenValue(Tokenizer *self)
{
	return self->currentvalue;
}

static void EmitTokenizerError(Tokenizer *self,const char *format,...)
{
	va_list va;
	va_start(va,format);
	vsnprintf(self->errorstring,sizeof(self->errorstring),format,va);
	va_end(va);

	longjmp(self->jmp,1);
}

static const char *TokenizerErrorString(Tokenizer *self)
{
	return self->errorstring;
}

