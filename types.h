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

#ifndef __TYPES_H
#define __TYPES_H

// This should be adapted depending on the compiler and the OS
typedef		unsigned 	char		uint8;
typedef					char		int8;
typedef 	unsigned 	short		uint16;
typedef 				short		int16;

// Define integer types with known size: int32, uint32, int64, uint64.
// If this doesn't work then insert compiler-specific definitions here:
#if defined(__GNUC__)
  // Compilers supporting C99 or C++0x have inttypes.h defining these integer types
  #include <inttypes.h>
  typedef int32_t  int32;
  typedef uint32_t uint32;
  typedef int64_t int64;
  typedef uint64_t uint64;

  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#elif defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) 
   // 16 bit systems use long int for 32 bit integer
  typedef   signed long int int32;
  typedef unsigned long int uint32;
#elif defined(_MSC_VER)
  // Microsoft have their own definition
  typedef   signed __int32  int32;
  typedef unsigned __int32 uint32;
  typedef   signed __int64  int64;
  typedef unsigned __int64 uint64;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#else
  // This works with most compilers
  typedef signed int          int32;
  typedef unsigned int       uint32;
  typedef long long           int64;
  typedef unsigned long long uint64;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#endif

#define UINT32_MAX		0xFFFFFFFF
#define INT32_MAX		0x7FFFFFFF
#define INT32_MIN		0xFFFFFFFF
#define UINT16_MAX		0xFFFF
#define INT16_MAX		0x7FFF
#define INT16_MIN		0xFFFF
#define UINT8_MAX		0xFF
#define INT8_MAX		0x7F
#define INT8_MIN		0xFF

#define TRUE	1
#define FALSE	0

#endif


