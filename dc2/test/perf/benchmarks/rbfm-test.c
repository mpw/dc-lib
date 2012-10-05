/* =====================================================================
 *  rbfm-test.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        October 13, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/04/03 19:41:32 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.8 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dc.h"
#include "rbfm.h"
#include "dijkstra.h"
#include "test-driver.h"

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

    LGraph *inGraph;
    LEdgeInfo *inEdgeInfo;
    LNodeInfo *inInitDist;
    ui4 *r_edge_weights;
    ui4 *r_dist;
    
    rbfm *rbfm_obj;
};


// ---------------------------------------------------------------------
// make_test
// ---------------------------------------------------------------------
test_t* make_test(size_t input_size, int seed, 
                 char* input_family, char* update_family, int check_correctness) {

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

    //deletes rbfm object...
    rbfm_delete (test->rbfm_obj);

    //deletes LGraph & associated info...
    LEdgeInfo_Delete (&(test->inEdgeInfo));
    LNodeInfo_Delete (&(test->inInitDist));
    LGraph_Delete (&(test->inGraph));
    
    //deallocates dynamic arrays...
    dc_free (test->r_edge_weights);
    dc_free (test->r_dist);
    
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
    return "rbfm-dc";
}


// ---------------------------------------------------------------------
// make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) 
{
    LGraph_TEdge *theThruEdge;
    ui4 theThruEdgeIndex, theThruWeight;
    
    LGraph_TNode *theThruNode;
    ui4 theThruNodeIndex, theThruDist;
    
    LException *e;
    
    //loads graph (file name stored in 'input_family' test parameter)...
    if (!LGraphUtil_LoadDimacs (test->input_family, &(test->inGraph), &(test->inEdgeInfo), TRUE))
        return "ERROR: could not load graph!\n";
    
    //LGraphUtil_RemoveSelfLoops (&test->inGraph);
    
    test->inInitDist=LNodeInfo_New (test->inGraph, LType_UI4);
    if (test->inInitDist==NULL)
        return "ERROR: could not create LNodeInfo struct!\n";
    
    //allocates reactive arrays...
    test->r_dist=(ui4 *)dc_malloc (LGraph_GetNodesCount (test->inGraph)*sizeof (unsigned int));
    if (test->r_dist==NULL)
        return "ERROR: could not allocate enough reactive mem for distances!\n";
    
    test->r_edge_weights=(ui4 *)dc_malloc (LGraph_GetEdgesCount (test->inGraph)*sizeof (unsigned int));
    if (test->r_edge_weights==NULL)
        return "ERROR: could not allocate enough reactive mem for edge weights!\n";
    
    //initializes weights...
    LGraph_ForAllEdges (test->inGraph, theThruEdge)
    {
        theThruEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
        
        //retrieves weight...
        LEdgeInfo_FetchItemAt (test->inEdgeInfo, theThruEdge, &theThruWeight);
        
        test->r_edge_weights[theThruEdgeIndex]=theThruWeight;
    }
    
    //printf ("Starting dijkstra!\n");
    Try
    {
        //runs dijkstra algo (from first node) to initialize distances...
        dijkstra(test->inGraph, LGraph_GetFirstNode (test->inGraph), test->inEdgeInfo,
            &(test->inInitDist), NULL);    
    //printf ("Finished dijkstra!\n");
    }
    Catch (e)
    {
        LException_Dump (e);
    }
               
    //copies computed distances...
    LGraph_ForAllNodes (test->inGraph, theThruNode)
    {
        theThruNodeIndex=LGraph_GetNodeIndex (theThruNode);
        
        LNodeInfo_FetchItemAt (test->inInitDist, theThruNode, &theThruDist);
        
        test->r_dist[theThruNodeIndex]=theThruDist;
    }
               
    return NULL;
}

// ---------------------------------------------------------------------
// do_from_scratch_eval
// ---------------------------------------------------------------------
char* do_from_scratch_eval(test_t* test) {

    //creates rbfm object...
    test->rbfm_obj=rbfm_new (test->inGraph, test->r_edge_weights, test->r_dist);

    return NULL;
}


// ---------------------------------------------------------------------
// do_updates
// ---------------------------------------------------------------------
char* do_updates(test_t* test) 
{
    double randomNumber;
    LGraph_TEdge *theThruEdge;
    ui4 theThruEdgeIndex;
    ui4 theWeight, theWeightNew;
    
    LGraph_TNode *theThruNode;
    ui4 theThruNodeIndex, theThruDist;
    
    LException *e;
    
    srand (test->seed);
    
    LGraph_ForAllEdges (test->inGraph, theThruEdge)
    {
        /*generates random number in (0,1)...*/
        randomNumber=rand ()/(RAND_MAX+1.0);    
        if (randomNumber > 0.01)
            continue;
        
        theThruEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
        
        theWeight=test->r_edge_weights[theThruEdgeIndex];
        theWeightNew=theWeight/2;//theWeight-((theWeight*99)/100);
        if (theWeight==theWeightNew)
            continue;
        
        test->r_edge_weights[theThruEdgeIndex]=theWeightNew;
        test->num_updates++;
        
        #if CHECK
        if (test->check)
            LEdgeInfo_AssignItemAt (test->inEdgeInfo, theThruEdge, &theWeightNew);
        #endif
        /*if (((theWeight*99)/100)>0)
        {
            test->r_edge_weights[theThruEdgeIndex]=test->r_edge_weights[theThruEdgeIndex]-((theWeight*99)/100);  
            test->num_updates++;
        }*/
    }
    
    #if CHECK
        
        //updates edgeinfo struct with new weight...
        //LEdgeInfo_AssignItemAt (test->inEdgeInfo, theThruEdge, &theWeightNew);
        
        //runs dijkstra...
    if (test->check)
    {
        Try
        {
            //runs dijkstra algo (from first node) to initialize distances...
            dijkstra(test->inGraph, LGraph_GetFirstNode (test->inGraph), test->inEdgeInfo,
                &(test->inInitDist), NULL);    
            //printf ("Finished dijkstra!\n");
        }
        Catch (e)
        {
            LException_Dump (e);
        }
        
        //compares distances...
        LGraph_ForAllNodes (test->inGraph, theThruNode)
        {
            theThruNodeIndex=LGraph_GetNodeIndex (theThruNode);
        
            LNodeInfo_FetchItemAt (test->inInitDist, theThruNode, &theThruDist);
        
            if (test->r_dist[theThruNodeIndex]!=theThruDist)
                return "[CHECK] ERROR: distances do not match!\n";
        }
    }   
        //printf ("%u\n", test->num_updates);
        
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
