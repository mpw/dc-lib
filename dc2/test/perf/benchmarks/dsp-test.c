/* =====================================================================
 *  dsp-test.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        October 13, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/04/03 11:01:19 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.5 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test-driver.h"
#include "dsp.h"

#include "LGraphUtil.h"
#include "LException.h"


// specific test object
struct test_s {
    size_t      input_size;
    int         seed;
    size_t      num_updates;
    int         check;
    char*       input_family;
    char*       update_family;
    
    #if CONV || CHECK
    
    #endif

    LGraph *graph;
    LEdgeInfo *edge_weights;
    dsp *d;
};


// ---------------------------------------------------------------------
// make_test
// ---------------------------------------------------------------------
test_t* make_test(size_t input_size, int seed, 
                 char* input_family, char* update_family, 
                 int check_correctness) {

    test_t* test = (test_t*)malloc(sizeof(test_t));
    if (test == NULL) return NULL;

    test->input_size = input_size;
    test->seed = seed;
    test->num_updates = 0;
    test->input_family = input_family;
    test->update_family = update_family;
    test->check = check_correctness;

    #if CONV || CHECK
    
    #endif

    return test;
}


// ---------------------------------------------------------------------
// del_test
// ---------------------------------------------------------------------
void del_test(test_t* test) {

    #if CONV || CHECK
    
    #endif

    //deletes dsp object...    
    dsp_Delete(&(test->d));

    //deletes LGraph & associated info...
    LEdgeInfo_Delete (&(test->edge_weights));
    LGraph_Delete (&(test->graph));

    free(test);
}

// ---------------------------------------------------------------------
// get_num_updates
// ---------------------------------------------------------------------
size_t get_num_updates(test_t* test) {
    return test->num_updates;
}


// ---------------------------------------------------------------------
// get_test_name
// ---------------------------------------------------------------------
char* get_test_name() {
    return "dsp";
}


// ---------------------------------------------------------------------
// make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) 
{
    
    // loads graph (file name stored in 'input_family' test parameter)...
    if (!LGraphUtil_LoadDimacs(test->input_family, 
            &(test->graph), &(test->edge_weights), TRUE))
        return "ERROR: could not load graph!\n";
    
    return NULL;
}

// ---------------------------------------------------------------------
// do_from_scratch_eval
// ---------------------------------------------------------------------
char* do_from_scratch_eval(test_t* test) {

    // creates dsp object...
    test->d = dsp_New(test->graph, 
                      LGraph_GetFirstNode (test->graph), 
                      test->edge_weights);

    return NULL;
}


// ---------------------------------------------------------------------
// do_updates
// ---------------------------------------------------------------------
char* do_updates(test_t* test)  {

    LGraph_TEdge *edge;

    srand(test->seed);

    LGraph_ForAllEdges (test->graph, edge) {

        /*generates random number in (0,1)...*/
        double rand_num = rand()/(RAND_MAX+1.0);    
        ui4 weight, new_weight;

        if (rand_num > 0.01) continue;

        weight = LEdgeInfo_UI4At(test->edge_weights, edge);
        new_weight = weight / 2;

        if (new_weight < weight) {
            dsp_UpdateEdge(test->d, edge, new_weight);
            test->num_updates++;
        }
    }

    #if CHECK
    if (test->check) {

        LNodeInfo *dist = NULL;
        LGraph_TNode *node;
        LException* e;

        Try {

            // make node info
            dist = LNodeInfo_New(test->graph, LType_UI4);
            if (dist == NULL)
                return "ERROR: could not create LNodeInfo struct!\n";

            // runs dijkstra algo (from first node) to get distances...
            dijkstra(test->graph, 
                LGraph_GetFirstNode(test->graph), test->edge_weights,
                &dist, NULL);

        }
        Catch (e) { 
            LException_Dump(e);
        }

        // compares distances...
        LGraph_ForAllNodes (test->graph, node) {

            ui4 ok_dist  = LNodeInfo_UI4At(dist, node);
            ui4 dsp_dist = dsp_GetNodeDistance(test->d, node);

            if (dsp_dist != ok_dist)
                return "[CHECK] ERROR: distances do not match!\n";
        }

        if (dist != NULL) LNodeInfo_Delete(&dist);
    }   

    #endif

    return NULL;
}


#if CONV || CHECK

// ---------------------------------------------------------------------
// make_conv_input
// ---------------------------------------------------------------------
char* make_conv_input(test_t* test) 
{
    return NULL;
}


// ---------------------------------------------------------------------
// do_conv_eval
// ---------------------------------------------------------------------
char* do_conv_eval(test_t* test) 
{
    return NULL;
}

#endif


/* Copyright (C) 2011 Camil Demetrescu, Andrea Ribichini

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
