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

/**
 * Name        : flashIO.cpp
 * Author      : Luc Bouganim
 * Version     : V2.1
 * Description : Main file of the FlashIO tool
 */

#include "input.h"
#include "rndFormat.h"
#include "genBench.h"
#include "blocAlloc.h"
#include "random.h"
#include "microbench.h"
#include "output.h"
#include <direct.h>
#include <fstream>
#include <io.h>

/**
 * initialize the random generator
 */
int32 seed = 200;        		// random seed
CRandomMother rg(seed);         // make instance of random number generator
 
/**
 * Special synchronization when parallel instance are running
 */
void RunParBench(sParams *PB) {
	
	sResults 	*RB;				// output results
    char 		str[MAX_STR];		// Temporary string
	int 		pi;					
	FILE*		fp;
	boolean 	aSem;
			
    if(PB->processID != 0) { // This is a Slave process
		printf("WAITING MAINProc TO START .... ");	// Should wait the master process
    	pi = 0;
		while ((fp = fopen("_SEMAPHORE", "r")) == NULL) { // Synchronization is done using files 
			pi++;
			if (pi == 1000) {
				printf(".");
				pi = 0;
			}
		}
		printf("STARTING \n");
		fclose(fp);
		sprintf(str, "_SEM_%d", PB->processID); // Create a "file" semaphore
		if ((fp  = fopen(str, "w")) == NULL ) HandleError("main", "Cannot create SEMi file",GetLastError(), ERR_ABORT);
		fclose( fp );
    }
	else { //MASTER
		// Before starting, Master checks that no slave semaphore is still present (ie. previous execution ended correctly)
		printf("CHECKING THAT ALL INSTANCES OF PREVIOUS EXP HAVE ENDED \n");
		aSem = TRUE;
		while (aSem == TRUE){
			aSem = FALSE;
			for (int32 nf = 0; nf < MAX_PARALLEL; ++nf) {
				sprintf(str, "_SEM_%d", nf); // Master checks that no slave semaphore is still present
				if ((fp  = fopen(str, "r")) != NULL ) {
					aSem = TRUE;
					fclose( fp );
				}
			}
		}
		printf ("CHECKED\n");
		pi = 0;
		// Now it can starts ! put his semaphore
		printf("STARTING (CREATING _SEMAPHORE FILE)\n");
		while ((fp = fopen("_SEMAPHORE", "w")) == NULL) {
			pi++;
			if (pi == 100) {
				printf(".");
				pi = 0;
			}
		}
		printf ("STARTING !\n");
		fclose(fp);
	}
    // Finally run the bench 
    RB = runBench(PB);
	OutputResults(PB, RB);
    if (PB->processID == 0) { // MASTER
    	printf("WAITING FOR THE END OF ALL INSTANCES \n");
    	aSem = TRUE;
    	while (aSem == TRUE){
    		aSem = FALSE;
    		for (int32 nf = 0; nf < MAX_PARALLEL; ++nf) {
    			sprintf(str, "_SEM_%d", PB->processID);
    			if ((fp  = fopen(str, "r")) != NULL ) {
    				aSem = TRUE;
    				fclose( fp );
    			}
    		}
    	}
    	printf("ENDED\n");
    	printf("REMOVING _SEMAPHORE .... ");
    	pi = 0;
		while (remove("_SEMAPHORE") != 0) {
			pi++;
			if (pi == 100) {
				printf(".");
				pi = 0;
			}
		}
    	printf("REMOVED");
    }
    else { // SLAVE
    	sprintf(str, "_SEM_%d", PB->processID);
    	printf("REMOVING %s .... ", str);
    	{
    		pi = 0;
    		while (remove(str) != 0) {
    			pi++;
    			if (pi == 100) {
    				printf(".");
    				pi = 0;
    			}
    		}
    	}
    	printf("REMOVED");
    }
}

//g++ flashIO.cpp genBench.cpp input.cpp microbench.cpp output.cpp random.cpp rndFormat.cpp utility.cpp blocAlloc.cpp -O0 -g -Wall

/**
 * Main of FlashIO - read parameters and trigger apropriate action
 */
int main(int argc, char** argv) {
	sParams *PB;					// Input parameters
	sResults *RB;					// Output results
    DISK_GEOMETRY 	pdg;            // disk drive geometry structure
    BOOL 		bResult;        // generic results flag
    char 			str[MAX_STR];	// Temporary string
    
    // Create working directories
   	mkdir("Traces");
   	mkdir("Details");
   	mkdir("Results");

    printf("FLASHIO V2.3 ©Luc Bouganim - 2008-2009  See www.uflip.org\n");
    InitLogName("TRACES\\LOG.txt");
    sprintf(str, "==>	");
	for (int yy = 0; yy < argc; ++yy) {
			sprintf(str + strlen(str), " %s", argv[yy]);
		}
	strcat(str, "\n");

	OutputString(OUT_LOG, str);
	PB = GetTestParameters(argc, argv);

	InitFileNames(PB);
	OutputString(OUT_TRA, str);
	if (PB->deviceSize == 0) {
		bResult = GetDriveGeometry (&pdg, PB->deviceNum);
		if (bResult) {
	    	sprintf(str, "Cylinders = %I64d\n", pdg.Cylinders.QuadPart);
	    	OutputString(OUT_TRA, str);
	    	sprintf(str, "Tracks/cylinder = %d\n", (uint32) pdg.TracksPerCylinder);
	    	OutputString(OUT_TRA, str);
	    	sprintf(str, "Sectors/track = %d\n", (uint32) pdg.SectorsPerTrack);
	    	OutputString(OUT_TRA, str);
	    	sprintf(str, "Bytes/sector = %d\n", (uint32) pdg.BytesPerSector);
	    	OutputString(OUT_TRA, str);
	    	PB->deviceSize = (pdg.Cylinders.QuadPart * (uint32)pdg.TracksPerCylinder *
	    	      	(uint32)pdg.SectorsPerTrack * (uint32)pdg.BytesPerSector)/SECTOR;
	    	sprintf(str, "Disk size = %20.0f (Sectors) = %d (MB)\n", (float)PB->deviceSize, PB->deviceSize / (2 * 1024));
	    	OutputString(OUT_TRA, str);
	    	PB->deviceSize = ((uint32)(PB->deviceSize/BLOCK))*BLOCK; // Use only a size aligned on block.
	    	sprintf(str, "Disk size Aligned = %20.0f (Sectors) = %d (MB)\n", (float)PB->deviceSize, PB->deviceSize / (2 * 1024));
	    		    	OutputString(OUT_TRA, str);
	    }
	    else HandleError("main", "GetDriveGeometry failed", 0, ERR_ABORT);

		if (PB->trimBeforeRun)
		{
			if (DeviceSupportsTrim(PB->deviceNum)) {
				sprintf(str, "Device supports TRIM.\n");
				OutputString(OUT_TRA, str);
			} else
				HandleError("No TRIM", "Device does not support TRIM", 0, ERR_ABORT);
		}
	}    

	DWORD numPartitions = HasDrivePartitions(PB->deviceNum);
	//printf("Partitions: %d\n", numPartitions);
	//OutputString(OUT_TRA, str);
	if (numPartitions)
	{
		HandleError("This drive has partitions on it. Delete the partitions first.", "Let's not kill the system drive", 0, ERR_ABORT);
	}

	switch (PB->mainFunction) {
		case RND_FORMAT:
			rndFormat(PB);	
			break;
		case GEN_PREPARE:
			GenPrepare(PB);
			break;
		case GEN_BENCH:
			GenBench(PB);
			break;
		case RUN_BENCH:
			if (PB->parDeg > 1) RunParBench(PB);
			else { 
				RB = runBench(PB);
				OutputResults(PB, RB);
			}
			break;
		default:
			PrintHelp("");
			break;
	}
	return 0;
}
