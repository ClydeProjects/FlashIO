/*  This file is part of the uFLIP software. See www.uflip.org

    uFLIP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    uFLIP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with uFLIP.  If not, see <http://www.gnu.org/licenses/>.
   
   	uFLIP was initially developed based on SQLIO2 by Leonard Chung although almost all SQLIO2 
   	code have disappeared. (leonard@ssl.berkeley.edu - see 
   	http://research.microsoft.com/en-us/um/siliconvalley/projects/sequentialio/ )
   	uFLIP also includes some lines from the pseudo random generator from Agner Fog
   	(see http://www.agner.org/random/)
   	
   	©Luc Bouganim - 2008-2009    
*/

#include "input.h"
#include "flashIO.h"
#include "utility.h"
#include <stdlib.h>
#include <time.h>

void GetValuesRF(const char* varName, const char* varValue, sParams* PB){
	//char 			str[MAX_STR];						// Temporary string
    //sprintf(str, "Reading parameter(s) [%s] [%s]\n", varName, varValue);
	//OutputString(OUT_LOG, str);
	if      (strcasecmp(varName, "Dev")==0)				PB->deviceNum 		= atoi(varValue);
	else if (strcasecmp(varName, "Bench")==0)			PB->microBenchID 	= atoi(varValue);
	else if (strcasecmp(varName, "Exp")==0) 			PB->expID 			= atoi(varValue);
	else if (strcasecmp(varName, "Size")==0) 			PB->deviceSize		= ((uint32)(atoi(varValue)/BLOCK))*BLOCK;
	else if (strcasecmp(varName, "Fake")==0) {
		if (strcasecmp(varValue, "True") == 0) 			PB->fake 			= TRUE;
		else if  (strcasecmp(varValue, "False") == 0) 	PB->fake 			= FALSE;
		else PrintHelp(varName);
	}
	else PrintHelp(varName);
}
void GetValuesGP(const char* varName, const char* varValue, sParams* PB){
 
	//char 			str[MAX_STR];						// Temporary string
    //sprintf(str, "Reading parameter(s) [%s] [%s]\n", varName, varValue);
	//OutputString(OUT_LOG, str);
	if (strcasecmp(varName, "OutName")==0) 				strcpy(PB->outName, varValue);
	else if (strcasecmp(varName, "Dev")==0)				PB->deviceNum 		= atoi(varValue);
	else if (strcasecmp(varName, "IOS")==0) 			PB->IOSize 			= atoi(varValue);
	else if (strcasecmp(varName, "Size")==0) 			PB->deviceSize		= ((uint32)(atoi(varValue)/BLOCK))*BLOCK;
	else if (strcasecmp(varName, "IOC")==0)				PB->IOCount 		= atoi(varValue);
	else if (strcasecmp(varName, "IOC2")==0)			PB->IOCount2 		= atoi(varValue);
	else if (strcasecmp(varName, "CErase")==0)			PB->collectErase 	= atoi(varValue);
	else if (strcasecmp(varName, "Pause")==0) 			PB->pauseExp 		= atoi(varValue)* 1000;
	else if (strcasecmp(varName, "NbRun")==0)			PB->nbRun	 		= atoi(varValue);
	else if (strcasecmp(varName, "Fake")==0) {
		if (strcasecmp(varValue, "True") == 0) 			PB->fake 			= TRUE;
		else if  (strcasecmp(varValue, "False") == 0) 	PB->fake 			= FALSE;
		else PrintHelp(varName);
	}
	else PrintHelp(varName);
}
void GetValuesGB(const char* varName, const char* varValue, sParams* PB){
    //char 			str[MAX_STR];						// Temporary string
    //sprintf(str, "Reading parameter(s) [%s] [%s]\n", varName, varValue);
	//OutputString(OUT_LOG, str);
	if (strcasecmp(varName, "ExpPlan")==0) 				strcpy(PB->expPlan, varValue);
	else if (strcasecmp(varName, "ExpSel")==0) 			strcpy(PB->expSelect, varValue);
	else if (strcasecmp(varName, "OutName")==0) 		strcpy(PB->outName, varValue);
	else if (strcasecmp(varName, "NbRun")==0)			PB->nbRun	 		= atoi(varValue);
	else if (strcasecmp(varName, "Dev")==0)				PB->deviceNum 		= atoi(varValue);
	else if (strcasecmp(varName, "IOS")==0) 			PB->IOSize 			= atoi(varValue);
	else if (strcasecmp(varName, "Size")==0) 			PB->deviceSize		= ((uint32)(atoi(varValue)/BLOCK))*BLOCK;
	else if (strcasecmp(varName, "IOCSR")==0)			PB->IOCountSR 		= atoi(varValue);
	else if (strcasecmp(varName, "IOCRR")==0)			PB->IOCountRR 		= atoi(varValue);
	else if (strcasecmp(varName, "IOCSW")==0)			PB->IOCountSW 		= atoi(varValue);
	else if (strcasecmp(varName, "IOCRW")==0)			PB->IOCountRW 		= atoi(varValue);
	else if (strcasecmp(varName, "IgnIOSR")==0) 		PB->ignoreIOSR 		= atoi(varValue);
	else if (strcasecmp(varName, "IgnIORR")==0) 		PB->ignoreIORR 		= atoi(varValue);
	else if (strcasecmp(varName, "IgnIOSW")==0) 		PB->ignoreIOSW 		= atoi(varValue);
	else if (strcasecmp(varName, "IgnIORW")==0) 		PB->ignoreIORW 		= atoi(varValue);
	else if (strcasecmp(varName, "CErase")==0)			PB->collectErase 	= atoi(varValue);
	else if (strcasecmp(varName, "Pause")==0) 			PB->pauseExp 		= atoi(varValue);
	else if (strcasecmp(varName, "Fake")==0) {
		if (strcasecmp(varValue, "True") == 0) 			PB->fake 			= TRUE;
		else if  (strcasecmp(varValue, "False") == 0) 	PB->fake 			= FALSE;
		else PrintHelp(varName);
	}
	else if (strcasecmp(varName, "TrimBeforeRun")==0) {
			if (strcasecmp(varValue, "True") == 0) 			PB->trimBeforeRun 			= TRUE;
			else if  (strcasecmp(varValue, "False") == 0) 	PB->trimBeforeRun 			= FALSE;
			else PrintHelp(varName);
		}
	else if (strcasecmp(varName, "BufferType")==0) {
		if (strcasecmp(varName, "N")==0) 				PB->bufferType 	= NO_BUFFERING;
		else if (strcasecmp(varValue, "H")==0)			PB->bufferType = HW_BUFFERING;
		else if (strcasecmp(varValue, "S")==0)			PB->bufferType = FS_BUFFERING;
		else if (strcasecmp(varValue, "A")==0)			PB->bufferType = HW_BUFFERING | FS_BUFFERING;
		else PrintHelp(varName);
	}
	else PrintHelp(varName);
}

int32 GetValues(const char* varName, const char* varValue, sParams* PB, char* valueList, int32* tabVal){
	
    //char 			str[MAX_STR];						// Temporary string
    //sprintf(str, "Reading parameter(s) [%s] [%s] [%s]\n", varName, varValue, valueList);
	//OutputString(OUT_LOG, str);
	
	if      (strcasecmp(varName, "Dev")==0)				PB->deviceNum 		= atoi(varValue);
	else if (strcasecmp(varName, "IOS")==0) 			PB->IOSize 			= atoi(varValue);
	else if (strcasecmp(varName, "Ratio")==0) 			PB->ratio 			= atoi(varValue);
	else if (strcasecmp(varName, "Size")==0) 			PB->deviceSize		= ((uint32)(atoi(varValue)/BLOCK))*BLOCK;
	else if (strcasecmp(varName, "IOC")==0)				PB->IOCount 		= atoi(varValue);
	else if (strcasecmp(varName, "IOC2")==0)			PB->IOCount2 		= atoi(varValue);
	else if (strcasecmp(varName, "TSize")==0)			PB->targetSize 		= atoi(varValue);
	else if (strcasecmp(varName, "TOffs")==0)			PB->targetOffset 	= atoi(varValue);
	else if (strcasecmp(varName, "TSize2")==0)			PB->targetSize2 	= atoi(varValue);
	else if (strcasecmp(varName, "TOffs2")==0)			PB->targetOffset2 	= atoi(varValue);
	else if (strcasecmp(varName, "Part")==0) 			PB->nbPartition 	= atoi(varValue);
	else if (strcasecmp(varName, "Shift")==0) 			PB->IOShift 		= atoi(varValue);
	else if (strcasecmp(varName, "IgnIO")==0) 			PB->ignoreIO 		= atoi(varValue);
	else if (strcasecmp(varName, "Base")==0) 			strcpy (PB->base, varValue);
	else if (strcasecmp(varName, "Base2")==0) 			strcpy (PB->base2, varValue);
	else if (strcasecmp(varName, "Order")==0) 			PB->order 			= atoi(varValue);
	else if (strcasecmp(varName, "PIO")==0)				PB->pauseIO 		= atol(varValue);
	else if (strcasecmp(varName, "PBurst")==0)			PB->pauseBurst 		= atol(varValue);
	else if (strcasecmp(varName, "BurstIO")==0) 		PB->burstIO 		= atoi(varValue);
	else if (strcasecmp(varName, "ParDeg")==0) 			PB->parDeg 			= atoi(varValue);
	else if (strcasecmp(varName, "PID")==0)				PB->processID 		= atoi(varValue);
	else if (strcasecmp(varName, "CErase")==0)			PB->collectErase 	= atoi(varValue);
	else if (strcasecmp(varName, "Pause")==0) 			PB->pauseExp 		= atoi(varValue)* 1000;
	else if (strcasecmp(varName, "Fake")==0) {
		if (strcasecmp(varValue, "True") == 0) 			PB->fake 			= TRUE;
		else if  (strcasecmp(varValue, "False") == 0) 	PB->fake 			= FALSE;
		else PrintHelp(varName);
	}
	else if (strcasecmp(varName, "BufferType")==0) {
		if (strcasecmp(varName, "N")==0) 				PB->bufferType = NO_BUFFERING;
		else if (strcasecmp(varValue, "H")==0)			PB->bufferType = HW_BUFFERING;
		else if (strcasecmp(varValue, "S")==0)			PB->bufferType = FS_BUFFERING;
		else if (strcasecmp(varValue, "A")==0)			PB->bufferType = HW_BUFFERING | FS_BUFFERING;
		else PrintHelp(varName);
	}
	else if (strcasecmp(varName, "Bench")==0)			PB->microBenchID 	= atoi(varValue);
	else if (strcasecmp(varName, "Exp")==0) 			PB->expID 			= atoi(varValue);
	else if (strcasecmp(varName, "Run")==0) 			PB->runID 			= atoi(varValue);
	else if (strcasecmp(varName, "Warn")==0) 			PB->warning			= atoi(varValue);
	else if (strcasecmp(varName, "Key")==0) 			PB->key				= atoi(varValue);
	else if (strcasecmp(varName, "Com")==0) 			strcpy(PB->comment, varValue);
	else if (strcasecmp(varName, "TrimBeforeRun")==0) {
			if (strcasecmp(varValue, "True") == 0) 			PB->trimBeforeRun 			= TRUE;
			else if  (strcasecmp(varValue, "False") == 0) 	PB->trimBeforeRun 			= FALSE;
			else PrintHelp(varName);
		}
	else 
		PrintHelp(varName);
	if (valueList != NULL){
		uint32 pos = 0;
		int32 i=0;
		char str[100];
		int32 j = 0;
		
		while (pos < strlen(valueList)) {
			while ((pos < strlen(valueList)) && ((valueList[pos]!='-') && ((valueList[pos] <'0') || (valueList[pos] >'9')))) 
				pos++; // ignore any non numeric value 
			while ((pos < strlen(valueList)) && ((valueList[pos]=='-') || ((valueList[pos]>='0') && (valueList[pos]<='9')))) 
				str[i++] = valueList[pos++]; // copy numbers into str
			str[i] = (char)0;
			if (strlen(str) > 0) {  
				if (j+1 < MAX_VARYING_VAL) 
					tabVal[j++] = atoll(str);
				else HandleError("GetValues", "Too much values for a varying parameter - check Experimentation Plan", 0,ERR_ABORT);
				//printf("[%I64d] ", tabVal[j-1]);
			}
			i = 0;
		}
		//printf("\n");
		return(j);
	}
	return(0);
}
//****************   CheckPB()   ***********************
// Sanity checks the given Params structure
void CheckPB(sParams* PB) {
	if (PB->IOSize <= 0)
		HandleError("Check Params", "Err. param. IOSize value: ", PB->IOSize,ERR_ABORT);  
	if (PB->IOCount <= 0)   
		HandleError("Check Params", "Err. param. IOCount value: ", PB->IOCount,ERR_ABORT);  
	if (PB->IOCount <= PB->ignoreIO)   
			HandleError("Check Params", "IoCount is less than ignoreIO !", PB->IOCount,ERR_ABORT);  
	if (PB->IOCount2 < 0)
		HandleError("Check Params", "Err. param. IOCount2 value: ", PB->IOCount2,ERR_ABORT);  
	if (PB->targetSize < 0)  
			HandleError("Check Params", "Err. param. targetSize value: ", PB->targetSize,ERR_ABORT);  
	//if ((PB->IOCount * PB->IOSize * abs(PB->order) + PB->IOShift > PB->targetSize) && (PB->microBenchID != 4)&& (PB->microBenchID != 3))  
	//			HandleError("Check Params", "targetSize is too small value: ", PB->targetSize,ERR_ABORT);
	//if (PB->targetSize + PB->targetOffset > PB->deviceSize)
	//	HandleError("Check Params", "targetSize + Offset > device Size value: ", PB->deviceSize,ERR_ABORT);  
	if (PB->targetOffset < 0) 
		HandleError("Check Params", "Err. param. targetOffset value: ", PB->targetOffset,ERR_ABORT);  
	if ((PB->nbPartition < 0) || (PB->nbPartition > PB->IOCount)) 
			HandleError("Check Params", "Err. param. nbPartition is negative or smaller than IOCOunt value: ", PB->nbPartition,ERR_ABORT);  
	if (PB->nbPartition > MAX_PARTITIONS) 
			HandleError("Check Params", "Err. param. nbPartition is greater than MAX_PARTITIONS value: ", PB->nbPartition,ERR_ABORT);  
	if ((PB->IOShift < 0) || (PB->IOShift > PB->IOSize)) 
		HandleError("Check Params", "Err. param. IOShift value: ", PB->IOShift,ERR_ABORT);  
	if (PB->ignoreIO < 0) 
		HandleError("Check Params", "Err. param. ignoreIO value: ", PB->ignoreIO,ERR_ABORT);  
	if (PB->pauseIO < 0) 
		HandleError("Check Params", "Err. param. pauseIO value: ", PB->pauseIO,ERR_ABORT);  
	if (PB->pauseBurst < 0) 
		HandleError("Check Params", "Err. param. pauseBurst value: ", PB->pauseBurst,ERR_ABORT);  
	if ((PB->burstIO < 0) || (PB->burstIO > PB->IOCount)) 
		HandleError("Check Params", "Err. param. burstIO value: ", PB->burstIO,ERR_ABORT);  
	if ((PB->parDeg < 0) || (PB->parDeg > MAX_PARALLEL)) 
		HandleError("Check Params", "Err. param. parDeg value: ", PB->parDeg,ERR_ABORT);  
	if ((PB->processID < 0) ||(PB->processID > PB->parDeg)) 
		HandleError("Check Params", "Err. param. processID value: ", PB->processID,ERR_ABORT);  
	if (PB->deviceNum < 0) 
		HandleError("Check Params", "Err. param. deviceNum value: ", PB->deviceNum,ERR_ABORT);  
	if ((PB->collectErase != INT32_MAX) && ((PB->collectErase < 0) || (PB->collectErase > 512)))
		HandleError("Check Params", "Err. param. collectErase value: ", PB->collectErase,ERR_ABORT);  
	if (PB->pauseExp < 0) 
		HandleError("Check Params", "Err. param. pauseExp value: ", PB->pauseExp,ERR_ABORT);  
}	
//****************   GetTestParameters()   ***********************
sParams*	GetTestParameters(unsigned int argc, char** argv) {
	
	sParams *PB;
    //char 			str[MAX_STR];						// Temporary string
	
	PB = (sParams*)malloc(sizeof(sParams));
	if (PB == NULL) HandleError("GetTestParameters", "couldn't allocate Param block", 0,ERR_ABORT);
	if (argc < 2) {
		free(PB);
		PrintHelp("");
	}

    //sprintf(str, "running [%s] with function [%s]\n", argv[0], argv[1]);
	//OutputString(OUT_LOG, "\n");
	//OutputString(OUT_LOG, str);
	
	if (strcasecmp(argv[1], "RandomFormat")==0) {
		PB->mainFunction = RND_FORMAT;
		InitParamsRF(PB);		
		for (uint32 i = 2; i < argc; i+=2) 
			GetValuesRF(argv[i], argv[i+1], PB);
	}
	else if (strcasecmp(argv[1], "GenPrepare")==0) {
		PB->mainFunction = GEN_PREPARE;
		InitParamsGP(PB);		
		for (uint32 i = 2; i < argc; i+=2) 
			GetValuesGP(argv[i], argv[i+1], PB);
	}
	else if (strcasecmp(argv[1], "GenBench")==0) {
		PB->mainFunction = GEN_BENCH;
		InitParamsGB(PB);
		for (uint32 i = 2; i < argc; i+=2) 
			GetValuesGB(argv[i], argv[i+1], PB);
	}
	else if (strcasecmp(argv[1], "Run")==0) {
		PB->mainFunction = RUN_BENCH;
		InitParamsR(PB);		
		for (uint32 i = 2; i < argc; i+=2) 
			GetValues(argv[i], argv[i+1], PB, NULL, NULL);
		CheckPB(PB);	
	}
	else {
		free(PB);
		PrintHelp("");
	}
return (PB);
}

void InitParams(sParams* PB) {
	DWORD 	nameSize = MAX_STR;	
	char	computerName[MAX_STR];
	time_t 			lTime = 0;					// variables used for time stamp generation
	static char   	cpTime[1024] = "";
	
	strcpy(PB->expPlan, "");
	strcpy(PB->expSelect, "");
	strcpy(PB->outName, "");
	PB->nbRun			= 3;
	PB->IOSize 			= 64;			 
	PB->IOCount 		= 2048;			
	PB->IOCountSR 		= 128;			
	PB->IOCountRR 		= 256;			
	PB->IOCountSW 		= 1024;			
	PB->IOCountRW 		= 5116;			
	PB->IOCount2 		= 20480;
	PB->targetSize 		= 0;		 
	PB->targetOffset 	= 0;	
	PB->targetSize2 	= 0;		 
	PB->targetOffset2 	= 0;	
	PB->nbPartition 	= 1;		
	PB->IOShift 		= 0;			
	PB->ignoreIO 		= 0;		
	PB->ignoreIOSR 		= 0;		
	PB->ignoreIORR 		= 0;		
	PB->ignoreIOSW 		= 0;		
	PB->ignoreIORW 		= 50;		
	PB->ratio 			= 0;
	PB->order 			= 1;           
	PB->pauseIO 		= 0;			
	PB->pauseBurst 		= 0;		
	PB->burstIO 		= 1;			
	PB->parDeg 			= 0;			
	PB->processID 		= 0;		
	strcpy(PB->comment, "");
	strcpy(PB->base, "SR");
	strcpy(PB->base2, "");
	time(&lTime);	// get the current time
	strcpy(cpTime, ctime(&lTime));	
	cpTime[strlen(cpTime) - 1] = 0; // ctime adds a trailing newline which we don't want
	strcpy(PB->timeStamp, cpTime);
	if (GetComputerName(computerName, &nameSize)) {	
		strcpy(PB->machine,	computerName);
	}
	PB->deviceNum 		= INT32_MAX;
	PB->deviceSize 		= 0;         
	PB->collectErase 	= INT32_MAX;		
	PB->pauseExp 		= 100;			
	PB->fake 			= FALSE;				
	PB->bufferType 		= NO_BUFFERING;
	PB->microBenchID 	= 0;
	PB->expID 			= 0;		
	PB->runID 			= 0;
	PB->warning			= 0;
	PB->key				= 0;
	PB->trimBeforeRun	= FALSE;
}
void InitParamsRF(sParams* PB) {
	InitParams(PB);
	PB->microBenchID = 0;	// Not really a micro bench
	PB->expID = 6;			// Exp 1 to 5 (3 times) allow determining startup, period, iocount		
	PB->runID = 1;			// Only one run !
	strcpy(PB->comment, "FOR");
}
void InitParamsGP(sParams* PB) {
	InitParams(PB);
	strcpy(PB->outName, "Prepare.bat");
}
void InitParamsGB(sParams* PB) {
	InitParams(PB);
	strcpy(PB->expPlan, "ExpPlan.txt");
	strcpy(PB->expSelect, "ExpSel.txt");
	strcpy(PB->outName, "Bench.bat");
}
void InitParamsR(sParams* PB) {
	InitParams(PB);
}


//****************   PrintHelp()   ***********************
// Prints program usage information
void PrintHelp(const char * varName) {
	
	fprintf(stderr, "\n");
	fprintf(stderr, "a keyword was not recognized: %s\n", varName);
	fprintf(stderr, "Usage: FlashIO <function> [options]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "<function> may include one of the following:\n");
	fprintf(stderr, "  RandomFormat  Writes entirely the device (random IOs of random sizes)\n");
	fprintf(stderr, "  GenPrepare    Generates a batch to prepare the bench\n");
	fprintf(stderr, "  GenBench      Generates a batch running all benchmark experiments\n");
	fprintf(stderr, "  Run           Runs a single experiment\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "For the RandomFormat function, [options] may include any of the following:\n");
	fprintf(stderr, "  Dev <device number>  Device number where format is performed\n");
	fprintf(stderr, "  Bench <id>           Bench ID (should be 0 = default)\n");
	fprintf(stderr, "  Exp <id>             Experiment ID (should be 6 = default)\n");
	fprintf(stderr, "  Size <Number Blocks> Number of Block of the device (detected if not specified)\n");
	fprintf(stderr, "  Fake <True/False>    Run fake format (no real IOs) - To check \n");
	fprintf(stderr, "\n");

	fprintf(stderr, "For the GenPrepare function, [options] may include any of the following:\n");
	fprintf(stderr, "  OutName <filename>   Filename for generating the bench\n");
	fprintf(stderr, "  Dev <device number>  Device number where tests are performed\n");
	fprintf(stderr, "  NbRun <n>            Number of run for each experiment\n");
	fprintf(stderr, "  IOS <io size>        IO request size in sectors (512 Bytes)\n");
	fprintf(stderr, "  IOC <IO Count>       Number of IOs, in blocks, for determining IgnoreIO\n");
	fprintf(stderr, "  IOC2 <IO Count>      Number of IOs, in blocks, for det. Startup and Period\n");
	fprintf(stderr, "  CErase <adress>      if Erase count should be collected, where\n");
	fprintf(stderr, "  Pause <nb of ms>     pause between exps. (a priori) \n");
	fprintf(stderr, "  Fake <True/False>    Run fake tests (no real IOs) - To check \n");
	fprintf(stderr, "  Size <Number Blocks> Number of Block of the device (detected if not specified)\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "For the GenBench function, [options] may include any of the following:\n");
	fprintf(stderr, "  ExpPlan <filename>   File containing the experimentation plan\n");
	fprintf(stderr, "  ExpSel <filename>    File containing the selection of bench to generate\n");
	fprintf(stderr, "  OutName <filename>   Filename for generating the bench\n");
	fprintf(stderr, "  NbRun <n>            Number of run for each experiment\n");
	fprintf(stderr, "  Dev <device number>  Device number where tests are performed\n");
	fprintf(stderr, "  IOS <io size>        IO request size in sectors (512 Bytes)\n");
	fprintf(stderr, "  IOCSR <IO Count>     Number of IOs, in blocks, for SR\n");
	fprintf(stderr, "  IOCRR <IO Count>     Number of IOs, in blocks, for RR\n");
	fprintf(stderr, "  IOCSW <IO Count>     Number of IOs, in blocks, for SW\n");
	fprintf(stderr, "  IOCRW <IO Count>     Number of IOs, in blocks, for RW\n");
	fprintf(stderr, "  IgnIOSR <Nb IO>      Number of IOs to ignore for SR\n");
	fprintf(stderr, "  IgnIORR <Nb IO>      Number of IOs to ignore for RR\n");
	fprintf(stderr, "  IgnIOSW <Nb IO>      Number of IOs to ignore for SW\n");
	fprintf(stderr, "  IgnIORW <Nb IO>      Number of IOs to ignore for RW\n");
	fprintf(stderr, "  CErase <adress>      if Erase count should be collected, where\n");
	fprintf(stderr, "  Pause <nb of ms>     pause between exps. (a priori) \n");
	fprintf(stderr, "  Fake <True/False>    Run fake tests (no real IOs) - To check \n");
	fprintf(stderr, "  BufferType <[N|Y|H|S]> enumerated buffering option\n");
	fprintf(stderr, "  Size <Number Blocks> Number of Block of the device (detected if not specified)\n");
	fprintf(stderr, "  TrimBeforeRun <True/False> TRIM the device before running the benchmark.\n");
	fprintf(stderr, "\n");
	
	fprintf(stderr, "For the Run function, [options] may include any of the following:\n");
	fprintf(stderr, "  Dev <device number>   Device number where tests are performed\n");
	fprintf(stderr, "  IOS <io size>         IO request size in sectors (512 Bytes)\n");
	fprintf(stderr, "  IOC <IO Count>        Number of IOs, in blocks, for one test\n");
	fprintf(stderr, "  TSize <targetsize>    in sectors for Locality(3) and Circularity(4)\n");
	fprintf(stderr, "  TOffs <offset>        in sectors offset at which tests will begun\n");
	fprintf(stderr, "  Part <Nb partitions>  Number of partitions for Partitioning(5)\n");
	fprintf(stderr, "  Shift <shift (B)>     Shift in sectors before IO\n");
	fprintf(stderr, "  IgnIO <Nb IO>         Number of IOs to ignore before computing stats\n");
	fprintf(stderr, "  IgnIO1 <Nb IO>        Number of IOs to ignore before computing stats for pattern 2\n");
	fprintf(stderr, "  Base <SR|RR|SW|RW>    Baseline pattern (or first one when mix)\n");
	fprintf(stderr, "  Base1 <SR|RR|SW|RW>   Second baseline pattern when mix\n");
	fprintf(stderr, "  Ratio <ratio>		 nb of pattern1 for 1 pattern 2 (if pos) and conversely if neg)\n");
	fprintf(stderr, "  Order <k>             Linear increase or decrease (in blocks)\n");
	fprintf(stderr, "  PIO <nb of us>        pause between IOs (in microsec)\n");
	fprintf(stderr, "  PBurst <nb of us>     Pause between burst of burstIO IOs\n");
	fprintf(stderr, "  BurstIO <B>           Number of IOs in a burst (B)\n");
	fprintf(stderr, "  ParDeg <D>            Parallel degree\n");
	fprintf(stderr, "  PID <n>               Process ID between 1 and 16 when parallel IOs\n");
	fprintf(stderr, "  CErase <adress>       if Erase count should be collected, where\n");
	fprintf(stderr, "  Pause <nb of ms>      pause between exps.\n");
	fprintf(stderr, "  Fake <True/False>     Run fake tests (no real IOs) - To check \n");
	fprintf(stderr, "  BufferType <[N|Y|H|S]> enumerated buffering option\n");
	fprintf(stderr, "  Bench <id>            Bench ID between 0 and 9\n");
	fprintf(stderr, "  Exp <id>              Experiment ID between 1 and 6\n");
	fprintf(stderr, "  Run <id>              Run ID between 1 and n\n");
	fprintf(stderr, "  Com <comments>        Experiment name (user defined string)\n");
	fprintf(stderr, "  Warn <ID>             Warning ID in case of problem when generating Bench\n");
	fprintf(stderr, "  Key <KeyID>           Unique key to identify sequentially the experiment\n");
	fprintf(stderr, "  Size <Number Blocks>  Number of Block of the device (detected if not specified)\n");
	fprintf(stderr, "  TrimBeforeRun <True/False> TRIM the device before running the benchmark.\n");
	fprintf(stderr, "\n");
	exit(1);
}

