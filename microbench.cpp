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

#include "microbench.h"
#include "random.h"
#include "flashIO.h"
#include "utility.h"
#include "genBench.h"
#include <winioctl.h>
#include <windows.h>
#include <stdio.h>


sResults* runBench(sParams* PB) {	
    DWORD			dwBytes = 0;                        	// bytes read on each request
    LARGE_INTEGER	ullFilePos;								// position for the next IO
    bool      		bStatus = TRUE;                       	// status from latest call
    int32     		FileFlags = 0;							// file flags
    HANDLE    		hFile = INVALID_HANDLE_VALUE;       	// file handle
    LPVOID    		lpRequest = NULL;						// IO buffer
	sResults*		RB = NULL;								// sRB to be returned
	sCUKTimes 		cukTimes;								// a timer
	double	  		fClock;									// elapsed clock time
	double	  		fUser;									// elapsed user time
	double	  		fKernel;								// elapsed kernel time
	double	  		fpClock=0;								// elapsed clock time
	double	  		fpUser=0;								// elapsed user time
	double	  		fpKernel=0;								// elapsed kernel time
	char 			ErrMsg[MAX_STR];						// Msg in case of error
    long 			currErase;								// Current number of erases
    int32 			i;										// Current IO 
    int32			LBA;									// position in sector for the next IO
    char			nameD[MAX_STR];							// Name of the device
    bool 			isSeq;									// Baseline pattern for normal patterns
    bool 			isRead;									// Baseline pattern for normal patterns
    bool 			isSeq1;									// Baseline pattern for mix patterns
    bool 			isRead1;								// Baseline pattern for mix patterns
    bool 			isSeq2;									// Baseline pattern for mix patterns	
    bool 			isRead2;								// Baseline pattern for mix patterns			
    bool 			doRead;									// for indicators
    bool 			doSeq;									// for indicators
    int32			iP1;									// Current IO for pattern 1
    int32			iP2;									// Current IO for pattern 2
    int32			pattern;								// Current Pattern (0 (no Mix), 1 or 2)
        
    
    RB = (sResults*)malloc(sizeof(sResults));	// allocate a new sResults
	if (RB == NULL) 	HandleError("RunBench", "Couldn't allocate sResults structure", 0, ERR_ABORT);
	
	RB->success 	= FALSE;		// by default, the test is not successful until we reach the end
	RB->minIO 		= INT32_MAX;	// measured clock time in seconds for the cheapest IO (not in place)
	RB->maxIO 		= 0;			// measured clock time in seconds for the more expensive IO (not in place)
	RB->avgIO 		= 0;			// measured clock time in seconds for IO (average)(not in place)
	RB->timeClock 	= 0;
	RB->lostTime	= 0;
	RB->timeKernel 	= 0;
	RB->timeUser 	= 0;
	RB->eraseCount  = 0;
	
	// Handle the special case of setup experiment where we want to perform Seq Read, then Rnd Writes, then
	// Seq Read again with countIOs.
	// We thus multiply IOCount by 3 to Count and detect the limit within the loop...
	if (strcasecmp(PB->comment, "SPE")== 0) PB->IOCount *= 3; 
	
	//allocate detailed measurement Tables
	RB->timing = (double *)malloc(sizeof(double)* PB->IOCount);	
	if (RB->timing == NULL) {
		free(RB);
		HandleError("RunBench", "Couldn't allocate timing table", 0, ERR_ABORT);
	}
	
	RB->dPos = (int32 *)malloc(sizeof(int32)* PB->IOCount);	
	if (RB->dPos == NULL) {
		free(RB->timing);
		free(RB);
		HandleError("RunBench", "Couldn't allocate dPos table", 0, ERR_ABORT);		
	}

	RB->isRND = (bool *)malloc(sizeof(bool)* PB->IOCount);	
	if (RB->isRND == NULL) {
		free(RB->dPos);
		free(RB->timing);
		free(RB);
		HandleError("RunBench", "Couldn't allocate isRND table", 0, ERR_ABORT);		
	}
	
	RB->isWrite = (bool *)malloc(sizeof(bool)* PB->IOCount);	
	if (RB->isWrite == NULL) {
		free(RB->isRND);
		free(RB->dPos);
		free(RB->timing);
		free(RB);
		HandleError("RunBench", "Couldn't allocate isWrite table", 0, ERR_ABORT);		
	}
		
	// allocate IO request data buffer
	lpRequest = VirtualAlloc(NULL, PB->IOSize * SECTOR, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);    
    if (lpRequest == NULL) {
		free(RB->isWrite);
		free(RB->isRND);
		free(RB->dPos);
		free(RB->timing);
		free(RB);
		HandleError("RunBench", "lpRequest buffer allocation failed", 0, ERR_ABORT);
	}

    if (!(PB->bufferType & FS_BUFFERING))		// if we're not doing FS buffering,
		FileFlags = FileFlags | FILE_FLAG_NO_BUFFERING;	// set flags appropriately
	if (!(PB->bufferType & HW_BUFFERING))		  // if we're not doing HW buffering
		FileFlags = FileFlags | FILE_FLAG_WRITE_THROUGH;  // set flags appropriately

	// Trim the device if required
	if (PB->trimBeforeRun)
		ExecuteTrim(PB->deviceNum, PB->deviceSize);

	// Open the device 
	sprintf(nameD, "\\\\.\\PhysicalDrive%d", PB->deviceNum);
	hFile = CreateFile  (nameD,  GENERIC_READ | GENERIC_WRITE, // desired access
								 FILE_SHARE_READ |FILE_SHARE_WRITE,  // share mode (all for // exp)
								 NULL,                         // security attributes
								 OPEN_EXISTING,                // open existing file
								 FileFlags,                    // flags & attributes
								 NULL);                        // file template

	if (hFile == INVALID_HANDLE_VALUE) {
		VirtualFree(lpRequest, 0, MEM_RELEASE);
		free(RB->isWrite);
		free(RB->isRND);
		free(RB->dPos);
		free(RB->timing);
		free(RB);
		HandleError("RunBench", "Unable to open the device",	GetLastError(), ERR_ABORT);											
	}
	
	//-------------------MAIN LOOP Initialization ---------------------------------------------------
	if (PB->collectErase != INT32_MAX) {
		currErase = GetEraseCount(hFile, PB->deviceNum, PB->collectErase);
		RB->initErase = currErase; /* initial nb of erase */
	}
	else RB->initErase = 0; 
	//Start the timers
	StartTimers(&cukTimes);	
	GetElapsedTime(cukTimes, &fClock, &fUser, &fKernel); // Get the elapsed time

	if (strcasecmp(PB->base, "SR") == 0) {isSeq = TRUE;		isRead = TRUE;		isSeq1 = TRUE;	isRead1 = TRUE;}
	if (strcasecmp(PB->base, "SW") == 0) {isSeq = TRUE;		isRead = FALSE;		isSeq1 = TRUE;	isRead1 = FALSE;}
	if (strcasecmp(PB->base, "RR") == 0) {isSeq = FALSE;	isRead = TRUE;		isSeq1 = FALSE;	isRead1 = TRUE;}
	if (strcasecmp(PB->base, "RW") == 0) {isSeq = FALSE;	isRead = FALSE;		isSeq1 = FALSE;	isRead1 = FALSE;}
	if (strcasecmp(PB->base2, "SR") == 0) {isSeq2 = TRUE;	isRead2 = TRUE;}
	if (strcasecmp(PB->base2, "SW") == 0) {isSeq2 = TRUE;	isRead2 = FALSE;}
	if (strcasecmp(PB->base2, "RR") == 0) {isSeq2 = FALSE;	isRead2 = TRUE;}
	if (strcasecmp(PB->base2, "RW") == 0) {isSeq2 = FALSE;	isRead2 = FALSE;}
			
	
	//-------------------MAIN LOOP---------------------------------------------------
	iP1 = 0;	// for Mix 
	iP2 = 0;	// for Mix
	for (i = 0; i < PB->IOCount; ++i) {

		if ((i == PB->IOCount/3) && (strcasecmp(PB->comment, "SPE")== 0))  {
			// Change pattern from Seq Read to Rand Writes
			isSeq = FALSE;
			isRead = FALSE;
		}
		if ((i == 2*PB->IOCount/3) && (strcasecmp(PB->comment, "SPE")== 0)) {
			// Change pattern from Rand Writes to Seq Read 
			isSeq = TRUE;
			isRead = TRUE;
		}
		// if MIX pattern, decide if we are currently doing Pattern1 or Pattern2
		if (PB->microBenchID == MIX) {
			if (PB->ratio >= 0) { 
				if ((i % (PB->ratio + 1)) == 0) pattern = 2;
				else pattern = 1;
			}
			if (PB->ratio < 0) {
				if ((i % (-PB->ratio + 1)) == 0) pattern = 1;
				else pattern = 2;
			}
		}
		else pattern = 0;

		if (pattern == 1) {
			if (isSeq1 == TRUE) {
				LBA = iP1 * PB->IOSize;
				LBA %= PB->targetSize;
				LBA += PB->targetOffset;
			}
			else LBA = PB->targetOffset + rg.IRandom(0, (PB->targetSize / PB->IOSize) - 1) * PB->IOSize;
			doRead = isRead1;
			doSeq = isSeq1;
			iP1++;
		}
		else if (pattern == 2) {
			if (isSeq2 == TRUE) {
				LBA = iP2 * PB->IOSize;
				LBA %= PB->targetSize2;
				LBA += PB->targetOffset2;
			}
			else LBA = PB->targetOffset2 + rg.IRandom(0, (PB->targetSize2 / PB->IOSize) - 1) * PB->IOSize;
			doRead = isRead2;
			doSeq = isSeq2;
			iP2++;
		}
		else {// pattern = 0
			if (isSeq == TRUE) {
				if (PB->nbPartition > 1) {
					int32 PS, Pi, Oi;
			
					PS = PB->targetSize/PB->nbPartition;
					Pi = i % PB->nbPartition;
					Oi = (((int32)(i / PB->nbPartition)) * PB->IOSize) % PS;
					LBA = PS * Pi + Oi + PB->IOShift;
				}
				else {
					LBA = PB->order*i*PB->IOSize + PB->IOShift;
				}
				LBA += PB->targetOffset;
				LBA %= (PB->targetSize - PB->IOSize);

			}
			else LBA = PB->targetOffset + rg.IRandom(0, (PB->targetSize / PB->IOSize) - 1) * PB->IOSize + PB->IOShift;
			doRead = isRead;
			doSeq = isSeq;
		}
				
		if ((LBA<0) ||(LBA>PB->deviceSize)) {
			sprintf(ErrMsg, "OUT OF BOUND LBA : LBA %d Dev Size %d TS %d TO %d Shift %d TS2 %d TO2 %d IOSIZE %d \n",
					LBA, PB->deviceSize, PB->targetSize, PB->targetOffset, PB->IOShift, PB->targetSize2, PB->targetOffset2, PB->IOSize);
			CloseHandle(hFile);
			VirtualFree(lpRequest, 0, MEM_RELEASE);
           	free(RB->isWrite);
           	free(RB->isRND);
           	free(RB->dPos);
           	free(RB->timing);
           	free(RB);
			HandleError("RunBench", ErrMsg, GetLastError(), ERR_ABORT);
		}
					
		// ----------------------------------- COMMON FOR ANY IO
		ullFilePos.QuadPart = LBA;
		ullFilePos.QuadPart *= SECTOR;
		
		// Set the pointer in file
		if (!SetFilePointerEx(hFile, ullFilePos, &ullFilePos, FILE_BEGIN)) {
			if (GetLastError() != NO_ERROR)	{
				sprintf(ErrMsg, "Unable to set file pointer to %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
					ullFilePos.QuadPart/(PB->IOSize * SECTOR));
				CloseHandle(hFile);
				VirtualFree(lpRequest, 0, MEM_RELEASE);                
                free(RB->isWrite);
                free(RB->isRND);
                free(RB->dPos);
                free(RB->timing);
                free(RB);
				HandleError("RunBench", ErrMsg, GetLastError(), ERR_ABORT);
			}
		}
		GetElapsedTime(cukTimes, &fpClock, &fpUser, &fpKernel); // Get the elapsed time
		RB->lostTime += fpClock - fClock;

		if (doSeq)
			RB->isRND[i] = FALSE;
		else
			RB->isRND[i] = TRUE;

		if (doRead) {// READ case
			RB->isWrite[i] = FALSE;
			//printf("for iteration %d READING block %I64d\n", i, ullFilePos.QuadPart);
			if (PB->fake == FALSE) {
				bStatus = ReadFile(hFile, lpRequest, PB->IOSize * SECTOR, &dwBytes, NULL);
				if (!bStatus) {
					sprintf(ErrMsg, "File read failed - file pointer %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
						ullFilePos.QuadPart/(PB->IOSize * SECTOR));
					CloseHandle(hFile);
					VirtualFree(lpRequest, 0, MEM_RELEASE);                
					free(RB->isWrite);
					free(RB->isRND);
					free(RB->dPos);
					free(RB->timing);
					free(RB);
					HandleError("RunBench", ErrMsg, GetLastError(), ERR_ABORT);
				}
				if (dwBytes == 0) {
					sprintf(ErrMsg, "Early EOF received - file pointer %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
						ullFilePos.QuadPart/(PB->IOSize * SECTOR));
					CloseHandle(hFile);
					VirtualFree(lpRequest, 0, MEM_RELEASE);                
					free(RB->isWrite);
					free(RB->isRND);
					free(RB->dPos);
					free(RB->timing);
					free(RB);
					HandleError("RunBench", ErrMsg, GetLastError(), ERR_ABORT);
				}
			}
		}
		else {
			// WRITE case                                     
			RB->isWrite[i] = TRUE;
			//printf("for iteration %d WRITING block %I64d of size %lu\n", i, ullFilePos.QuadPart/SECTOR, PB->IOSize*SECTOR);
			if (PB->fake == FALSE) {
				bStatus = WriteFile(hFile, lpRequest,PB->IOSize * SECTOR, &dwBytes, NULL);
				if (!bStatus) {
					sprintf(ErrMsg, "File write failed - file pointer %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
						ullFilePos.QuadPart/(PB->IOSize*SECTOR));
					CloseHandle(hFile);
					VirtualFree(lpRequest, 0, MEM_RELEASE);                
					free(RB->isWrite);
					free(RB->isRND);
					free(RB->dPos);
					free(RB->timing);
					free(RB);
					HandleError("RunBench", ErrMsg, GetLastError(), ERR_ABORT);
				}
			}
		}
		GetElapsedTime(cukTimes, &fClock, &fUser, &fKernel); // Get the elapsed time
		RB->timing[i] = fClock - fpClock;
		RB->dPos[i] = ullFilePos.QuadPart/SECTOR;
		RB->timeClock 	+= fClock - fpClock;		// clock time
		RB->timeKernel += fKernel -fpKernel;	// kernel time
		RB->timeUser 	+= fUser - fpUser;		// user time
		if (i >= PB->ignoreIO) {
			if (fClock - fpClock < RB->minIO) RB->minIO = fClock - fpClock; 
			if (fClock - fpClock > RB->maxIO) RB->maxIO = fClock - fpClock; 
			RB->avgIO += fClock - fpClock;
		}
		// makes the I/O pause
		if ((PB->pauseIO != 0) && (PB->fake == FALSE)) 
			WaitMicrosec((unsigned long)PB->pauseIO);
		if (((i+1) % PB->burstIO == 0) && (PB->pauseBurst != 0) && (PB->fake == FALSE))
		{
			//printf ("%lu pause\n", i);
			WaitMicrosec((unsigned long)PB->pauseBurst);
		}
	}
	if (PB->collectErase != INT32_MAX) {
		int32 x;
		
		x = currErase;
		currErase = GetEraseCount(hFile, PB->deviceNum, PB->collectErase);
		RB->eraseCount += currErase -x;
		RB->endErase = currErase; /* final nb of erase */
	}
	else RB->endErase = 0;
		
	RB->avgIO /= (float)(PB->IOCount - PB->ignoreIO);
		
	// Computing some stats
	RB->stdDevIO = 0;
	for (int32 it = PB->ignoreIO; it < PB->IOCount; ++it) {
		RB->stdDevIO += (RB->timing[it] - RB->avgIO)*(RB->timing[it] - RB->avgIO);
	}
	RB->stdDevIO /= (float)(PB->IOCount - PB->ignoreIO); 
	RB->stdDevIO = sqrt(RB->stdDevIO);
	printf("Erases [%d],  ", RB->eraseCount);
	printf("AvgIO [%d]\n ", (int32) (RB->avgIO * 1000000.0));
	
	RB->success = TRUE;	// success!
	//-------------------------------------------------------------------------------
    // Common Exit, close handles, deallocate storage  
	CloseHandle(hFile);
	VirtualFree(lpRequest, 0, MEM_RELEASE);
    // make the final pause
    if ((PB->pauseExp != 0)  && (PB->fake == FALSE))WaitMicrosec((unsigned long) PB->pauseExp);
    return RB;
}

