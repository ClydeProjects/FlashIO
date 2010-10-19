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

#include <time.h>
#include "utility.h"
#include "output.h"
#include <windows.h>

// 64-bit move used to set next read/write file offset
#define Copy64(Dest, Src)  *( (PULONGLONG) &(Dest)) = *( (PULONGLONG) &(Src)) 

LARGE_INTEGER   TicksPerSecond = {{0,0}};	// Global ticks per second
double			TicksPerMicrosecond = 0;// Global ticks/microsecond

/****************   OutputLine()   ***********************/
// OutputLine() outputs the given NULL-terminated string to console, and additionally, if specified, the error target
void OutputLine(const char* buffer) {
	OutputString(OUT_ERR, buffer);
	OutputString(OUT_LOG, buffer);
	OutputString(OUT_TRA, buffer);
	fprintf(stderr, "%s\n", buffer);	// output to console
}

/****************   HandleError()   ***********************/
// HandleError() provides error handling. It takes five arguments and depending
// upon the values of those arguments, decides how to handle the error. The arguments are:
//		caller	-	string containing the name of the function in which the error
//		error		-	string containing the error message
//		errNum		-	error number
//		errLevel	-	severity of the error	(can be ERR_SUCCESS or ERR_ABORT)
//

void HandleError(const char* caller, const char* error, int32 errNum, int32 errLevel) {
	
	char		buffer[MAX_STR] = ""; 
	
	sprintf(buffer, "%s: %s (%d)\n", caller,  error,  errNum);
	OutputLine(buffer);	
	if (errLevel == ERR_ABORT) {
		OutputLine("Exiting...\n");
		WaitMicrosec(2000);
		exit(1);	// kill the program
	}
}

/****************   InitTimer()   ***********************/
// InitTimer is an internal function which sets the number of ticks per second and ticks per ms.
void InitTimer(void)
	{
	QueryPerformanceFrequency(&TicksPerSecond);
	TicksPerMicrosecond = (float) (TicksPerSecond.LowPart/ 1E6);
	return;
	}

/****************   StartTimers()   ***********************/
// StartTimers() initializes a timer which is returned in cukTimes.
// The cukTimes structure can later be passed to GetElapsedTime() to retrieve the
// elapsed time on that particular timer.
//
// There are no limitations on the number of outstanding timers.
void StartTimers(sCUKTimes* cukTimes)
	{
	FILETIME  K, U, ignore;

	if (!QueryPerformanceCounter(&cukTimes->liC)) {
		// no performance counter available
		HandleError("InitTimer", "This system does not support a high-resolution performance counter", GetLastError(), ERR_ABORT);
	}
	GetProcessTimes(
			GetCurrentProcess(), // me              
			&ignore,	// when the process was created
			&ignore,	// when the process was destroyed
			&K,			// time the process has spent in kernel mode
			&U			// time the process has spent in user mode
			);
	Copy64(cukTimes->liU, U); // copy int64
	Copy64(cukTimes->liK, K); // copy int64
	return;
	}

/****************   GetElapsedTime()   ***********************/ 
// GetElapsedTime() takes a timer initalized by StartTimers() along with three pointers to doubles.
// It returns the elapsed wall clock time along with the consumed user and kernel mode times.
void GetElapsedTime(sCUKTimes cukTimes,
					  double* ClockTime, 
					  double* UserTime, 
					  double* KernelTime)
	{
	LARGE_INTEGER StopTime;
	FILETIME  K, U, ignore;
	LARGE_INTEGER		StopTimeU; 
	LARGE_INTEGER		StopTimeK;

	// if this is a first-time call, initialize the globals.
	if (TicksPerSecond.LowPart == 0) InitTimer();
	// Get the current tick count
	QueryPerformanceCounter(&StopTime);
	// convert ticks to seconds. 
	*ClockTime = ((double) (StopTime.QuadPart - cukTimes.liC.QuadPart))
		/ TicksPerMicrosecond/1e6;	
	// get user an kernel times
	GetProcessTimes(
			GetCurrentProcess(), // me              
			&ignore,			// when the process was created
			&ignore,			// when the process was destroyed
			&K,					// time the process has spent in kernel mode
			&U					// time the process has spent in user mode
			);
	Copy64(StopTimeU, U);
	Copy64(StopTimeK, K);		// copy times to 64 bit ints
	// do the math (converting to seconds), times are in 100 ns units
	*UserTime   = (double) (StopTimeU.QuadPart - cukTimes.liU.QuadPart)/1e7;
	*KernelTime = (double) (StopTimeK.QuadPart - cukTimes.liK.QuadPart)/1e7;
	return;
	}


/****************   ElapsedMicroseconds()   ***********************/ 
// ElapsedMicroseconds() takes a start and stop time which were returned from
// QueryPerformanceCounter(). It returns the delta between the two in microseconds.
double  ElapsedMicroseconds(LARGE_INTEGER StartTime, LARGE_INTEGER StopTime) {
	LARGE_INTEGER dif;
	double delta;
	
	// if this is a first-time call, initialize the globals.
	if (TicksPerSecond.LowPart == 0) InitTimer();
	// compute the ticks elapsed since start time.
	dif.QuadPart =  (  StopTime.QuadPart -  StartTime.QuadPart);
	// convert ticks to microseconds. (first do low part)
	delta = dif.LowPart / TicksPerMicrosecond;		// low part
	 if (dif.HighPart != 0)							// high part 
		delta +=  ldexp((double) dif.HighPart,32)/TicksPerMicrosecond;
	return delta;
	}


/****************   TimeElapsed()   ***********************/ 
// When TimeElapsed() is first called or bInitTimer is set to TRUE, it starts timing from the time
// it was first called. When the dwTestTime seconds have elapsed, TimeElapsed() returns TRUE.
// Otherwise it returns FALSE.
boolean TimeElapsed(DWORD dwTestTime, boolean bInitTimer) {
	static LARGE_INTEGER liStart = {0};
	LARGE_INTEGER liStop;

	if ((liStart.QuadPart == 0) || (bInitTimer))		// first time through. Init timers.
		QueryPerformanceCounter(&liStart);
	QueryPerformanceCounter(&liStop);
	if ((ElapsedMicroseconds(liStart, liStop) / 1000000) >= dwTestTime)	// has the time elapsed?
		return TRUE;
	return FALSE;
}


/****************   WaitMicrosec()   ***********************/ 
// WaitMicrosec() waits (actively) the number of microseconds (parameter)
void WaitMicrosec(int32 waitTime) {

	sCUKTimes 		cukTimes;				// a timer
	double	  		fClock;					// elapsed clock time
	double	  		fpClock;				// elapsed clock time
	double	  		fUser;					// elapsed user time
	double	  		fKernel;				// elapsed kernel time
		
	//Start the timers
	StartTimers(&cukTimes);	
	GetElapsedTime(cukTimes, &fClock, &fUser, &fKernel); // Get the elapsed time
	while ((fpClock - fClock)*1000000.0 < (float)waitTime - 8)GetElapsedTime(cukTimes, &fpClock, &fUser, &fKernel);
}

BOOL ExecuteTrim(int32 NumDrive, int32 MaxLBA)
{
	// Intel undocumented vendor call for performing TRIM on Intel disks.
	HANDLE hDevice;               // handle to the drive to be examined
	BOOL bResult;                 // results flag
	DWORD cbBytesReturned = 0;    // discard results
	int32 numTrims = 512;		  // Number of blocks to send with the command
	TRIM_PAIR* trimPairs;
	char nameDrive[200];

	sprintf(nameDrive, "\\\\.\\PhysicalDrive%d", NumDrive);

	hDevice = CreateFile(nameDrive,  // drive
						GENERIC_READ|GENERIC_WRITE,                // no access to the drive
						FILE_SHARE_READ | FILE_SHARE_WRITE,// share mode

						NULL,             // default security attributes
						OPEN_EXISTING,    // disposition
						0,                // file attributes
						NULL);            // do not copy file attributes

	if (hDevice == INVALID_HANDLE_VALUE)
		return (FALSE);

	int currentLBA = 0;

	while (currentLBA < MaxLBA)
	{
		trimPairs = (TRIM_PAIR *)malloc(numTrims * sizeof(TRIM_PAIR));
		memset(trimPairs, 0, sizeof(TRIM_PAIR)*numTrims) ;

		for (int i=0;i<numTrims;i++)
		{
			TRIM_PAIR tr;
			tr.key = currentLBA;
			tr.t1 = 0x0;
			if ((currentLBA + 0xffff) >= MaxLBA)
				tr.t2 = MaxLBA - currentLBA;
			else
				tr.t2 = 0xffff;
			trimPairs[i] = tr;

			currentLBA += 0xffff;

			if (currentLBA >= MaxLBA)
				break;
		}

		_ATA_PASS_THROUGH_DIRECT aa;
		memset(&aa, 0, sizeof(_ATA_PASS_THROUGH_DIRECT));

		aa.Length = sizeof(_ATA_PASS_THROUGH_DIRECT);
		aa.AtaFlags = ATA_FLAGS_USE_DMA | ATA_FLAGS_DATA_OUT;
		aa.DataTransferLength = sizeof(TRIM_PAIR) * numTrims;
		aa.TimeOutValue = 5; // Seconds

		aa.DataBuffer = trimPairs;

		aa.CurrentTaskFile[0] = 1;			// Features
		aa.CurrentTaskFile[1] = sizeof(TRIM_PAIR);	// Sector Count
		aa.CurrentTaskFile[2] = 0;			// Sector Number Register
		aa.CurrentTaskFile[3] = 0;			// Cylinder Low Register
		aa.CurrentTaskFile[4] = 0;			// Cylinder High Register
		aa.CurrentTaskFile[5] = 0x40;		// Device/Head Register
		aa.CurrentTaskFile[6] = 0x84; 		// ATA vendor call ( Intel TRIM command )
		aa.CurrentTaskFile[7] = 0;			// Reserved

		SetLastError(0);
		// ATA_PASS_THROUGHT_DIRECT returns the result in the same array we pass to it.
		bResult = DeviceIoControl(hDevice,  // device to be queried
				IOCTL_ATA_PASS_THROUGH_DIRECT,  // operation to perform
										 &aa, sizeof(aa), // no input buffer
										 &aa, sizeof(aa),     // output buffer
										 &cbBytesReturned,                 // # bytes returned
										(LPOVERLAPPED) NULL);  // synchronous I/O

		DWORD dwError = GetLastError();
		if (dwError != 0 || !bResult)
			HandleError("TRIM failed", "Some error occurred while performing TRIM. Remember to run FlashIO with Administrator rights.", 0, ERR_ABORT); // If access denied, it can also be a non supported call.

		if (aa.CurrentTaskFile[0] != 0)
			HandleError("TRIM failed", "The command was sent to the device. But it returned an error.", 0, ERR_ABORT);

		free(trimPairs);
	}

	CloseHandle(hDevice);

	return true;
}

DWORD HasDrivePartitions(int32 NumDrive)
{
	HANDLE hDevice;               // handle to the drive to be examined
	BOOL bResult;                 // results flag
	DWORD BytesReturned;
	char nameDrive[200];

	DWORD size = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + 8 * sizeof(PARTITION_INFORMATION_EX);

	DRIVE_LAYOUT_INFORMATION_EX *pDriveInfoEx;

	sprintf(nameDrive,"\\\\.\\PhysicalDrive%d", NumDrive);

	hDevice = CreateFile(nameDrive,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

	if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		return 1;
	}

	pDriveInfoEx = (DRIVE_LAYOUT_INFORMATION_EX*)new char[size];

	bResult = DeviceIoControl(hDevice,IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, pDriveInfoEx, size, &BytesReturned, NULL);

	DWORD res = pDriveInfoEx->PartitionCount;

	delete[] (char*) pDriveInfoEx;
	CloseHandle(hDevice);

	return res;
}

BOOL GetDriveGeometry(DISK_GEOMETRY *pdg, int32 NumDrive )
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult;                 // results flag
  DWORD junk;                   // discard results
  char nameDrive[200];
  
  sprintf(nameDrive, "\\\\.\\PhysicalDrive%d", NumDrive);

  hDevice = CreateFile(nameDrive,  // drive 
                    0,                // no access to the drive
                    FILE_SHARE_READ | // share mode
                    FILE_SHARE_WRITE, 
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    0,                // file attributes
                    NULL);            // do not copy file attributes

  if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
  {
    return (FALSE);
  }

  bResult = DeviceIoControl(hDevice,  // device to be queried
      IOCTL_DISK_GET_DRIVE_GEOMETRY,  // operation to perform
                             NULL, 0, // no input buffer
                            pdg, sizeof(*pdg),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O

  CloseHandle(hDevice);

  return (bResult);
}

BOOL DeviceSupportsTrim(int32 NumDrive)
{
	HANDLE hDevice;               // handle to the drive to be examined
	BOOL bResult;                 // results flag
	DWORD cbBytesReturned;                   // discard results

	char nameDrive[200];

	sprintf(nameDrive, "\\\\.\\PhysicalDrive%d", NumDrive);

	hDevice = CreateFile(nameDrive,  // drive
	                    0,                // no access to the drive
	                    FILE_SHARE_READ | // share mode
	                    FILE_SHARE_WRITE,
	                    NULL,             // default security attributes
	                    OPEN_EXISTING,    // disposition
	                    0,                // file attributes
	                    NULL);            // do not copy file attributes

	if (hDevice == INVALID_HANDLE_VALUE)
		return (FALSE);

	STORAGE_PROPERTY_QUERY query;
	char buffer [10000];

	query.PropertyId = StorageDeviceTrimProperty;
	query.QueryType = PropertyStandardQuery;

	bResult = DeviceIoControl(hDevice,  // device to be queried
			IOCTL_STORAGE_QUERY_PROPERTY,  // operation to perform
	                             &query, sizeof(query), // no input buffer
	                             &buffer, sizeof(buffer),     // output buffer
	                            &cbBytesReturned,                 // # bytes returned
	                            (LPOVERLAPPED) NULL);  // synchronous I/O
	DWORD dwError = GetLastError();
	if (dwError == ERROR_NOT_SUPPORTED || !bResult)
		HandleError("DeviceSupportTrim", "The device driver do not support TRIM. Upgrade to newest driver.", 0, ERR_ABORT);

	DEVICE_TRIM_DESCRIPTOR *description = (DEVICE_TRIM_DESCRIPTOR *) &buffer;

	CloseHandle(hDevice);

	return description->TrimEnabled;
}

int32 GetEraseCount(HANDLE hDevice, int32 NumDrive, int32 address)
{
  BOOL bResult;                 // results flag
  DWORD junk;                   // discard results
  SENDCMDINPARAMS pSCIP;
  SENDCMDOUTPARAMS pSCOP;
 
  // necessary values for reading SMART attributes
  pSCIP.bDriveNumber = NumDrive; 		// device number
  pSCIP.cBufferSize = 512;		 		// buffer size for reading data
  pSCIP.irDriveRegs.bDriveHeadReg = 0xA0 | ((NumDrive & 1) << 4); 	
  pSCIP.irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_VALUES; 	// reading attributes
  pSCIP.irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION; 		// Execute a SMART Function
  pSCIP.irDriveRegs.bSectorCountReg = 1; 							// 1 sector to read
  pSCIP.irDriveRegs.bSectorNumberReg = 1;							// Sector n° 1
  pSCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;						// Cylinder number HI
  pSCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;						// Cylinder number LOW
	
  bResult = DeviceIoControl(hDevice,  // device to be queried
		  	DFP_RECEIVE_DRIVE_DATA,  // operation to perform
                             (void *) &pSCIP, sizeof(pSCIP), 
                             (void *) &pSCOP, sizeof(pSCOP),
                             &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O
  if (bResult)  return(*((int32 *) &(pSCOP.bBuffer[address])));
  return(0);
}
