/* ============================================================================
 *  LHashStatic.h
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        April 12, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2006/04/19 08:35:39 $
 *  Changed by:     $Author: ribbi $   
 *  Revision:       $Revision: 1.2 $    
*/

#ifndef __LHashStatic__
#define __LHashStatic__

#include "LType.h"
#include "LArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LHashStatic_ID   ????

/*row of hash table...*/
struct TSlot
{
	ui4     mKey;
	void*   mItemRef;
};

typedef struct LHashStatic LHashStatic;

struct LHashStatic 
{
	ui4 mNumEntries;
	struct TSlot* mHashTable;
};

LHashStatic*	LHashStatic_New			(ui4 inMinNumEntries);
void       	LHashStatic_Delete		(LHashStatic** ThisA);
void       	LHashStatic_InsertItem        	(LHashStatic* This, void* inItem, ui4 inKey);
Bool       	LHashStatic_IsInTable         	(LHashStatic* This, ui4 inKey);
void*      	LHashStatic_GetItemByKey      	(LHashStatic* This, ui4 inKey);

ui4        	LHashStatic_GetUsedMem		(LHashStatic* This);
ui4		LHashStatic_GetNumEntries	(LHashStatic* This);

LArray*    	LHashStatic_GetAllItems         (LHashStatic* This);
LArray*    	LHashStatic_GetAllKeys          (LHashStatic* This);

#ifdef __cplusplus
}
#endif

#endif

/* Copyright (C) 2006 Andrea Ribichini

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
