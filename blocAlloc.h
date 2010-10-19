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

#ifndef BLOCALLOC_H_
#define BLOCALLOC_H_
#include "types.h"

typedef struct elt {
	int32	inf;
	int32	sup;
	struct elt*	prev;
	struct elt*	next;
} item;


item* InitMemList(int32 size);
int32 MemMinAddress(item* list);
int32 MemSearch(item* list, int32 size);
//int32 MemConstrainedSearch(item *list, int32 A, int32 B, int32 size);
int32 MemAlloc(item *list, int32 A, int32 B);
int32 MemAllocNearestAfterA(item *list, int32 A, int32 size);

#endif /*BLOCALLOC_H_*/
