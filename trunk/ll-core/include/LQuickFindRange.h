/* ============================================================================
 *  LQuickFindRange.h
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        March 25, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2006/03/26 18:11:02 $
 *  Changed by:     $Author: ribbi $   
 *  Revision:       $Revision: 1.1 $    
*/

#ifndef __LQuickFindRange__
#define __LQuickFindRange__

#include "LType.h"
#include "LArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LQuickFindRange_ID   ????

/* defines */
#define LQuickFindRange_BAD_ITEM 0xffffffff

typedef struct LQuickFindRange LQuickFindRange;

struct LQuickFindRange 
{
    ui4       mRange;
    ui4*      mMasterArray;
};

LQuickFindRange*     LQuickFindRange_New                    (ui4 inRange);
void                 LQuickFindRange_Delete                 (LQuickFindRange** ThisA);
void                 LQuickFindRange_MakeSet                (LQuickFindRange* This, ui4 inItem);
ui4                  LQuickFindRange_Find                   (LQuickFindRange* This, ui4 inItem);
ui4                  LQuickFindRange_Union                  (LQuickFindRange* This, ui4 inItem1, ui4 inItem2);
ui4                  LQuickFindRange_GetUsedMem             (LQuickFindRange* This);

LArray*              LQuickFindRange_GetAllItems            (LQuickFindRange* This);

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
