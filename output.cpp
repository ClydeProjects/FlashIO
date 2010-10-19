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

#include <stdlib.h>
#include "output.h"
#include "utility.h"
#include "genBench.h"
#include <time.h>
#include <winioctl.h>

char			OutputFileName [MAX_OUTPUT_FILES][MAX_FILENAME];
#define 		OUTPUTPREFIX ""


static int cmpdoubles(const void *p1, const void *p2)
{
   return ((*((double *)p1) > *((double *)p2))?1:-1); 
}

void InitLogName(const char *str){
	strcpy(OutputFileName[OUT_LOG], str);
}

// initialize the output filenames to correct values
void InitFileNames(sParams* PB) {

    char base[MAX_STR];
    
	sprintf (base, "%01d_%01d", PB->microBenchID, PB->expID);
	sprintf(OutputFileName[OUT_RES], "%sRESULTS\\RES_%s.csv", OUTPUTPREFIX, base);
	sprintf (base + strlen(base), "_%02d", PB->processID);
	sprintf(OutputFileName[OUT_ALL], "%sTIMINGS\\TIM_%s.csv", OUTPUTPREFIX, base);
	sprintf(OutputFileName[OUT_AVG], "%sTIMINGS\\AVG_%s.csv", OUTPUTPREFIX, base);
	sprintf(OutputFileName[OUT_SOR], "%sTIMINGS\\SOR_%s.csv", OUTPUTPREFIX, base);
	sprintf (base + strlen(base), "_%04d", PB->key);
	switch (PB->microBenchID) {
		case 0:
			sprintf(OutputFileName[OUT_COL], "%s", PB->comment);
			break;
		case GRA:
			sprintf(OutputFileName[OUT_COL], "P%.1f",(double)PB->IOSize/(1024.0/(double)SECTOR));
			break;
		case ALI:
			sprintf(OutputFileName[OUT_COL], "P%.1f",(double)PB->IOShift/(1024.0/(double)SECTOR));
			break;
		case LOC:
			sprintf(OutputFileName[OUT_COL], "P%.0f",(double)PB->targetSize/(1024.0/(double)SECTOR));
			break;
		case PAT:
			sprintf(OutputFileName[OUT_COL], "P%03d",PB->nbPartition);
			break;
		case ORD:
			sprintf(OutputFileName[OUT_COL], "P%03d",PB->order);
			break;
		case PAR:
			sprintf(OutputFileName[OUT_COL], "P%02d",PB->parDeg);
			break;
		case MIX:
			sprintf(OutputFileName[OUT_COL], "P%02d",PB->ratio);
			break;
		case PIO:
			sprintf(OutputFileName[OUT_COL], "P%05d",PB->pauseIO);
			break;
		case PBU:
			sprintf(OutputFileName[OUT_COL], "P%06d",PB->burstIO);
			break;
			default:
				break;
		}
	sprintf (base + strlen(base), "_%s_%01d", OutputFileName[OUT_COL], PB->runID);
	sprintf(OutputFileName[OUT_ERR], "%sTRACES\\ERR_%s.txt", OUTPUTPREFIX, base);
	sprintf(OutputFileName[OUT_TMP], "%sTRACES\\TMP_%s.txt", OUTPUTPREFIX, base);
	sprintf(OutputFileName[OUT_TIM],"%sDETAILS\\DET_%s.csv", OUTPUTPREFIX, base);
	sprintf(OutputFileName[OUT_TRA], "%sTRACES\\TRA_%s.txt", OUTPUTPREFIX, base);
}



// OutputString() writes out the given NULL-terminated string to the specified file
void OutputString(int32 numFile, const char* str) {
	FILE*	fp = NULL;	// file pointer

	fp = fopen(OutputFileName[numFile], "a+");
	if (!fp) {
		if (numFile != OUT_LOG) OutputString(OUT_LOG, str);
		if (numFile != OUT_ERR)  HandleError("OutputString - cannot open file: ", OutputFileName[numFile], GetLastError(), ERR_ABORT);
		else {
			printf("Cannot output error messages - exiting\n");
			printf("%s\n", str);
			exit(1);
		}
	}
	if (numFile == OUT_LOG) {
		time_t 			lTime = 0;					// variables used for time stamp generation
		static char   	cpTime[1024] = "";

		time(&lTime);	// get the current time
		strcpy(cpTime, ctime(&lTime));	
		cpTime[strlen(cpTime) - 1] = 0; // ctime adds a trailing newline which we don't want
		fprintf(fp, "[%s] - %s", cpTime, str);
	}
	else
		fprintf(fp, "%s", str);				// output the string
	fclose(fp);							// close the output file if we opened it
}

/****************   OutputResults()   ***********************/
// OutputResults() takes param and result structures . It formats the data contained
// within the two structures and converts it into a single CSV line which can be easily imported
// into a spreadsheet. 
// It also completes (and sorts) the ALL and SOR files.


void OutputResults(sParams* PB, sResults* RB) {
	FILE*	fp = NULL;	// file pointer
	FILE* 	fp2 = NULL;
	char 	str[MAX_STR];
	boolean Empty = TRUE;
	double 	runSum;
	int32 	reinitVal; 
	char 	res[MAX_STR];
    HANDLE    		lp = INVALID_HANDLE_VALUE;       	// file handle
    DWORD			dwBytes = 0;  
    bool      		bStatus = TRUE;   

  	// Check if open for read works ==> Not new
	lp = INVALID_HANDLE_VALUE; 
    if ((lp = CreateFile(OutputFileName[OUT_RES], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))!= INVALID_HANDLE_VALUE)
    {
    	CloseHandle(lp);
	    Empty = FALSE;	     
    }
    else if (GetLastError() != ERROR_FILE_NOT_FOUND)
    	Empty = FALSE;	     
        	
	lp = INVALID_HANDLE_VALUE; 
    while ((lp = CreateFile(OutputFileName[OUT_RES], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) 
		printf("Could not open result file, %ld\n Retrying .... ", GetLastError());
	if (SetFilePointer(lp, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER) HandleError("OutputRes", "Could not open RES file", GetLastError(), ERR_ABORT);
    
	if (Empty) {
		sprintf(res, "Comment; Key; WG; RunID; PDeg; ProcID; Base; Base1; Ratio; TS2; Order; NbPart; Shift; TSize; PauseIO; Burst; IoSize; "); //IN (17 cols)
		strcat(res, "AvgIO; MinIO; MaxIO; stdD; Erase; InitEr; EndEr; Lost; Time; User; Kernel; "); //OUT (11 cols)
		strcat(res, "IgnIO; Offset; pauseBurst; CompName; TimeStamp;  PauseExp; Erase; BenchID; ExpID; Buff; Device; TotFile; IOC; TO2 \n"); //Details (12 cols)
	}
	else
		strcpy(res, "");
	
	/*IN (17 col)*/
	sprintf(res + strlen(res), "%s; ", PB->comment); 		// identify the experiment
	sprintf(res + strlen(res), "%4d; ", PB->key); 			// Key
	sprintf(res + strlen(res), "%1d; ", PB->warning); 		// warning
	sprintf(res + strlen(res), "%3d; ", PB->runID); 		// RunID between 0 and nbRun
	sprintf(res + strlen(res), "%d; ", PB->parDeg);		// varying 6
	sprintf(res + strlen(res), "%d; ", PB->processID);		// reference
	sprintf(res + strlen(res), "%s; ", PB->base);
	sprintf(res + strlen(res), "%s; ", PB->base2);
	sprintf(res + strlen(res), "%d; ", PB->ratio);			// Varying 7
	sprintf(res + strlen(res), "%.1f; ", (double)PB->targetSize2/(1024.0/(double)SECTOR));	
	sprintf(res + strlen(res), "%d; ", PB->order);			// Varying 5
	sprintf(res + strlen(res), "%d; ", PB->nbPartition);	// Varying 4
	sprintf(res + strlen(res), "%.1f; ", (double)PB->IOShift/(1024.0/(double)SECTOR));		// Varying 2
	sprintf(res + strlen(res), "%.1f; ", (double)PB->targetSize/(1024.0/(double)SECTOR));	// Varyin 3
	sprintf(res + strlen(res), "%d; ", PB->pauseIO);		// Varying 8
	sprintf(res + strlen(res), "%d; ", PB->burstIO);	// Varying 9
	sprintf(res + strlen(res), "%.1f; ", (double)PB->IOSize/(1024.0/(double)SECTOR));		// Varying 1
	
	// OUT (11 col)
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->avgIO * 1000000.0)); //Result
	sprintf(res + strlen(res), "%d; ", (uint32)  (RB->minIO * 1000000.0));
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->maxIO * 1000000.0));
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->stdDevIO* 1000000.0));
	sprintf(res + strlen(res), "%d; ", RB->eraseCount);
	sprintf(res + strlen(res), "%d; ", RB->initErase);
	sprintf(res + strlen(res), "%d; ", RB->endErase);
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->lostTime  * 1000000.0));
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->timeClock * 1000000.0));
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->timeUser  * 1000000.0));
	sprintf(res + strlen(res), "%d; ", (uint32) (RB->timeKernel* 1000000.0));
	
	// Details (12 col) 
	sprintf(res + strlen(res), "%d; ", PB->ignoreIO);		
	sprintf(res + strlen(res), "%.1f; ", (double)PB->targetOffset/(1024.0/(double)SECTOR));	
	sprintf(res + strlen(res), "%d; ", PB->pauseBurst);		
	sprintf(res + strlen(res), "%s; ", PB->machine);
	sprintf(res + strlen(res), "%s; ", PB->timeStamp);
	sprintf(res + strlen(res), "%d; ", PB->pauseExp/1000);				
	if (PB->collectErase == INT32_MAX)
		strcat(res, "NO; ");
	else
		sprintf(res + strlen(res), "%d; ", PB->collectErase);	
	sprintf(res + strlen(res), "%d; ", PB->microBenchID);	
	sprintf(res + strlen(res), "%d; ", PB->expID);			
	switch(PB->bufferType) {				
	case NO_BUFFERING:
		strcat(res, "NoBuff; ");
		break;
	case FS_BUFFERING:
		strcat(res, "FSBuff; ");
		break;
	case HW_BUFFERING:
		strcat(res, "HWBuff; ");
		break;
	case (FS_BUFFERING | HW_BUFFERING):
		strcat(res, "HW_SW; ");
		break;
	default:
		strcat(res, "????; ");
		break;
	}
	//DEV
	sprintf(res + strlen(res), "%d; ", PB->deviceNum);	
	sprintf(res + strlen(res), "%d; ", PB->deviceSize);	
	sprintf(res + strlen(res), "%d; ", PB->IOCount);
	sprintf(res + strlen(res), "%.1f; ", (double)PB->targetOffset2/(1024.0/(double)SECTOR));	
				
	strcat(res, "\n");
	bStatus = WriteFile(lp, res, strlen(res), &dwBytes, NULL);
    OutputString(OUT_TRA, res);
	
    CloseHandle(lp);
    
	// OUTPUT GLOBAL TIMINGS
	if ((fp  = fopen(OutputFileName[OUT_ALL], "r")) != NULL ) {
		fp2= fopen(OutputFileName[OUT_TMP], "a+");
		if (fp2 == NULL) HandleError("OutputRes", "Could not open TMP file", GetLastError(), ERR_ABORT); 
		fgets(str, MAX_STR,fp);
		str[strlen(str)-1] = '\0';
		fprintf(fp2, "%s; %s\n", str, OutputFileName[OUT_COL]);
		for (int32 it = 0; it < PB->IOCount; ++it) {
			if (fgets(str, MAX_STR,fp) == NULL) sprintf(str, " \n");
			str[strlen(str)-1] = '\0';
			fprintf(fp2, "%s; %d\n", str, (uint32) (RB->timing[it] * 1000000.0));
		}
		fclose(fp2);
		fclose(fp);
		remove(OutputFileName[OUT_ALL]);
		rename(OutputFileName[OUT_TMP], OutputFileName[OUT_ALL]);
		remove(OutputFileName[OUT_TMP]);
	}
	else {
		fp = fopen(OutputFileName[OUT_ALL], "a+");
		fprintf(fp, "NUMIO; %s\n", OutputFileName[OUT_COL]);
		for (int32 it = 0; it < PB->IOCount; ++it)	
			fprintf(fp, "%d; %d\n", it, (uint32) (RB->timing[it] * 1000000.0));
		fclose(fp);
	}	
	// OUTPUT RUNNING AVG
	if ((fp  = fopen(OutputFileName[OUT_AVG], "r")) != NULL ) {
		fp2= fopen(OutputFileName[OUT_TMP], "a+");
		if (fp2 == NULL) HandleError("OutputRes", "Could not open TMP file", GetLastError(), ERR_ABORT);
		fgets(str, MAX_STR,fp);
		str[strlen(str)-1] = '\0';
		fprintf(fp2, "%s; %s\n", str, OutputFileName[OUT_COL]);
		runSum = 0;
		reinitVal = 0;
		for (int32 it = PB->ignoreIO; it < PB->IOCount; ++it) {
			if ((strcasecmp(PB->comment, "SPE")== 0)  && ((it == PB->IOCount/3) || (it == 2 * PB->IOCount/3))) {
				runSum = 0;
				reinitVal += PB->IOCount/3;
			}
			runSum += RB->timing[it];
			if (fgets(str, MAX_STR,fp) == NULL) sprintf(str, " \n");
			str[strlen(str)-1] = '\0';
			fprintf(fp2, "%s; %d\n", str, (uint32) (runSum/((double)(it - PB->ignoreIO - reinitVal + 1)) * 1000000.0));
		}
		fclose(fp2);
		fclose(fp);
		remove(OutputFileName[OUT_AVG]);
		rename(OutputFileName[OUT_TMP], OutputFileName[OUT_AVG]);
		remove(OutputFileName[OUT_TMP]);
	}
	else {
		fp = fopen(OutputFileName[OUT_AVG], "a+");
		fprintf(fp, "NUMIO; %s\n", OutputFileName[OUT_COL]);
		runSum = 0;
		reinitVal = 0;
		for (int32 it = PB->ignoreIO; it < PB->IOCount; ++it) {
			if ((strcasecmp(PB->comment, "SPE")== 0)  && ((it == PB->IOCount/3) || (it == 2 * PB->IOCount/3))) {
				runSum = 0;
				reinitVal += PB->IOCount/3;
			}
			runSum += RB->timing[it];
			fprintf(fp, "%d; %d\n", it, (uint32) (runSum/((double)(it - PB->ignoreIO - reinitVal + 1))  * 1000000.0));
		}
		fclose(fp);
	}	
	// output detailed timing with erase and type
	fp = fopen(OutputFileName[OUT_TIM], "a+");
	if (!fp) HandleError("OutputString", "Could not open TIMING file", GetLastError(), ERR_ABORT);
	for (int32 it = 0; it < PB->IOCount; ++it) {
		fprintf(fp, "%c; %c; %d; %ld\n", RB->isRND[it]?'R':'S', RB->isWrite[it]?'W':'R', RB->dPos[it], (long) (RB->timing[it]* 1000000.0));
	} 
	fclose(fp);								
	
//	// SORT TIMINGS
//    qsort(RB->timing, PB->IOCount, sizeof(double), cmpdoubles);
//
//	// OUTPUT SORTED GLOBAL TIMINGS
//	if ((fp  = fopen(OutputFileName[OUT_SOR], "r")) != NULL ) {
//		fp2= fopen(OutputFileName[OUT_TMP], "a+");
//		if (fp2 == NULL) HandleError("OutputRes", "Could not open TMP file", GetLastError(), ERR_ABORT);
//		fgets(str, MAX_STR,fp);
//		str[strlen(str)-1] = '\0';
//		fprintf(fp2, "%s; %s\n", str, OutputFileName[OUT_COL]);
//		for (int32 it = 0; it < PB->IOCount; ++it) {
//			if (fgets(str, MAX_STR,fp) == NULL) sprintf(str, " \n");
//			str[strlen(str)-1] = '\0';
//			fprintf(fp2, "%s; %d\n", str, (uint32) (RB->timing[it] * 1000000.0));
//		}
//		fclose(fp2);
//		fclose(fp);
//		remove(OutputFileName[OUT_SOR]);
//		rename(OutputFileName[OUT_TMP], OutputFileName[OUT_SOR]);
//		remove(OutputFileName[OUT_TMP]);
//	}
//	else {
//		fp = fopen(OutputFileName[OUT_SOR], "a+");
//		fprintf(fp, "NUMIO; %s\n", OutputFileName[OUT_COL]);
//		for (int32 it = 0; it < PB->IOCount; ++it)
//			fprintf(fp, "%d;%d\n", it, (uint32) (RB->timing[it] * 1000000.0));
//		fclose(fp);
//	}
	
}
