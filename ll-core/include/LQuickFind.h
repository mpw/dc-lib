/* ============================================================================
 *  LQuickFind.h
 * ============================================================================

 *  Author:         (C) 2005 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        September 2, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2006/03/30 09:01:19 $
 *  Changed by:     $Author: ribbi $   
 *  Revision:       $Revision: 1.3 $    
*/

#ifndef __LQuickFind__
#define __LQuickFind__

#include "LType.h"
#include "LHash.h"
#include "LArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LQuickFind_ID   0x8038

/* defines */
#define LQuickFind_BAD_ITEM 0xffffffff

/*enables stats...*/
#define LQuickFind_STATS 1

typedef struct LQuickFind LQuickFind;

struct LQuickFind 
{
    LHash*      mHashTable;
    
#if LQuickFind_STATS
    ui4 mNumUnions;
    ui4 mItemsMovedByUnions;
    ui4 mNumFinds;
#endif

};

#if LQuickFind_STATS
typedef struct LQuickFind_Stats
{
	ui4 mNumUnions;
	ui4 mItemsMovedByUnions;
	ui4 mNumFinds;	
	ui4 mMaxColListLength;
} LQuickFind_Stats;
#endif

LQuickFind*     LQuickFind_New                    ();
void            LQuickFind_Delete                 (LQuickFind** ThisA);
void            LQuickFind_MakeSet                (LQuickFind* This, ui4 inItem);
ui4             LQuickFind_Find                   (LQuickFind* This, ui4 inItem);
ui4             LQuickFind_Union                  (LQuickFind* This, ui4 inItem1, ui4 inItem2);
ui4             LQuickFind_GetUsedMem             (LQuickFind* This);

LArray*         LQuickFind_GetAllItems            (LQuickFind* This);

#if LQuickFind_STATS
void            LQuickFind_GetStats               (LQuickFind* This, LQuickFind_Stats* outStats);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* Copyright (C) 2005 Andrea Ribichini

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
