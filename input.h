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

#ifndef __INPUT_H

#define __INPUT_H

#include "flashIO.h"

/********************************************************
*				Test Parameter Defaults
********************************************************/
	
/********************************************************
*				Functions
********************************************************/
 
/****************   Public functions   ***********************/

// main application loop
sParams* GetTestParameters(unsigned int argc, char** argv);
void PrintHelp(const char * varName);

/****************   Private functions   ***********************/
void InitParams(sParams* PB);
void InitParamsRF(sParams* PB);
void InitParamsGB(sParams* PB);
void InitParamsGP(sParams* PB);
void InitParamsR(sParams* PB);
void CheckParams (sParams* PB);
int32 GetValues(const char* varName, const char* varValue, sParams* PB, char* valueList, int32* tabVal);

/****************   Imported functions   ***********************/

#endif
