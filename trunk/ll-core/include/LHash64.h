/* ============================================================================
 *  LHash64.h
 * ============================================================================

 *  Author:         (C) 2002 Stefano Emiliozzi
 *  License:        See the end of this file for license information
 *  Created:        December 28, 2002
 *  Module:         LL

 *  Last changed:   $Date: 2007/01/04 15:41:39 $
 *  Changed by:     $Author: ribbi $   
 *  Revision:       $Revision: 1.1 $    
*/

#ifndef __LHash64__
#define __LHash64__

#include "LType.h"
#include "LSystem.h"
#include "LArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LHash64_ID   0x8028

#define LHash64_STATS 1

typedef struct LHash64 LHash64;

struct LHash64 
{
    struct TSlot*   mData;
    ui4             mDataSize;
    ui4             mItemsCount;    /* number of items                            */
    ui4             mEntriesCount;  /* number of entries                          */
    ui1             mHashLength;    /* length of the hashing value                */
    ui4             mCollisionKeys; /* number of keys with at least one collision */
    ui8             mRandomOdd;     /* random odd 64bit integer used for hashing  */
    Bool            mDebug;         /* used to set on/off debug                   */
    
#if LHash64_STATS
    ui4             mMaxColListLength;
#endif
};

enum 
{
    LHash64_OBJECT_NULL_POINTER = LHash64_ID<<16
};


LHash64*   LHash64_New                    ();
void       LHash64_Delete                 (LHash64** ThisA);
void       LHash64_InsertItem             (LHash64* This, void* inItem, ui8 inKey);
void       LHash64_RemoveItem             (LHash64* This, ui8 inKey);
Bool       LHash64_IsInTable              (LHash64* This, ui8 inKey);
void       LHash64_RemoveAllItems         (LHash64* This);
void*      LHash64_GetItemByKey           (LHash64* This, ui8 inKey);
ui4        LHash64_GetUsedMem             (LHash64* This);
ui4        LHash64_GetItemsCount          (LHash64* This);
ui4        LHash64_GetCollisionKeysCount  (LHash64* This);
void       LHash64_SetDebug               (LHash64* This, Bool inDebug);
void       LHash64_Dump                   (LHash64* This);

LArray*    LHash64_GetAllItems            (LHash64* This);
LArray*    LHash64_GetAllKeys             (LHash64* This);

void       LHash64_ReplaceItemByKey       (LHash64* This, ui8 inKey, void *inNewItem);

#if LHash_STATS
ui4        LHash64_GetMaxColListLength    (LHash64* This);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* Copyright (C) 2002 Stefano Emiliozzi

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
