/* ============================================================================
 *  LSort.h
 * ============================================================================

 *  Author:         (C) 2005 Camil Demetrescu, Irene Finocchi, Francesco Iovine
 *  License:        See the end of this file for license information
 *  Created:        Feb 7, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/25 15:04:13 $
 *  Changed by:     $Author: irene $
 *  Revision:       $Revision: 1.2 $
*/

#ifndef __LSort__
#define __LSort__

#include "LType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LSort_ID 0x8035

/* PUBLIC FUNCTION PROTOTYPES */
void LSort_Selection (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_Insertion (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_Bubble    (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_RecMerge  (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_IterMerge (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_RecQuick  (void* inItems, ui4 inItemsCount, LType_TType inItemType);
void LSort_Shell     (void* inItems, ui4 inItemsCount, LType_TType inItemType);

#ifdef __cplusplus
}
#endif

#endif


/* Copyright (C) 2005  C. Demetrescu, I. Finocchi, F. Iovine

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
