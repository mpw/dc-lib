/* ============================================================================
 *  dijkstra.h
 * ============================================================================

 *  Author:         (c) 2003 Camil Demetrescu, Stefano Emiliozzi
 *  License:        See the end of this file for license information
 *  Created:        January 9, 2003
 *  Module:         LL

 *  Last changed:   $Date: 2012/09/25 14:36:27 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/

#include "LGraph.h"
#include "LNodeInfo.h"
#include "LEdgeInfo.h"
#include "LHeap.h"


void dijkstra(LGraph* inGraph, LGraph_TNode* inSource, LEdgeInfo* inWeightArray,
               LNodeInfo** outDistArray, LNodeInfo** outPredArray);


/* Copyright (C) 2003 Camil Demetrescu, Stefano Emiliozzi

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
