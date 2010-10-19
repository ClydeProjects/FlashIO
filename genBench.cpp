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
#include "input.h"
#include "flashIO.h"
#include "utility.h"
#include "blocAlloc.h"
#include "genBench.h"
#include "random.h"

#define DEVICE_TOO_SMALL 	1
#define TEST_EXCEED_DEVICE 	2
#define MAXBENCH 			10
#define MAXMODE 			 6

void GenExp(FILE*	fp, sParams* PB) {
	
	char cm[MAX_STR];
	
	if (PB->processID > 0)
		sprintf(cm, "start flashIO Run ");
	else
		sprintf(cm, "flashIO Run ");
	
	
	if (PB->warning != 0)				sprintf(cm + strlen(cm), "Warn %d ", 		PB->warning);
	sprintf(cm + strlen(cm), "Com %s ",												PB->comment);
	sprintf(cm + strlen(cm), "Bench %d ",											PB->microBenchID);
	sprintf(cm + strlen(cm), "Exp %d ", 											PB->expID);
	sprintf(cm + strlen(cm), "Key %d ", 											PB->key);
	sprintf(cm + strlen(cm), "Run %d ", 											PB->runID);
	sprintf(cm + strlen(cm), "Dev %d ", 											PB->deviceNum);
	sprintf(cm + strlen(cm), "IOS %d ", 											PB->IOSize);
	if (PB->microBenchID == ALI)		sprintf(cm + strlen(cm), "Shift %d ", 		PB->IOShift);
	if (PB->microBenchID == LOC) 		sprintf(cm + strlen(cm), "TSize %d ", 		PB->targetSize);
	if (PB->microBenchID == PAT) 		sprintf(cm + strlen(cm), "Part %d ", 		PB->nbPartition);
	if (PB->microBenchID == ORD) 		sprintf(cm + strlen(cm), "Order %d ", 		PB->order);
	if (PB->microBenchID == PAR) 		sprintf(cm + strlen(cm), "ParDeg %d ", 	PB->parDeg); 
	if (PB->microBenchID == PAR) 		sprintf(cm + strlen(cm), "PID %d ",		PB->processID); 
	if (PB->microBenchID == MIX) 		{
		sprintf(cm + strlen(cm), "Base2 %s ", 										PB->base2);	
		sprintf(cm + strlen(cm), "Ratio %d ", 										PB->ratio);
		sprintf(cm + strlen(cm), "TSize2 %d ",										PB->targetSize2);
		sprintf(cm + strlen(cm), "TOffs2 %d ",										PB->targetOffset2);
	}
	if (PB->microBenchID == PIO) 		sprintf(cm + strlen(cm), "PIO %d ", 		PB->pauseIO);
	if (PB->microBenchID == PBU) 		sprintf(cm + strlen(cm), "PBurst %d ", 	PB->pauseBurst);
	if (PB->microBenchID == PBU) 		sprintf(cm + strlen(cm), "BurstIO %d ",	PB->burstIO);
	sprintf(cm + strlen(cm), "Base %s ", 											PB->base);
	sprintf(cm + strlen(cm), "IOC %d ",											PB->IOCount);
	if (PB->microBenchID != LOC)
		sprintf(cm + strlen(cm), "TSize %d ",										PB->targetSize);
	sprintf(cm + strlen(cm), "TOffs %d ",											PB->targetOffset);
	sprintf(cm + strlen(cm), "IgnIO %d ", 											PB->ignoreIO);
	sprintf(cm + strlen(cm), "Pause %d ", 											PB->pauseExp);
	if (PB->collectErase != INT32_MAX) 	sprintf(cm + strlen(cm), "CErase %d ", 	PB->collectErase);
	if (PB->fake == TRUE) 				strcat(cm, "Fake True "); 
	if (PB->bufferType == HW_BUFFERING)	strcat(cm, "BufferType H ");
	if (PB->bufferType == FS_BUFFERING)	strcat(cm, "BufferType S ");
	if (PB->bufferType == (HW_BUFFERING | FS_BUFFERING))	strcat(cm, "BufferType A ");
	if (PB->trimBeforeRun == TRUE)	strcat(cm, "TrimBeforeRun True ");
	strcat(cm, "\n");
	fprintf(fp, "%s", cm);
	OutputString(OUT_LOG, cm);
	
}


/* Compute Params
 * The difficulty here is to acomodate all the benchmark runs into the memory available
 * on the device without having to "reformat" it.
 * Allocation is based on the assumption that benchmark runs are done in a given order:
 * 1) read only benchmark (do not modify the "state" of the flash (Exp. 1 to 15)
 * 2) random writes (on the whole device) ==> modify slightly, but randomly the state (Exp. 16-22)
 * 3) Sequential writes  on focused areas ==> modify the state, but the focus moves 
 * 	  ==> the memory is "consumed" sequentially. If no more memory is available warning 
 * 		  DEVICE_TOO_SMALL is set and the offset is chosen randomly (thus the test is not ok)
 * 4) Mix patterns SR/SW, RR/SW, SW/RW
 * 5) parallelism and partitionned patterns (Exp. 32 & 33)==> Memory is allocated in the 
 * 	  remaining part of the device. Tuning of IOCount and experiments should be done to fit 
 * 	  in the device.
 * 6) Ordered patterns (Exp. 34): when the experiment is "focused" (small gaps), we do as 
 * 	  with sequential up to the point where there is no place. Then, it is random.
 * Memory allocation is done thanks to the blocAlloc module. 
 */


void ComputeParams(sParams* PB, item* memList, int32 value, int32 nbVal ) {
	
	static 	int32 	startAddress = 0;
	static int32 	shift = 0;
	int32 			size;
	int32 			size1;
	int32 			size2;
	bool 			isRead;
	bool 			isSeq;
	
	
	printf("Generates %d.%d with param %d\n", PB->microBenchID, PB->expID,  value);
	
	if (strcasecmp(PB->base, "SR") == 0) {
		PB->ignoreIO = PB->ignoreIOSR;
		PB->IOCount = PB->IOCountSR;
		isSeq = TRUE;
		isRead = TRUE;
	}
	if (strcasecmp(PB->base, "SW") == 0) {
		PB->ignoreIO = PB->ignoreIORR;
		PB->IOCount = PB->IOCountRR;
		isSeq = TRUE;
		isRead = FALSE;
	}
	if (strcasecmp(PB->base, "RR") == 0) {
		PB->ignoreIO = PB->ignoreIOSW;
		PB->IOCount = PB->IOCountSW;
		isSeq = FALSE;
		isRead = TRUE;
	}
	if (strcasecmp(PB->base, "RW") == 0) {
		PB->ignoreIO = PB->ignoreIORW;
		PB->IOCount = PB->IOCountRW;
		isSeq = FALSE;
		isRead = FALSE;
	}
	
	PB->targetSize = PB->deviceSize; // Default value for target size
	if (PB->microBenchID == GRA) PB->IOSize = value;
	if (PB->microBenchID == ALI) PB->IOShift = value;
	if (PB->microBenchID == LOC) PB->targetSize = value * PB->IOSize ;
	if (PB->microBenchID == PAT) PB->nbPartition = value;
	if (PB->microBenchID == ORD) PB->order = value;
	if (PB->microBenchID == PAR) PB->parDeg = value;
	if (PB->microBenchID == MIX) PB->ratio = value;
	if (PB->microBenchID == PIO) PB->pauseIO = value;
	if (PB->microBenchID == PBU) PB->burstIO = value;

	// Number of sectors potentially touched by the experiment 
	size = PB->IOSize * PB->IOCount;
	if (PB->IOShift != 0) size += PB->IOSize; 
		
	if (PB->microBenchID == LOC) size = PB->targetSize;
	if (PB->microBenchID == ORD) size = PB->IOSize * abs(PB->order)* PB->IOCount;
	
	// 1-GRANULARITY , 2-ALIGNMENT, 8-PAUSE, 9-BURST
	if ((PB->microBenchID == GRA) || (PB->microBenchID == ALI)|| 
		(PB->microBenchID == PIO)|| (PB->microBenchID == PBU)) {
		
		if (isSeq == FALSE) 
			PB->targetOffset = 0;
		else if (isRead == TRUE) {
			PB->targetOffset = rg.IRandom(0,(int32)((PB->targetSize - size)/BLOCK))*BLOCK;
			PB->targetSize = size;
		}
		else {
			PB->targetOffset = MemSearch(memList, size);
			PB->targetSize = size;
		}
	}
	// 3-LOCALITY
	else if (PB->microBenchID == LOC) {
		if ((isRead == TRUE) || (value > 1024)) 
			PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize - size)/BLOCK))*BLOCK;
		else {
			PB->targetOffset = MemSearch(memList, size);
			//printf("==>%d  %d\n",size, PB->targetOffset );
		}
	}
	// 07-MIX 
	else if (PB->microBenchID == MIX) {
		int32 I1, I2, C1, C2;
		
		I1 = PB->ignoreIO;
		C1 = PB->IOCount;
		if (strcasecmp(PB->base2, "SR") == 0) {
			I2 = PB->ignoreIOSR;
			C2 = PB->IOCountSR;
		}
		
		if (strcasecmp(PB->base2, "SW") == 0) {
			I2 = PB->ignoreIOSW;
			C2 = PB->IOCountSW;
		}
				
		if (strcasecmp(PB->base2, "RR") == 0) {
			I2 = PB->ignoreIORR;
			C2 = PB->IOCountRR;
		}

		if (strcasecmp(PB->base2, "RW") == 0) {
			I2 = PB->ignoreIORW;
			C2 = PB->IOCountRW;
		}

		if (PB->ratio < 0) {
			PB->ignoreIO = max(I1 + (I1-1)*(-PB->ratio) + 1, I2 * (-PB->ratio + 1)/(-PB->ratio) + 1);
			PB->IOCount = max(C1-I1, C2-I2) + PB->ignoreIO;
			size1 = PB->IOSize * (1+ (int32)(PB->IOCount/(-PB->ratio + 1)));
			size2 = PB->IOSize * (1+ (int32)((PB->IOCount/(-PB->ratio + 1)) * (-PB->ratio)));
		}
		else if (PB->ratio > 0) {
			PB->ignoreIO = max(I2 + (I2-1) * PB->ratio + 1, (I1 * (PB->ratio + 1)/PB->ratio) + 1);
			PB->IOCount = max(C1-I1, C2-I2) + PB->ignoreIO;
			size1 = PB->IOSize * (1+ (int32)(PB->IOCount/(PB->ratio + 1)));
			size2 = PB->IOSize * (1+ (int32)((PB->IOCount/(PB->ratio + 1)) * (PB->ratio)));
		}
		else {
			PB->ignoreIO = I2;
			PB->IOCount = C2;
			size1 = 0;
			size2 = PB->IOSize * PB->IOCount;
		}
		
		if (((strcasecmp(PB->base, "SR") == 0) &&  (strcasecmp(PB->base2, "RR") == 0)) ||
			((strcasecmp(PB->base, "SR") == 0) &&  (strcasecmp(PB->base2, "RW") == 0))) {
			PB->targetSize = size1;
			PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize - size1)/BLOCK))*BLOCK;
			PB->targetOffset2 = 0;
			PB->targetSize2 = PB->deviceSize;
		}
		if ((strcasecmp(PB->base, "RR") == 0) &&  (strcasecmp(PB->base2, "RW") == 0)) {
			PB->targetSize = PB->deviceSize;
			PB->targetOffset = 0;
			PB->targetOffset2 = 0;
			PB->targetSize2 = PB->deviceSize;
		}
		if ((strcasecmp(PB->base, "RR") == 0) &&  (strcasecmp(PB->base2, "SW") == 0)) {
			PB->targetOffset2 = MemSearch(memList, size2);
			PB->targetSize2 = size2;
			PB->targetOffset = MemMinAddress(memList);
			PB->targetSize = PB->deviceSize - PB->targetOffset;
		}
		if ((strcasecmp(PB->base, "SW") == 0) &&  (strcasecmp(PB->base2, "RW") == 0)) {
			PB->targetOffset = MemSearch(memList, size1);
			PB->targetSize = size1;
			PB->targetOffset2 = MemMinAddress(memList);
			PB->targetSize2 = PB->deviceSize - PB->targetOffset2;
		}
		if ((strcasecmp(PB->base, "SR") == 0) &&  (strcasecmp(PB->base2, "SW") == 0)) {
			PB->targetOffset2 = MemSearch(memList, size2);
			PB->targetSize2 = size2;
			PB->targetOffset = rg.IRandom((int32)((PB->deviceSize - MemMinAddress(memList))/2)/BLOCK,(int32)((PB->deviceSize - size1)/BLOCK))*BLOCK;
			PB->targetSize = size1;
		}
	}
	// 6-PARALLELISM
	else if (PB->microBenchID == PAR) {
		PB->targetSize = ((int32)((PB->deviceSize / PB->parDeg)/BLOCK))*BLOCK;
		if (isSeq == FALSE) 
			PB->targetOffset =  PB->processID * PB->targetSize;
		else if (isRead == TRUE) {
			PB->targetOffset =  rg.IRandom((PB->processID * PB->targetSize)/BLOCK, 
									 		 ((PB->processID + 1) * PB->targetSize - size)/BLOCK) * BLOCK;
			PB->targetSize = size;
		}
		else { // SEQUENTIAL WRITE..... // CAUTION : PARALLEL EXPERIMENT MUST BE AFTER PARTITIONING
			PB->targetSize = (PB->deviceSize - startAddress) / PB->parDeg;
			if (PB->targetSize < size) HandleError("ComputeParam", "device too small", 0, ERR_ABORT);
			PB->targetOffset =  MemAllocNearestAfterA(memList, (PB->processID) * PB->targetSize + startAddress, size);
			PB->targetSize = size;
		}
	}
	// 4-PARTITIONING
	else if (PB->microBenchID == PAT) {
		if ((isRead == TRUE) || (PB->nbPartition > 16)) {
			PB->targetOffset =  0;
			PB->targetSize = ((int32) (PB->deviceSize/(MAX_PARTITIONS * BLOCK))) * MAX_PARTITIONS * BLOCK;
		}
		else { // SEQUENTIAL WRITE..... // CAUTION : PARTITIONNING EXPERIMENT MUST BE THE FIRST ...
			if (startAddress == 0)
				startAddress = MemMinAddress(memList);
			if ((size % (16*BLOCK)) != 0)
				size = ((int32) (size / (16*BLOCK)) + 1 ) * (16 * BLOCK);
			PB->targetSize = PB->deviceSize - startAddress - size * nbVal * PB->nbRun;
			PB->targetSize = (int32) (PB->targetSize/ (16 * BLOCK)) * (16 * BLOCK); 
			if (PB->targetSize < size) HandleError("ComputeParam", "device too small", 0, ERR_ABORT);
			PB->targetOffset = startAddress + shift;
			for (int32 k = 0; k < PB->nbPartition; ++k) {
				long temp;
				temp = MemAlloc(memList, PB->targetOffset + k * PB->targetSize/PB->nbPartition,  
						PB->targetOffset + k * PB->targetSize/PB->nbPartition + size/PB->nbPartition);
			}	
			shift = shift + size/PB->nbPartition;
		}
		
	}
	// 5-ORDER
	else if (PB->microBenchID == ORD) {
		size = PB->IOSize * PB->IOCount * PB->order;
		if (size == 0)  size = BLOCK;
		PB->targetSize = abs(size);
		if (isRead == TRUE) {
			PB->targetOffset =  rg.IRandom(0,(int32)((PB->deviceSize - PB->targetSize)/BLOCK))*BLOCK;
			if (size < 0)
				PB->targetOffset =  PB->deviceSize - PB->targetOffset;
		}
		else {
			PB->targetOffset =  MemSearch(memList, abs(size));
			if (PB->targetOffset == INT32_MAX) {
				PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize - abs(size))/BLOCK))*BLOCK;
				PB->warning = DEVICE_TOO_SMALL;
				OutputString(OUT_LOG, "DEVICE TOO SMALL \n");
			}	
			if (size <0)
				PB->targetOffset =  PB->targetOffset - size;
		}
	}

	if (PB->targetOffset == INT32_MAX) HandleError("ComputeParam", "device too small", 0, ERR_ABORT);
		
	if ((PB->order >= 0) && (PB->targetOffset + PB->targetSize > PB->deviceSize)) {
		char st[MAX_STR];
		
		sprintf(st, "Adjusting TargetSize (TO = %d, TS = %d DS = %d\n", PB->targetOffset, PB->targetSize, PB->deviceSize);
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		PB->warning = PB->warning | TEST_EXCEED_DEVICE;
		OutputString(OUT_LOG, st);
	}
	if ((PB->order < 0) && (PB->targetOffset - PB->targetSize < 0)) {
		char st[MAX_STR];

		sprintf(st, "Adjusting TargetSize Reverse Order (TO = %d, TS = %d DS = %d\n", PB->targetOffset, PB->targetSize, PB->deviceSize);		PB->targetSize = PB->targetOffset;
		PB->warning = PB->warning | TEST_EXCEED_DEVICE;
		OutputString(OUT_LOG, st);
	}
}


int32 parseExp(FILE*	fp, int32 currExp, sParams* PB, int32* tabVal) {
	char 		str[MAX_STR];
	int32 		numExp;
	char 		varName [MAX_STR];
	char 		varValue [MAX_SIZE_PARAM];
	char 		valueList [MAX_STR];
	int32 		nbVal = 0;
	int32		n;
	
	rewind(fp);
	numExp = 0; // Exp 0 does not exists
	while ((numExp != currExp) && (fgets(str, MAX_STR,fp) != NULL)) numExp = atoi(str); // Find the experiment to parse
	if (numExp != currExp) HandleError("parseExp", "bug !", 0, ERR_ABORT);
	numExp = 0;
	while ((numExp == 0) && (fgets(str, MAX_STR,fp) != NULL)) {
		str[strlen(str) -1] = (char)0;	// remove newline
		numExp = atoi(str); 
		if (numExp == 0) { // a line to parse !
			uint32 pos = 0;
			int32 i=0;
						
			while ((pos < strlen(str)) && ((str[pos] == (char)32) || (str[pos] == (char)9))) pos++; //ignore tabs and spaces
			while ((pos < strlen(str)) && ((str[pos] != (char)32) && (str[pos] != (char)9))) varName[i++] = str[pos++]; //Param Name
			if ((i != 0) && (varName[0] != '=')) { // There is a new parameter !
				varName[i] = (char)0;
				i = 0;
				while ((pos < strlen(str)) && ((str[pos] == (char)32) || (str[pos] == (char)9))) pos++; //ignore tabs and spaces
				while ((pos < strlen(str)) && ((str[pos] != (char)32) && (str[pos] != (char)9))) varValue[i++] = str[pos++]; //Param Value
				varValue[i] = (char)0;
				i = 0;
				while (pos < strlen(str)) valueList[i++] = str[pos++]; //Rest of the line
				valueList[i] = (char)0;
				n = GetValues(varName, varValue, PB, valueList, tabVal);
				if (n != 0) 
					nbVal = n;
			}
		}
	}	
	return (nbVal);
}
void parseSel(sParams* PB, bool* tabSel) {
	FILE*		fp = NULL;
	int32		microB;
	int32		mode;
	char 		str[MAX_STR];
	
	if ((fp  = fopen(PB->expSelect, "r")) == NULL ) 
		HandleError("parseSel", "Cannot open experimentation selection file", GetLastError(), ERR_ABORT);
	for (int i = 0; i < MAXBENCH * MAXMODE; ++i) {
		tabSel[i]= FALSE;
	}
	while (fgets(str, MAX_STR, fp) != NULL) {
		str[1] = '\0';
		microB = atoi(str); // retrieve a number on 1 digits at the begining
		str[3] = '\0';
		mode = atoi(&(str[2])); // retrieve a number on 1 digits at position 2
		if ((microB > 0) && (microB <= MAXBENCH) && (mode >0) && (mode <= MAXMODE))
			tabSel[(microB - 1) * MAXMODE + mode - 1] = TRUE;
	}
	fclose(fp);
}

/* This function generates a batch named prepare.bat (default).
 * The batch contains 6 experiments
 * Exp1: SPE - (Special) - runs IOCount SR then IOCount RW then IOCount SR
 * 		 The goal is here to determine the Pause value
 * Exp2: SIO.SR - runs IOCount2 SR - goal: determine IOIgnoreSR and IOCountSR
 * Exp3-5 : same for RR, SW, RW
 * Exp6 : Random format of the device 
 */ 
 	void GenPrepare(sParams* PB) {
	FILE*		fp2 = NULL;	// file pointer
	int32 		size;

	fp2 = fopen(PB->outName, "w");
	if (fp2 == NULL) HandleError("GenBench", "Could not open output file", GetLastError(), ERR_ABORT);
	strcpy(PB->comment, "SPE");
	strcpy(PB->base, "SR");
	PB->expID = 1;
	PB->microBenchID = 0;
	PB->pauseExp = 10000;
	size = PB->IOSize * PB->IOCount;
	for (PB->runID = 0; PB->runID < PB->nbRun; (PB->runID)++) {
		PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize/2 - size)/BLOCK))*BLOCK; // We choose a random location in the first half of the device
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		GenExp(fp2, PB);
	}
	PB->IOCount = PB->IOCount2;
	size = PB->IOSize * PB->IOCount;
	strcpy(PB->comment, "SIO.SR");
	PB->expID = 2;
	for (PB->runID = 0; PB->runID < PB->nbRun; (PB->runID)++) {
		PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize/2 - size)/BLOCK))*BLOCK; // We choose a random location in the first half of the device
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		GenExp(fp2, PB);
	}
	strcpy(PB->base, "RR");
	strcpy(PB->comment, "SIO.RR");
	PB->expID++;
	for (PB->runID = 0; PB->runID < PB->nbRun; (PB->runID)++) {
		PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize/2 - size)/BLOCK))*BLOCK; // We choose a random location in the first half of the device
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		GenExp(fp2, PB);
	}
	strcpy(PB->base, "SW");
	strcpy(PB->comment, "SIO.SW");
	PB->expID++;
	for (PB->runID = 0; PB->runID < PB->nbRun; (PB->runID)++) {
		PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize/2 - size)/BLOCK))*BLOCK; // We choose a random location in the first half of the device
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		GenExp(fp2, PB);
	}
	strcpy(PB->base, "RW");
	strcpy(PB->comment, "SIO.RW");
	PB->expID++;
	for (PB->runID = 0; PB->runID < PB->nbRun; (PB->runID)++) {
		PB->targetOffset = rg.IRandom(0,(int32)((PB->deviceSize/2 - size)/BLOCK))*BLOCK; // We choose a random location in the first half of the device
		PB->targetSize = PB->deviceSize - PB->targetOffset;
		GenExp(fp2, PB);
	}
	PB->expID++;
	fprintf(fp2, "FlashIO RandomFormat Dev %d Bench %d Exp %d ", PB->deviceNum, PB->microBenchID, PB->expID);

	if (PB->trimBeforeRun == TRUE)
			fprintf(fp2, "TrimBeforeRun True\n");
		else
			fprintf(fp2, "\n");

	if (PB->fake == TRUE) 
		fprintf(fp2, "Fake True\n Pause\n");
	else
		fprintf(fp2, "\n Pause\n");

	fclose(fp2);
}



void GenBench(sParams* PBBench) {

	boolean 	finished = FALSE;
	int32		prevExp = 0;
	int32		currExp;
	int32 		numExp;
	FILE*		fp = NULL;	// file pointer
	FILE*		fp2 = NULL;	// file pointer
	char 		str[MAX_STR];
	int32		tabVal[MAX_VARYING_VAL];
	sParams		PBExp;
	int32		nbVal;
	item* 		memList;
	bool		tabSel[MAXBENCH*MAXMODE];
	int32		nbExp = 0;
	int32		key = 0;
	
	parseSel(PBBench, tabSel);
	// Allocate data structure for computing target offset
	memList = InitMemList((int32)(PBBench->deviceSize));
	fp2 = fopen(PBBench->outName, "w");
	if (fp2 == NULL) HandleError("GenBench", "Could not open output file", GetLastError(), ERR_ABORT);
	
	while (finished == FALSE) {
	// find the next experiment - it is the smallest numExp, greater than prevExp 
		if ((fp  = fopen(PBBench->expPlan, "r")) == NULL ) 
			HandleError("GenBench", "Cannot open experimentation plan file", GetLastError(), ERR_ABORT);
		currExp = INT32_MAX;
		while (fgets(str, MAX_STR,fp) != NULL) {
			numExp = atoi(str);
			if ((numExp > prevExp) && (numExp < currExp)) {
				currExp = numExp;
			}
		}
		if ((currExp > prevExp) && (currExp != INT32_MAX)) {
			printf("===================  Order Number %d\n", currExp);
			
			//New param structure
			InitParams(&PBExp);
			PBExp.deviceNum = PBBench->deviceNum;
			PBExp.IOSize = PBBench->IOSize;
			PBExp.IOCount = PBBench->IOCount;
			PBExp.IOCountSR = PBBench->IOCountSR;
			PBExp.IOCountRR = PBBench->IOCountRR;
			PBExp.IOCountSW = PBBench->IOCountSW;
			PBExp.IOCountRW = PBBench->IOCountRW;
			PBExp.ignoreIO = PBBench->ignoreIO;
			PBExp.ignoreIOSR = PBBench->ignoreIOSR;
			PBExp.ignoreIORR = PBBench->ignoreIORR;
			PBExp.ignoreIOSW = PBBench->ignoreIOSW;
			PBExp.ignoreIORW = PBBench->ignoreIORW;
			PBExp.collectErase = PBBench->collectErase;
			PBExp.pauseExp = PBBench->pauseExp;
			PBExp.fake = PBBench->fake;
			PBExp.bufferType = PBBench->bufferType;
			PBExp.deviceSize = PBBench->deviceSize;	
			PBExp.burstIO = PBBench->burstIO;
			PBExp.nbRun = PBBench->nbRun;  
			
			// parse the experiment
			nbVal = parseExp(fp, currExp, &PBExp, tabVal);
			if (tabSel[(PBExp.microBenchID - 1) * MAXMODE + PBExp.expID - 1] == TRUE) {
				for (int32 exp = 0; exp < nbVal; ++exp) {
					// Compute the different parameters
					if (PBExp.microBenchID == PAR)  {
						for (PBExp.runID = 0; PBExp.runID < PBExp.nbRun; (PBExp.runID)++) {
							PBExp.parDeg = tabVal[exp];
							for (int32 pID = 0; pID < PBExp.parDeg; ++pID) {
								PBExp.processID = PBExp.parDeg - pID - 1;
								PBExp.key = key++;
								ComputeParams(&PBExp, memList,PBExp.parDeg, nbVal );
								GenExp(fp2, &PBExp);
								nbExp++;
								PBExp.IOSize = PBBench->IOSize;
								PBExp.IOCount = PBBench->IOCount;
								PBExp.ignoreIO = PBBench->ignoreIO;
								
							}
						}
					}
					else {
						for (PBExp.runID = 0; PBExp.runID < PBExp.nbRun; (PBExp.runID)++) {
							PBExp.key = key++;
							ComputeParams(&PBExp, memList, tabVal[exp], nbVal);
							GenExp(fp2, &PBExp);
							nbExp ++;
							PBExp.IOSize = PBBench->IOSize;
							PBExp.IOCount = PBBench->IOCount;
							PBExp.ignoreIO = PBBench->ignoreIO;
						}
					}
				}
			}
			else
				printf("===================  Not Selected\n");
			fprintf(fp2, "\n");		
		}
		else finished = TRUE;
		prevExp = currExp;
		if (fp) fclose(fp);
	}
	fclose(fp2);							// close the output file if we opened it
	while (memList) {
		item *tmp = memList->next;
		free(memList);
		memList = tmp;
	}
	sprintf(str, "%d Experiments have been generated\n", nbExp);
	OutputString(OUT_LOG, str);

}
