/* ============================================================================
 *  LQuickFindStatic.h
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        April 7, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2006/04/10 23:04:46 $
 *  Changed by:     $Author: ribbi $   
 *  Revision:       $Revision: 1.1 $    
*/

#ifndef __LQuickFindStatic__
#define __LQuickFindStatic__

#include "LType.h"
#include "LArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LQuickFindStatic_ID   ????

/* defines */
#define LQuickFindStatic_BAD_ITEM 0xffffffff

typedef struct _tagLQuickFindStatic_Item
{
	ui4	mParent;
	ui4	mFirstChild;
	union
	{
		ui4	mNextSibling;
		ui4	mNumChildren;
	};
	
} LQuickFindStatic_Item;

typedef struct LQuickFindStatic LQuickFindStatic;

struct LQuickFindStatic 
{
    ui4       			mRange;
    LQuickFindStatic_Item*      mMasterArray;
};

LQuickFindStatic*    LQuickFindStatic_New                    (ui4 inRange);
void                 LQuickFindStatic_Delete                 (LQuickFindStatic** ThisA);
void                 LQuickFindStatic_MakeSet                (LQuickFindStatic* This, ui4 inItem);
ui4                  LQuickFindStatic_Find                   (LQuickFindStatic* This, ui4 inItem);
ui4                  LQuickFindStatic_Union                  (LQuickFindStatic* This, ui4 inItem1, ui4 inItem2);
ui4                  LQuickFindStatic_GetUsedMem             (LQuickFindStatic* This);

LArray*              LQuickFindStatic_GetAllItems            (LQuickFindStatic* This);

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
