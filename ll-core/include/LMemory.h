/* ============================================================================
 *  LMemory.h
 * ============================================================================

 *  Author:         (c) 2001 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 29, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2006/04/04 14:35:37 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/


#ifndef __LMemory__
#define __LMemory__

#include "LType.h"
#include "LConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LMemory_ID  0x8007

/*enables stats...*/
#define LMemory_STATS 1

enum { 
    LMemory_OUT_OF_MEMORY = LMemory_ID<<16,
    LMemory_NULL_POINTER_FREE_REQUEST,
    LMemory_ZERO_BYTE_ALLOCATION_REQUEST
};

#if LMemory_STATS
typedef struct LMemory_Stats
{
	ui4 mNumMalloc;
	ui8 mMallocByteCount;
	ui4 mMaxMalloc;
	
	ui4 mNumCalloc;
	ui8 mCallocByteCount;
	ui4 mMaxCalloc;
	
	ui4 mNumRealloc;	
	ui8 mReallocByteCount;
	ui4 mMaxRealloc;
	
	ui4 mNumFree;
	
} LMemory_Stats;
#endif

void*   LMemory_Malloc              (ui4 inSize);
void*   LMemory_Calloc              (ui4 inSize);
void*   LMemory_Realloc             (void* inPtr, ui4 inSize);
void    LMemory_Copy                (const void* inSource, void* outDest, ui4 inSize);
void    LMemory_Move                (const void* inSource, void* outDest, ui4 inSize);
void    LMemory_Set                 (void* outPtr, i1 inByte, ui4 inSize);
void    LMemory_Clear               (void* outPtr, ui4 inSize);
i4      LMemory_Compare             (const void* inPtr1, const void* inPtr2, ui4 inSize);

#if LMemory_STATS
void    LMemory_InitStats           ();
void    LMemory_GetStats            (LMemory_Stats* outStats);
#endif

#ifdef __LL_DEBUG__
i4      LMemory_GetBlocksCount      ();
#endif

void    _LMemory_Free               (void* inDummy, void** inPtrA);

#define LMemory_NewObject(T,O)      ((T*)(_LMemory_gTmp=LMemory_Malloc(sizeof(T)),  \
                                     AtMem_(T,_LMemory_gTmp)=(O),_LMemory_gTmp))
#define LMemory_Free(O)             _LMemory_Free(*(O),(void**)(O))
#define LMemory_DeleteObject(O)     LMemory_Free(O)

extern void* _LMemory_gTmp;

#ifdef __cplusplus
}
#endif

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
