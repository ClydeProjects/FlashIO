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

#ifndef __UTILITY_H

#define __UTILITY_H

#include "flashIO.h"
#include "output.h"

#include <math.h>
#include <windows.h>
#include <winioctl.h>

// enumeration of error levels to HandleError
#define	ERR_SUCCESS					0	// used for non-error conditions such as progress indicators
#define	ERR_ABORT					1	// indicates a critical error has occured 
										// to HandleError()

/****************   sCUKTimes   ***********************/
// sCUKTimes is the container used to represent a timer. It is passed to various
// timing functions which initialize the timer or measure the elapsed time.
typedef struct {
	LARGE_INTEGER	liC;	// initial wall clock time
	LARGE_INTEGER	liU;	// initial user time
	LARGE_INTEGER	liK;	// initial kernel time
} sCUKTimes;



// Values for system call for retreiving smart attributes
#define DFP_RECEIVE_DRIVE_DATA 0x7C088
#define SMART_READ_ATTRIBUTE_VALUES 0xD0
#define IDE_EXECUTE_SMART_FUNCTION 0xB0
#define SMART_CYL_LOW 0x4F
#define SMART_CYL_HI 0xC2 

//structure du membre irDriveRegs de SENDCMDINPARAMS
typedef struct{
	byte bFeaturesReg;
	byte bSectorCountReg;
	byte bSectorNumberReg;
	byte bCylLowReg;
	byte bCylHighReg;
	byte bDriveHeadReg;
	byte bCommandReg;
	byte bReserved;
} IDEREGS;

// structure de commande des IO pour IDE
typedef struct{
	long cBufferSize;
	IDEREGS irDriveRegs;
	byte bDriveNumber;
	byte bReserved[3];
	long dwReserved[4];
	byte bBuffer[1];
} SENDCMDINPARAMS;

// Structure for sending TRIM to device
typedef struct{
	unsigned int key;
	unsigned short t1;
	unsigned short t2;
} TRIM_PAIR;


// Structure du membre DStatus de SENDCMDOUTPARAMS
typedef struct{
	byte bDriveError;
	byte bIDEStatus;
	byte bReserved[2];
	long dwReserved[2];
}DRIVERSTATUS;

// structure de commande des IO pour IDE
typedef struct{
	long cBufferSize;
	DRIVERSTATUS DStatus;
	byte bBuffer[512];
} SENDCMDOUTPARAMS;

/* These structures are copied from winioctl.h and ntddscsi.h from
 * Windows Driver Kit 7600.16385.1 because MinGW did'nt have updated
 * headers.
 */

#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER

// IOCTLs 0x0463 to 0x0468 reserved for dependent disk support.
#define IOCTL_STORAGE_QUERY_PROPERTY                CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES    CTL_CODE(IOCTL_STORAGE_BASE, 0x0501, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_ATA_PASS_THROUGH_DIRECT   CTL_CODE(IOCTL_SCSI_BASE, 0x040c, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// Structure for reporting back TRIM capability
typedef struct _DEVICE_TRIM_DESCRIPTOR {
    DWORD       Version;          // keep compatible with STORAGE_DESCRIPTOR_HEADER
    DWORD       Size;             // keep compatible with STORAGE_DESCRIPTOR_HEADER

    BOOLEAN     TrimEnabled;
} DEVICE_TRIM_DESCRIPTOR, *PDEVICE_TRIM_DESCRIPTOR;

// Types of queries
typedef enum _STORAGE_QUERY_TYPE {
    PropertyStandardQuery = 0,          // Retrieves the descriptor
    PropertyExistsQuery,                // Used to test whether the descriptor is supported
    PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
    PropertyQueryMaxDefined     // use to validate the value
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

// define some initial property id's
typedef enum _STORAGE_PROPERTY_ID {
    StorageDeviceProperty = 0,
    StorageAdapterProperty,
    StorageDeviceIdProperty,
    StorageDeviceUniqueIdProperty,              // See storduid.h for details
    StorageDeviceWriteCacheProperty,
    StorageMiniportProperty,
    StorageAccessAlignmentProperty,
    StorageDeviceSeekPenaltyProperty,
    StorageDeviceTrimProperty,					// This is the one we want
    StorageDeviceWriteAggregationProperty
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

// Data Set Management Query structure - additional parameters for specific queries can follow
// the header
typedef struct _STORAGE_PROPERTY_QUERY {
    // ID of the property being retrieved
    STORAGE_PROPERTY_ID PropertyId;
    // Flags indicating the type of query being performed
    STORAGE_QUERY_TYPE QueryType;
    // Space for additional parameters if necessary
    BYTE  AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

// ATA pass through direct structure.
typedef struct _ATA_PASS_THROUGH_DIRECT {
    USHORT Length;
    USHORT AtaFlags;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR ReservedAsUchar;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG ReservedAsUlong;
    PVOID DataBuffer;
    UCHAR PreviousTaskFile[8];
    UCHAR CurrentTaskFile[8];
} ATA_PASS_THROUGH_DIRECT, *PATA_PASS_THROUGH_DIRECT;

// ATA Pass Through Flags
#define ATA_FLAGS_DRDY_REQUIRED         (1 << 0)
#define ATA_FLAGS_DATA_IN               (1 << 1)
#define ATA_FLAGS_DATA_OUT              (1 << 2)
#define ATA_FLAGS_48BIT_COMMAND         (1 << 3)
#define ATA_FLAGS_USE_DMA               (1 << 4)
#define ATA_FLAGS_NO_MULTIPLE           (1 << 5)

/* End of structures from Windows Driver Kit 7600.16385.1 */

/****************   Public functions   ***********************/
void HandleError(const char* caller, const char* error, int32 errNum, int32 errLevel );
void GetElapsedTime(sCUKTimes cukTimes, double* ClockTime,  double* UserTime, double* KernelTime);
void StartTimers(sCUKTimes* cukTimes);
void OutputLine(const char* buffer);
boolean TimeElapsed(DWORD dwTestTime, boolean bInitTimer = FALSE);
double  ElapsedMicroseconds(LARGE_INTEGER StartTime, LARGE_INTEGER StopTime);
void MeasureIdleTime(double* fUser, double* fKernel, double fTestTime);
const char* UtilityVersion(void);
DWORD HasDrivePartitions(int32 NumDrive);
BOOL GetDriveGeometry(DISK_GEOMETRY *pdg, int32 NumDrive );
BOOL ExecuteTrim(int32 NumDrive, int32 MaxLBA);
BOOL DeviceSupportsTrim(int32 NumDrive);
int32 GetEraseCount(HANDLE hDevice, int32 NumDrive, int32 address);
void WaitMicrosec(int32 waitTime);


/****************   Private functions   ***********************/
void InitTimer(void);
#endif

