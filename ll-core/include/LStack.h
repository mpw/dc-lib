/* ============================================================================
 *  LStack.h
 * ============================================================================

 *  Author:         (c) 2003 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        Nov 27, 2003
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/

#ifndef __LStack__
#define __LStack__

#include "LType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LStack_ID   0x0000

/* TYPEDEFS */
typedef struct LStack LStack;

/* EXCEPTION CODES */
enum { 
    LStack_EMPTY_STACK = LStack_ID<<16
};

/* PUBLIC FUNCTION PROTOTYPES */
LStack*      LStack_New           (LType_TType inItemType);
void         LStack_Delete        (LStack** ThisA);

Bool         LStack_IsEmpty       (LStack* This);
ui4          LStack_GetItemsCount (LStack* This);

void         LStack_Push          (LStack* This, const void* inItem);
void         LStack_Pop           (LStack* This);
void         LStack_Top           (LStack* This, void* outItem);
void         LStack_MultiPop      (LStack* This, ui4 inItemsCount);

ui4          LStack_GetUsedMem    (LStack* This);
LType_TType  LStack_GetItemType   (LStack* This);

#ifdef __cplusplus
}
#endif

#endif


/* Copyright (C) 2003 Irene Finocchi

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
