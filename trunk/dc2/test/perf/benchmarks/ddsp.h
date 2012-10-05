/* ============================================================================
 *  ddsp.h
 * ============================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        April 3, 2011
 *  Module:         DC

 *  Last changed:   $Date: 2011/04/03 15:13:35 $
 *  Changed by:     $Author: demetres $   
 *  Revision:       $Revision: 1.2 $    
*/

#ifndef __ddsp__
#define __ddsp__

#include "LGraph.h"
#include "LEdgeInfo.h"

#if DDSP_STAT
typedef struct {
    ui4 num_updates;
    ui4 num_scans;
} ddssp_stat;
#endif

typedef struct ddsp ddsp;

ddsp* ddsp_new(LGraph* graph, LGraph_TNode* source, LEdgeInfo* weight);
void  ddsp_delete(ddsp* d);
ui4   ddsp_get_dist(ddsp* d, LGraph_TNode* node);
void  ddsp_decrease(ddsp* d, LGraph_TEdge* edge, ui4 new_weight);
ui4   ddsp_get_used_mem(ddsp* d);

#if DDSP_STAT
void  ddsp_get_stat(ddsp* d, ddssp_stat* stat);
#endif

#endif


/* Copyright (C) 2011 Camil Demetrescu

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

