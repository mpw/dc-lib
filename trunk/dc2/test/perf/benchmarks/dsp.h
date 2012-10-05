/* ============================================================================
 *  dsp.h
 * ============================================================================

 *  Author:         (C) 2003 Stefano Emiliozzi
 *  License:        See the end of this file for license information
 *  Created:        March 17, 2003
 *  Module:         LL

 *  Last changed:   $Date: 2011/04/02 18:39:49 $
 *  Changed by:     $Author: demetres $   
 *  Revision:       $Revision: 1.1 $    
*/

#ifndef __dsp__
#define __dsp__

#include "LType.h"
#include "LGraph.h"
#include "LNodeInfo.h"
#include "LHeap.h"
#include "LHash.h"
#include "LEdgeInfo.h"
#include "LEdgeMap.h"

#include "dijkstra.h"

typedef struct dsp dsp;

dsp* dsp_New      (LGraph* inGraph, LGraph_TNode* inSource, LEdgeInfo* inEdgeCost);
dsp* dsp_NewShared(LGraph* inGraph, LGraph_TNode* inSource, LEdgeInfo* inEdgeCost,
                   LArray* inGraphNodes, LHeap* inHeap, LArray* inList0, LArray* inList1);

void          dsp_Delete         (dsp** ThisA);
void          dsp_UpdateEdge     (dsp* This, LGraph_TEdge* inEdgeUV, ui4 inNewWeight);
void          dsp_DeleteEdge     (dsp* This, LGraph_TEdge* inEdgeUV);
void          dsp_InsertEdge     (dsp* This, LGraph_TNode* inSrc, LGraph_TNode* inTrg, ui4 inWeight);
ui4           dsp_GetNodeDistance(dsp* This, LGraph_TNode* inNode);
LGraph_TNode* dsp_GetNodeParent  (dsp* This, LGraph_TNode* inNode);
LGraph_TNode* dsp_GetSourceNode  (dsp* This);
ui4           dsp_GetUsedMem     (dsp* This);

#endif


/* Copyright (C) 2003 Stefano Emiliozzi 

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
 * USA
*/

