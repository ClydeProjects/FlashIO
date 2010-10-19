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

#include "rndFormat.h"
#include "random.h"
#include "utility.h"
#include <winioctl.h>
#include <stdio.h>




void rndFormat(sParams *PB) {	
	DWORD     		dwBytes = 0;                        	// bytes read on each request
    LARGE_INTEGER	ullFilePos;								// position at which to do the next IO
    BOOL      		bStatus = TRUE;                       	// status from latest call
    HANDLE    		hFile = INVALID_HANDLE_VALUE;       	// file handle
    LPVOID    		lpRequest = NULL;						// IO buffer
	sCUKTimes 		cukTimes;				// a timer
	double	  		Clock;					// elapsed clock time
	double	  		dummy;					// elapsed user time
	double	  		pClock=0;				// elapsed clock time
	double			timeStep;				// Time for one step during format
	int32 			nbIO;					// Total number of IO to do in blocks
    int32			nbPages;				// Number of pags of 512 B in the device
    int32 			blockSize;				// Varying size of a block
    int32			ioPos;
    int32 			nbIODone;
    int32 			IOStep;
    int32			realPos;
    int32 			step;
    char 			str[MAX_STR];						// Temporary string
    int32* 			doneIO;
    
    nbPages = (unsigned long)(PB->deviceSize);
    nbPages = ((int32)(nbPages / 256))*256; // Must be a multiple of 256...

    // allocate and initialize the "doneIO" table of long
    doneIO = (int32 *)malloc(sizeof(int32)* nbPages);    
    if (doneIO == NULL) HandleError("rndFormat", "doneIO buffer allocation failed", 0, ERR_ABORT);
    for (int32 k = 0; k < nbPages; ++k) 
    	doneIO[k] = k;
	    
    // allocate IO request data buffer
	lpRequest = VirtualAlloc(NULL, 128 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);    //128 KB for larger blocks
    if (lpRequest == NULL) {
		free(doneIO);
		HandleError("rndFormat", "lpRequest buffer allocation failed", 0, ERR_ABORT);
	}

	//----------------------------------------------------------------------------------------------
	//Opening the device
    sprintf(str, "\\\\.\\PhysicalDrive%d", PB->deviceNum);
	hFile = CreateFile  (str,  // drive
			GENERIC_READ | GENERIC_WRITE, // desired access
			0,                            // share mode (none)
			NULL,                         // security attributes
			OPEN_EXISTING,                // open existing file			
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, // flags & attributes			
			NULL);                        // file template
	if (hFile == INVALID_HANDLE_VALUE) { 
		VirtualFree(lpRequest, 0, MEM_RELEASE);
		free(doneIO);
	   	HandleError("rndFormat", "Unable to open device",	GetLastError(), ERR_ABORT);											
	}
	    
	//-------------------MAIN LOOP Initialization ---------------------------------------------------
	nbIO = nbPages;
	//Start the timers
	StartTimers(&cukTimes);	
	GetElapsedTime(cukTimes, &pClock, &dummy, &dummy); // Get the elapsed time
	OutputString(OUT_TRA, "Starting formating the device\n");
	int formSize = 256; // from 128KB (256 sectors) to 512 B (1 sector)
	for (int pp = 0; pp < formSize*512; ++pp) 
		((byte *)lpRequest)[pp] = (byte)formSize;
	blockSize= formSize*512;
	int nbShrink = 9; // 9 steps to go from 128KB to 512B
	
	//-------------------  MAIN LOOP ------------------------------------------------------------------
	
	step = 1000;
	IOStep = 0;
	printf("\nFormating with blocks of size %d \n", formSize*512);
	for (nbIODone = 0; nbIODone < nbIO; ++nbIODone) {
		if (nbIODone > step) {
			printf(".");
			step += 1000;
		}
		
		if ((nbIO - nbIODone) <=(nbShrink -1)* (nbIO/9)) {
			if (nbShrink !=1)	{
				GetElapsedTime(cukTimes, &Clock, &dummy, &dummy); // Get the elapsed time
				timeStep = Clock - pClock;
				sprintf(str, "End of phase %d \n", 10-nbShrink);
				OutputString(OUT_TRA, str);
				sprintf(str, "		%u IOs of size %3.1f KB performed\n", (nbIODone - IOStep)/formSize, (double)formSize/2);
				OutputString(OUT_TRA, str);
				sprintf(str, "		Duration %f, (avg for each IO: %f)\n", timeStep, timeStep/((nbIODone - IOStep)/formSize));
				OutputString(OUT_TRA, str);
				IOStep = nbIODone;
				pClock = Clock;				
				formSize /=2;
				nbShrink--;
				for (int pp = 0; pp < formSize*512; ++pp) {
					((byte *)lpRequest)[pp] = (byte)formSize;
				}
				blockSize= formSize*512;
			}
			else printf("Strange ..... %u\n", nbIODone);
			printf("\nFormating with blocks of size %d \n", formSize*512);
		}
		ioPos= rg.IRandom(0,(nbIO - nbIODone)/formSize-1)*formSize;
		realPos = doneIO[ioPos];
		for (int32 ll = 0; ll < formSize; ++ll) {
			if (doneIO[ioPos + ll] == realPos+ll)	{
				doneIO[ioPos + ll] = doneIO[nbIO - nbIODone - formSize + ll + ll];
				nbIODone++;
			}
			else	{
				printf("PBM2 IN FORMAT !!!! %d %d %d \n",ll, doneIO[ioPos + ll], realPos+ll );
				CloseHandle(hFile);
				VirtualFree(lpRequest, 0, MEM_RELEASE);
				free(doneIO);
			  	exit(1);
			}
		}
		nbIODone--;
		ullFilePos.QuadPart = (LONGLONG) 512 * (LONGLONG) realPos;	/* beware the overflow! */
		// Set the pointer in file
		if (!SetFilePointerEx(hFile, ullFilePos, &ullFilePos, FILE_BEGIN)) {
			if (GetLastError() != NO_ERROR)	{
				sprintf(str, "Unable to set file pointer to %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
					ullFilePos.QuadPart/(int32)blockSize);
				CloseHandle(hFile);
				VirtualFree(lpRequest, 0, MEM_RELEASE);
				free(doneIO);
				HandleError("rndFormat", str, GetLastError(), ERR_ABORT);
			}
		}
		
		if (!(PB->fake))	{
			bStatus = WriteFile(hFile, lpRequest, blockSize, &dwBytes, NULL);
		}
		if (!bStatus) {
			sprintf(str, "File write failed - file pointer %I64d (in blocks %I64d)",ullFilePos.QuadPart, 
					ullFilePos.QuadPart/(int32)blockSize);
			CloseHandle(hFile);
			VirtualFree(lpRequest, 0, MEM_RELEASE);
			free(doneIO);
			HandleError("rndFormat", str, GetLastError(), ERR_ABORT);
		}
	}	
	GetElapsedTime(cukTimes, &Clock, &dummy, &dummy); // Get the elapsed time
	timeStep = Clock - pClock;
	sprintf(str, "End of phase %d \n", 10-nbShrink);
	OutputString(OUT_TRA, str);
	sprintf(str, "		%u IOs of size %3.1f KB performed\n", (nbIODone - IOStep)/formSize, (double)formSize/2);
	OutputString(OUT_TRA, str);
	sprintf(str, "		Duration %f, (avg for each IO: %f)\n", timeStep, timeStep/((nbIODone - IOStep)/formSize));
	OutputString(OUT_TRA, str);
	//-------------------------------------------------------------------------------
    // Common Exit, close handles, deallocate storage  
	CloseHandle(hFile);
	free(doneIO);
	VirtualFree(lpRequest, 0, MEM_RELEASE);
}

