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
* mother.cpp   
* Author:        Agner Fog
* Date created:  1999
* Date modified: 2008
* 		This module has been modified by Luc Bouganim in 2008 keeping only 
* 		those functions necessary for the FlashIO module 
* Platform:This implementation uses 64-bit integers for intermediate calculations.
*          Works only on compilers that support 64-bit integers.
* Description:
* 		Random Number generator of type 'Mother-Of-All generator'.
*
* This is a multiply-with-carry type of random number generator
* invented by George Marsaglia.  The algorithm is:             
* S = 2111111111*X[n-4] + 1492*X[n-3] + 1776*X[n-2] + 5115*X[n-1] + C
* X[n] = S modulo 2^32
* C = floor(S / 2^32)
*
* Copyright 1999-2007 by Agner Fog. 
* GNU General Public License http://www.gnu.org/licenses/gpl.html
******************************************************************************/


#include "random.h"


// Output random bits
uint32 CRandomMother::BRandom() {
  uint64 sum;
  sum = (uint64)2111111111ul * (uint64)x[3] +
     (uint64)1492 * (uint64)(x[2]) +
     (uint64)1776 * (uint64)(x[1]) +
     (uint64)5115 * (uint64)(x[0]) +
     (uint64)x[4];
  x[3] = x[2];  x[2] = x[1];  x[1] = x[0];
  x[4] = uint32(sum >> 32);            // Carry
  x[0] = uint32(sum);                  // Low 32 bits of sum
  return x[0];
} 


// returns a random number between 0 and 1:
double CRandomMother::Random() {
   return (double)BRandom() * (1./(65536.*65536.));
}


// returns integer random number in desired interval:
int CRandomMother::IRandom(int min, int max) {
   // Output random integer in the interval min <= x <= max
   // Relative error on frequencies < 2^-32
   if (max <= min) {
      if (max == min) return min; else return 0x80000000;
   }
   // Multiply interval with random and truncate
   int r = int((max - min + 1) * Random()) + min; 
   if (r > max) r = max;
   return r;
}


// this function initializes the random number generator:
void CRandomMother::RandomInit (uint32 seed) {
  int i;
  uint32 s = seed;
  // make random numbers and put them into the buffer
  for (i = 0; i < 5; i++) {
    s = s * 29943829 - 1;
    x[i] = s;
  }
  // randomize some more
  for (i=0; i<19; i++) BRandom();
}
