/* ============================================================================
 *  LSystem.h
 * ============================================================================

 *  Author:         (c) 2001-2005 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 28, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/

#ifndef __LSystem__
#define __LSystem__

#include "LConfig.h"
#include "LType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LSystem_ID   0x8004

enum { 
    LSystem_INTERNAL_ERROR  = LSystem_ID<<16
};

typedef void (*LSystem_THandler)(const i1*);

void        LSystem_GetString           (i1* outStr, ui4 inSize);
void        LSystem_Print               (const i1* inMsg, ...);
void        LSystem_Write               (const i1* inMsg, ui4 inSize);
void        LSystem_Exit                ();
void        LSystem_InstallPrintHandler (LSystem_THandler inHandler);
void        LSystem_OpenBlock           ();
void        LSystem_CloseBlock          (i1** outBlock, ui4* outSize);

#ifdef __cplusplus
}
#endif

#endif


/* Copyright (C) 2001-2003 Camil Demetrescu

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
