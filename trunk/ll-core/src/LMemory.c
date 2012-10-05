/* ============================================================================
 *  LMemory.c
 * ============================================================================

 *  Author:         (c) 2001 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 29, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2006/04/04 14:35:37 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/


#include "LMemory.h"
#include "LException.h"
#include "LSystem.h"
#include <stdlib.h>
#include <string.h>


/* GLOBAL VARIABLES */
void* _LMemory_gTmp;

#if LMemory_STATS
ui4 LMemory_gNumMalloc;
ui8 LMemory_gMallocByteCount;
ui4 LMemory_gMaxMalloc;

ui4 LMemory_gNumCalloc;
ui8 LMemory_gCallocByteCount;
ui4 LMemory_gMaxCalloc;

ui4 LMemory_gNumRealloc;	
ui8 LMemory_gReallocByteCount;
ui4 LMemory_gMaxRealloc;

ui4 LMemory_gNumFree;
#endif

#ifdef __LL_DEBUG__
static i4 sBlocksCount=0;
#endif

/* ----------------------------------------------------------------------------
 *  Malloc
 * ----------------------------------------------------------------------------
*/
void* LMemory_Malloc(ui4 inSize){
    void* theTemp;

    if (inSize==0) 
        Throw(LMemory_ZERO_BYTE_ALLOCATION_REQUEST);

    theTemp=malloc(inSize);
    if (theTemp==NULL) Throw(LMemory_OUT_OF_MEMORY);

    #ifdef __LL_DEBUG__
    sBlocksCount++;
    #endif
    
#if LMemory_STATS
    LMemory_gNumMalloc++;
    LMemory_gMallocByteCount=LMemory_gMallocByteCount+inSize;
    if (inSize>LMemory_gMaxMalloc)
	    LMemory_gMaxMalloc=inSize;
#endif
    
    return theTemp;
}


/* ----------------------------------------------------------------------------
 *  Calloc
 * ----------------------------------------------------------------------------
*/
void* LMemory_Calloc(ui4 inSize){
    void* theTemp;
    
    theTemp=calloc(1,inSize);
    if (theTemp==NULL) Throw(LMemory_OUT_OF_MEMORY);

    #ifdef __LL_DEBUG__
    sBlocksCount++;
    #endif
    
#if LMemory_STATS
    LMemory_gNumCalloc++;
    LMemory_gCallocByteCount=LMemory_gCallocByteCount+inSize;
    if (inSize>LMemory_gMaxCalloc)
	    LMemory_gMaxCalloc=inSize;
#endif

    return theTemp;
}


/* ----------------------------------------------------------------------------
 *  Realloc
 * ----------------------------------------------------------------------------
*/
void* LMemory_Realloc(void* inPtr, ui4 inSize){
    void* theTemp;

    theTemp=realloc(inPtr,inSize);
    if (theTemp==NULL) 
        Throw(LMemory_OUT_OF_MEMORY);
    
#if LMemory_STATS
    LMemory_gNumRealloc++;
        LMemory_gReallocByteCount=LMemory_gReallocByteCount+inSize;
	if (inSize>LMemory_gMaxRealloc)
		LMemory_gMaxRealloc=inSize;
#endif

    return theTemp;
}


/* ----------------------------------------------------------------------------
 *  Free
 * ----------------------------------------------------------------------------
*/
void _LMemory_Free(void* inDummy, void** inPtrA){

    #ifdef __LL_DEBUG__
    if ((*inPtrA) == NULL) Throw(LMemory_NULL_POINTER_FREE_REQUEST);
    #endif

    #ifdef __LL_DEBUG__
    sBlocksCount--;
    #endif

#if LMemory_STATS
    LMemory_gNumFree++;
#endif
    
    if ((*inPtrA) != NULL) free(*(inPtrA));
    inDummy, /* CD051117 so the compiler won't complain */
    (*(inPtrA)) = NULL;
}


/* ----------------------------------------------------------------------------
 *  Copy
 * ----------------------------------------------------------------------------
*/
void  LMemory_Copy(const void* inSource, void* outDest, ui4 inSize){
    memcpy(outDest,inSource,inSize);
}


/* ----------------------------------------------------------------------------
 *  Move
 * ----------------------------------------------------------------------------
*/
void  LMemory_Move(const void* inSource, void* outDest, ui4 inSize){
    memmove(outDest,inSource,inSize);
}


/* ----------------------------------------------------------------------------
 *  Set
 * ----------------------------------------------------------------------------
*/
void  LMemory_Set(void* outPtr, i1 inByte, ui4 inSize){
    memset(outPtr, inByte, inSize);
}


/* ----------------------------------------------------------------------------
 *  Clear
 * ----------------------------------------------------------------------------
*/
void  LMemory_Clear(void* outPtr, ui4 inSize){
    memset(outPtr, 0, inSize);
}


/* ----------------------------------------------------------------------------
 *  Compare
 * ----------------------------------------------------------------------------
*/
i4  LMemory_Compare(const void* inPtr1, const void* inPtr2, ui4 inSize){
    return memcmp(inPtr1,inPtr2,inSize);
}

#ifdef __LL_DEBUG__
/* ----------------------------------------------------------------------------
 *  GetBlocksCount
 * ----------------------------------------------------------------------------
 * return number of blocks allocated with Malloc and Calloc and still not deallocated */
i4  LMemory_GetBlocksCount(){
    return sBlocksCount;
}
#endif

#if LMemory_STATS
void LMemory_InitStats ()
{
	LMemory_gNumMalloc=0;
	LMemory_gMallocByteCount=0;
	LMemory_gMaxMalloc=0;
	
	LMemory_gNumCalloc=0;
	LMemory_gCallocByteCount=0;
	LMemory_gMaxCalloc=0;
	
	LMemory_gNumRealloc=0;	
	LMemory_gReallocByteCount=0;
	LMemory_gMaxRealloc=0;

	LMemory_gNumFree=0;
}

void LMemory_GetStats (LMemory_Stats* outStats)
{
	outStats->mNumMalloc=LMemory_gNumMalloc;
	outStats->mMallocByteCount=LMemory_gMallocByteCount;
	outStats->mMaxMalloc=LMemory_gMaxMalloc;
	
	outStats->mNumCalloc=LMemory_gNumCalloc;
	outStats->mCallocByteCount=LMemory_gCallocByteCount;
	outStats->mMaxCalloc=LMemory_gMaxCalloc;

	outStats->mNumRealloc=LMemory_gNumRealloc;
	outStats->mReallocByteCount=LMemory_gReallocByteCount;
	outStats->mMaxRealloc=LMemory_gMaxRealloc;

	outStats->mNumFree=LMemory_gNumFree;
}
#endif


/* Copyright (C) 2001 Camil Demetrescu

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
