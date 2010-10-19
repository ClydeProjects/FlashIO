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

#ifndef __FLASHIO_H
#define __FLASHIO_H

#include <stdio.h>
#include "types.h"
#include "random.h"

extern CRandomMother rg;

// max number of characters in common strings (should be refined)
#define	MAX_STR 1000

#define MAX_PARTITIONS 256

// enumeration of buffering options
#define		NO_BUFFERING		0	// Neither HW nor SW buffering
#define		FS_BUFFERING		1	// Software Buffering
#define		HW_BUFFERING		2	// Hardware Buffering

// enumeration of IO types
#define		IO_SEQUENTIAL		0	// Sequential IOs
#define		IO_RANDOM			1	// Random IOs

// Files identifiers
#define 	OUT_ERR	0	// Error file
#define 	OUT_TIM	1	// Timing file
#define 	OUT_RES	2	// Result file
#define 	OUT_TRA	3	// Trace file
#define 	OUT_ALL	4	// File containing all results in a bidimensional table
#define 	OUT_SOR	5	// Same as OUT_ALL but sorted by IO cost
#define		OUT_TMP	6	// Temporary file (for sorting)
#define		OUT_COL	7	// name of the column in the ALL and SOR files
#define 	OUT_AVG	8	// Same as OUT_ALL but presents the running average (excluding IgnoreIO)
#define		OUT_LOG 9	// sequential log accross experiments
#define 	MAX_OUTPUT_FILES 10	// Number of Output files

// Main functions
#define 	RND_FORMAT 		1
#define 	GEN_PREPARE 	2
#define 	GEN_BENCH		3
#define 	RUN_BENCH		4


#define 	MAX_VARYING_VAL	100		// Maximum number of values for a varying parameter
#define 	MAX_SIZE_PARAM	30		// Maximum size of a parameter string in Exp Plan
#define		SECTOR 			512		// Number of byte in a sector (smallest I/O unit)
#define		BLOCK			256		// Number of sectors in a Block (for alignement of target offset)
#define 	MAX_PARALLEL 	32		// Maximum number of concurrent execution for PAR microbench

// Input Parameters
typedef struct { // See the comments in the help function of the input.cpp module
	int32		mainFunction;		// RND_FORMAT, GEN_PREPARE, GEN_BENCH, RUN_BENCH
	char		expPlan[MAX_STR];	// Filename containing the experimentation plan
	char		expSelect[MAX_STR];	// Filename of a file containing the selection of bench to generate wrt to expPlan
	char 		outName[MAX_STR]; 	// Filename for generating the bench or the preparation
	int32		nbRun;				// Number of run for each experiment
	int32		IOSize;				// size in sectors (512B) of each IO 
	int32		IOCount;			// Total Nb of IOs
	int32		IOCountSR;			// Total Nb of IOs for generating bench for SR
	int32		IOCountRR;			// Total Nb of IOs for generating bench for RR
	int32		IOCountSW;			// Total Nb of IOs for generating bench for RR
	int32		IOCountRW;			// Total Nb of IOs for generating bench for RW
	int32		IOCount2;			// Total Nb of IOs used only for determining startup/period
	int32		targetSize;			// Size in sectors of the space where test will be perfomed 
	int32 		targetOffset;		// offset in sectors to perform tests
	int32		targetSize2;			// Size in sectors of the space where test will be perfomed 
	int32 		targetOffset2;		// offset in sectors to perform tests
	int32 		nbPartition;		// For partitionned patterns (P)
	int32		IOShift;			// Small shift for testing unaligned IOs. in sectors.
	int32 		ignoreIO;			// Number of IOs to ignore when computing stats.
	int32		ratio;				// ration of pattern 1 / pattern 2 (if negative it is the contrary)
	int32		order;           	// For ordered patterns, linear increase or decrease (in blocks)
	int32 		pauseIO;			// pause between IOs (in microseconds)
	int32		pauseBurst;			// Pause between burst of B IOs (in microseconds)
	int32		burstIO;			// Number of IOs in a burst (B)
	int32		parDeg;				// Parallel degree
	int32 		processID;			// Process ID between 1 and 16 when parallel IOs
	char		comment[MAX_STR];	// user defined string containing information about the current test. No commas allowed.
	char		timeStamp[MAX_STR]; // string containing system generated time and date stamp
	char		machine[MAX_STR];	// machine name of system under test
	int32 		deviceNum;			// Device number
	int32	 	deviceSize;         // size of the device, in sectors
	int32	 	collectErase;		// indicate if Erase count (SMART) should be collected and its address
	int32		pauseExp;			// pause between exps (in ms)
	bool		fake;				// Run fake tests (no real IOs) - To check the arguments
	int32		bufferType;			// enumerated buffering option
	int32 		microBenchID;		// Bench ID between 0 and 9 (0 = preprare)
	int32 		expID;				// Experiment ID between 1 and 5
	int32		runID; 				// RunID between 1 and n (depending on the number of values)
	int32		warning; 			// Warning messageID - can be computed during the bench generation
	int32		key;				// a unique key to identify sequentially the experiment
	int32 		ignoreIOSR;			// Number of IOs to ignore when computing stats for generating bench for SR.
	int32 		ignoreIORR;			// Number of IOs to ignore when computing stats for generating bench for RR.
	int32 		ignoreIOSW;			// Number of IOs to ignore when computing stats for generating bench for SW.
	int32 		ignoreIORW;			// Number of IOs to ignore when computing stats for generating bench for RW.
	char		base[3];			// SR, RR, SW, RW for generating the bench
	char		base2[3];			// SR, RR, SW, RW for generating MIX bench
	bool		trimBeforeRun;		// If true, the device is trimmed before the benchmark.
} sParams;

// Output parameters 
typedef struct {
	double		lostTime;		// time lost between IOs 
	double		timeClock;		// measured clock time in seconds
	double		timeUser;		// measured user-mode processor time in milliseconds (ms)
	double		timeKernel;		// measured kernel-mode processor time in milliseconds (ms)
	double		minIO;			// measured clock time in seconds for the cheapest IO (not in place)
	double		maxIO;			// measured clock time in seconds for the more expensive IO (not in place)
	double		avgIO;			// measured clock time in seconds for IO (average)(not in place)
	double*		timing;			// detailed timings
	bool*		isWrite;		// detailed operationtype (TRUE = WRITE)
	bool*		isRND;			// detailed operation rendomness (TRUE = RANDOM)
	int32*		dPos;	 		// detailed block position (in block number)
	double 		stdDevIO;		// Standard deviation
	int32		initErase;		// Initial counter of erase
	int32		endErase;		// Final counter of erase
	int32 		eraseCount;		// Number of erase done during the test (for S.M.A.R.T SSD)
	bool		success;
} sResults;

#endif
