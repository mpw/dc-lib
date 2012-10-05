/* ============================================================================
 *  LConnectivity.h
 * ============================================================================

 *  Author:         (C) 2005 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        September 30, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $   
 *  Revision:       $Revision: 1.1.1.1 $    
*/

#ifndef __LConnectivity__
#define __LConnectivity__

#include "LType.h"
#include "LGraph.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMPONENT ID */
#define LConnectivity_ID   0x8039

Bool            LConnectivity_IsConnected         (LGraph* inGraph);

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
