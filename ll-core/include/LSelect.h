/* ============================================================================
 *  LSelect.h
 * ============================================================================

 *  Author:         (c) 2005 Irene Finocchi, Francesco Iovine
 *  License:        See the end of this file for license information
 *  Created:        Feb 7, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/25 15:04:13 $
 *  Changed by:     $Author: irene $
 *  Revision:       $Revision: 1.1 $
*/

#ifndef __LSelect__
#define __LSelect__

#include "LType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LSelect_ID 0x803A

/* EXCEPTION CODES */
enum { 
    LSelect_BAD_SELECT = LSelect_ID<<16
};

/* PUBLIC FUNCTION PROTOTYPES */
void LSelect_Rand   (void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inSelect);
void LSelect_Determ (void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inSelect);

#ifdef __cplusplus
}
#endif

#endif


/* Copyright (C) 2005 Irene Finocchi and Francesco Iovine

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
