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

/*************************** RANDOMC.H ***************** 2007-09-22 Agner Fog *
*
* This file contains class declarations and other definitions for the C++ 
* library of uniform random number generators.
*
* Overview of classes:
* ====================
* class CRandomMother:
* Random number generator of type Mother-of-All (Multiply with carry).
* Source file random.cpp
*
* Member functions (methods):
* ===========================
*
* All these classes have identical member functions:
*
* Constructor(uint32 seed):
* The seed can be any integer. Usually the time is used as seed.
* Executing a program twice with the same seed will give the same sequence of
* random numbers. A different seed will give a different sequence.
*
* void RandomInit(uint32 seed);
* Re-initializes the random number generator with a new seed.
*
* void RandomInitByArray(uint32 seeds[], int length);
* In CRandomMersenne only: Use this function if you want to initialize with
* a seed with more than 32 bits. All bits in the seeds[] array will influence
* the sequence of random numbers generated. length is the number of entries
* in the seeds[] array.
*
* double Random();
* Gives a floating point random number in the interval 0 <= x < 1.
* The resolution is 32 bits in CRandomMother and CRandomMersenne.
*
* int IRandom(int min, int max);
* Gives an integer random number in the interval min <= x <= max.
* (max-min < MAXINT).
* The precision is 2^-32 (defined as the difference in frequency between 
* possible output values). The frequencies are exact if max-min+1 is a
* power of 2.
*
* int IRandomX(int min, int max);
* Same as IRandom, but exact. In CRandomMersenne only.
* The frequencies of all output values are exactly the same for an 
* infinitely long sequence. (Only relevant for extremely long sequences).
*
* uint32 BRandom();
* Gives 32 random bits. 
*
* Copyright:
============
* © 1997 - 2007 Agner Fog. All software in this library is published under the
* GNU General Public License with the further restriction that it cannot be 
* used for gambling applications. See licence.htm
*******************************************************************************/

#ifndef RANDOM_H
#define RANDOM_H


#include "types.h"

/***********************************************************************
System-specific user interface functions
***********************************************************************/

void EndOfProgram(void);               // System-specific exit code (userintf.cpp)

void FatalError(char * ErrorText);     // System-specific error reporting (userintf.cpp)


/***********************************************************************
Define random number generator classes
***********************************************************************/


class CRandomMother {             // Encapsulate random number generator
public:
   void RandomInit(uint32 seed);       // Initialization
   int IRandom(int min, int max);      // Get integer random number in desired interval
   double Random();                    // Get floating point random number
   uint32 BRandom();                   // Output random bits
   CRandomMother(uint32 seed) {   // Constructor
      RandomInit(seed);}
protected:
   uint32 x[5];                        // History buffer
};

#endif
