/* =====================================================================
 *  ddsp.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        April 3, 2011
 *  Module:         DC

 *  Last changed:   $Date: 2011/04/03 15:13:35 $
 *  Changed by:     $Author: demetres $   
 *  Revision:       $Revision: 1.3 $    
*/


#include <stdio.h>
#include <stdlib.h>
#include "ddsp.h"
#include "LNodeInfo.h"
#include "LHeap.h"
#include "LException.h"

#define _INF LType_MAX_UI4


/* dect Struct */
struct ddsp {
    LHeap*     heap;    /* main queue                         */
    LNodeInfo* idx ;    /* index of node in queue             */
    LNodeInfo* dist;    /* distance from the source           */
    LEdgeInfo* weight;  /* cost of the edges of the graph     */
    LGraph*    graph;   /* input graph                        */

    #if DDSP_STAT
    ddssp_stat stat;    /* statistics */
    #endif
};


/* private function */
static void _do_loop(ddsp* d);


/* comparator for the Heap */
static Bool _comp(ui4 inA, ui4 inB) {
    return inA < inB;
}


/* ---------------------------------------------------------------------
 *  new
 * ---------------------------------------------------------------------
 *  constructor */
ddsp* ddsp_new(LGraph* graph, LGraph_TNode* source, LEdgeInfo* weight) {

    LGraph_TNode* node;

    /* make dect */
    ddsp* d = (ddsp*)malloc(sizeof(ddsp));
    if (d == NULL) return NULL;

    /* init priority queue and index map */
    d->dist   = LNodeInfo_New(graph, LType_UI4);
    d->idx    = LNodeInfo_New(graph, LType_UI4);
    d->heap   = LHeap_New(_comp);
    d->weight = weight;

    /* init node info */
    LGraph_ForAllNodes(graph, node) {
        LNodeInfo_UI4At(d->dist, node) = _INF;
        LNodeInfo_UI4At(d->idx, node)  = _INF;
    }

    /* enqueue source node */
    LNodeInfo_UI4At(d->idx, source) = 
        LHeap_Add(d->heap, (void*)source, 0);
    LNodeInfo_UI4At(d->dist, source) = 0;

    /* find initial distances */
    _do_loop(d);

    #if DDSP_STAT
    d->stat.num_scans = 0;
    d->stat.num_updates = 0;
    #endif

    return d;
}


/* ---------------------------------------------------------------------
 *  ddsp_delete
 * ---------------------------------------------------------------------
 *  destructor */
void ddsp_delete(ddsp* d) {
    LHeap_Delete(&d->heap);
    LNodeInfo_Delete(&d->dist);
    LNodeInfo_Delete(&d->idx);
    free(d);
}


/* ---------------------------------------------------------------------
 *  ddsp_get_dist
 * ---------------------------------------------------------------------
 *  return distance of node from source */
ui4 ddsp_get_dist(ddsp* d, LGraph_TNode* node) { 
    return LNodeInfo_UI4At(d->dist, node); 
}


/* ---------------------------------------------------------------------
 *  ddsp_decrease
 * ---------------------------------------------------------------------
 *  decrease weight of edge by delta */
void ddsp_decrease(ddsp* d, LGraph_TEdge* edge, ui4 new_weight) {

    LGraph_TNode* node;
    i4 delta;
    
    delta = new_weight - LEdgeInfo_UI4At(d->weight, edge);

    if (delta > 0) {
        printf("new distance is larger than old distance\n");
        exit(1);
    }

    /* update edge weight */
    LEdgeInfo_UI4At(d->weight, edge) = new_weight;


    /* enqueue start node */
    node = LGraph_GetSource(edge);
    LNodeInfo_UI4At(d->idx, node) = 
        LHeap_Add(d->heap, (void*)node, 
            LNodeInfo_UI4At(d->dist, node));

    /* propagate changes */
    _do_loop(d);

    #if DDSP_STAT
    d->stat.num_updates++;
    #endif
}


/* ---------------------------------------------------------------------
 *  get_used_mem
 * ---------------------------------------------------------------------
 *  return total memory usage in bytes of the object */
ui4 ddsp_get_used_mem(ddsp* d) {
    ui4 mem = 0; 
    mem += sizeof(ddsp);
    mem += LNodeInfo_GetUsedMem(d->dist);
    mem += LNodeInfo_GetUsedMem(d->idx);
    mem += LHeap_GetUsedMem(d->heap);
    return mem;
}



#if DDSP_STAT
/* ---------------------------------------------------------------------
 *  get_stat
 * ---------------------------------------------------------------------
 *  get statistics */
void ddsp_get_stat(ddsp* d, ddssp_stat* stat) {
    *stat = d->stat;
}
#endif


/* ---------------------------------------------------------------------
 *  _do_loop
 * ---------------------------------------------------------------------
 *  perform propagation loop */
void _do_loop(ddsp* d) {

    /* main loop */
    while (!LHeap_Empty(d->heap)) {

        LGraph_TNode* node;
        LGraph_TEdge* edge;
        ui4 dist;

        #if DDSP_STAT
        d->stat.num_scans++;
        #endif

        /* extract node with minimum distance from inSource */
        LHeap_ExtractMin(d->heap, (void**)&node, &dist);
        LNodeInfo_UI4At(d->idx, node) = _INF;

        /* scan adjacents */
        LGraph_ForAllOut(node, edge) {

            LGraph_TNode* adj = LGraph_GetTarget(edge);
            ui4 adj_dist = dist + LEdgeInfo_UI4At(d->weight, edge);

            /* relaxation */
            if (adj_dist < LNodeInfo_UI4At(d->dist, adj)) {

                /* insert or update node key in priority queue */
                if (LNodeInfo_UI4At(d->idx, adj) == _INF) 
                     LNodeInfo_UI4At(d->idx, adj) = 
                         LHeap_Add(d->heap, (void*)adj, adj_dist);
                else 
                     LHeap_Update(d->heap, (void*)adj, adj_dist, 
                                  LNodeInfo_UI4At(d->idx, adj));

                /* update distance */
                LNodeInfo_UI4At(d->dist, adj) = adj_dist;
            }
        }
    }
}


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

