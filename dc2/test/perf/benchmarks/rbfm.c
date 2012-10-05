/* ============================================================================
 *  rbfm.c
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        February 6, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/04/04 21:28:52 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.7 $
*/

#include <stdio.h>
#include <stdlib.h>

#include "dc.h"

#include "rbfm.h"
#include "heap_sched.h"
#include "dc_sched_heap.h"
#include "dc_sched_stack.h"

//cons param...
typedef struct rbfm_cons_param_s
{
    dc_cons* cons;
    rbfm *obj;
    LGraph_TNode *node;
} rbfm_cons_param;

//object structure...
struct rbfm_s
{
    LGraph *g;
    ui4 *edge_weights;
    ui4 *dist;
    rbfm_cons_param **param_array;
    dc_group *group;
};

// ---------------------------------------------------------------------
// rbfm_cons
// ---------------------------------------------------------------------
void rbfm_cons (rbfm_cons_param* inParam)
{    
    LGraph_TNode *theSourceNode=inParam->node;
    ui4 theSourceNodeIndex=LGraph_GetNodeIndex (theSourceNode);
    
    LGraph_TEdge *theThruEdge;
    ui4 theEdgeIndex;
    ui4 theEdgeWeight;
    
    LGraph_TNode *theTargetNode;
    ui4 theTargetNodeIndex;
    
    ui4 *theDistances=(inParam->obj)->dist;
    ui4 *theWeights=(inParam->obj)->edge_weights;
    
    ui4 theDistance=theDistances[theSourceNodeIndex];
    
    //if node is disconnected from source...
    if (theDistance==0xffffffff)
        return; //returns...
        
    LGraph_ForAllOut (theSourceNode, theThruEdge)
    {
        theEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
        theEdgeWeight=theWeights[theEdgeIndex];
        
        theTargetNode=LGraph_GetTarget (theThruEdge);
        theTargetNodeIndex=LGraph_GetNodeIndex (theTargetNode);
        
        if ((theDistance+theEdgeWeight) < dc_inactive (theDistances[theTargetNodeIndex]))
        {
            theDistances[theTargetNodeIndex]=theDistance+theEdgeWeight;
        }
    }
}

//comparator...
int my_comp (dc_cons *cons_1, dc_cons *cons_2)
{
    rbfm_cons_param *param_1, *param_2;
    ui4 index_1, index_2;
    rbfm *obj;
    
    //params...
    param_1=dc_get_cons_param (cons_1);
    param_2=dc_get_cons_param (cons_2);
    
    //test object...
    obj=param_1->obj;
    
    //indices...
    index_1=LGraph_GetNodeIndex (param_1->node);
    index_2=LGraph_GetNodeIndex (param_2->node);
    
    //comparison...
    return dc_inactive(obj->dist[index_1]) - dc_inactive(obj->dist[index_2]);
//    return index_1-index_2;//dc_inactive (obj->dist[index_1]) - dc_inactive (obj->dist[index_2]);
}


// ---------------------------------------------------------------------
// rbfm_new
// ---------------------------------------------------------------------
rbfm* rbfm_new (LGraph* g, ui4* edge_weights, ui4* dist)
{
    LGraph_TNode *theFirstNode, *theThruNode;
    ui4 theFirstNodeIndex, theThruNodeIndex;
    int i;
    
    //creates object...
    rbfm *obj=malloc (sizeof (rbfm));
    if (obj==NULL)
        return NULL;
    
    //initializes object...
    obj->g=g;
    obj->edge_weights=edge_weights;
    obj->dist=dist;
    obj->group=dc_new_group ();
    #if DEFAULT_SCHED
        dc_set_scheduler_type (obj->group, /*dc_sched_stack_g*/dc_sched_heap_g); 
    #else
        dc_set_scheduler_type (obj->group, heap_sched_g);
        heap_set_comp (dc_get_scheduler (obj->group), my_comp);
    #endif
    
    //gets first node (and its index)...
    theFirstNode=LGraph_GetFirstNode (obj->g);
    theFirstNodeIndex=LGraph_GetNodeIndex (theFirstNode);
    
    //initializes distances...
    //REMOVED: initialize with dijkstra in test driver...
    /*LGraph_ForAllNodes (obj->g, theThruNode)
    {
        theThruNodeIndex=LGraph_GetNodeIndex (theThruNode);
        
        if (theThruNodeIndex==theFirstNodeIndex)
        {
            obj->dist[theThruNodeIndex]=0; //0 if source....
        }
        else
        {
            obj->dist[theThruNodeIndex]=0xffffffff; //-1 if any other node...
        }
    }*/
    
    //creates param array...
    obj->param_array=malloc (LGraph_GetNodesCount (obj->g)*sizeof (rbfm_cons_param *));
    
    //installs constraints...
    i=0;
    LGraph_ForAllNodes (obj->g, theThruNode)
    {
        //creates cons_param struct...
        rbfm_cons_param *theParam=malloc (sizeof (rbfm_cons_param));
        theParam->obj=obj;
        theParam->node=theThruNode;        
        theParam->cons=dc_new_cons ((dc_cons_f)rbfm_cons, theParam, obj->group);
        
        obj->param_array[i]=theParam;
        i++;
    }    
    
    return obj;
}

// ---------------------------------------------------------------------
// rbfm_delete
// ---------------------------------------------------------------------
void rbfm_delete (rbfm* obj)
{
    int i;
    int n=LGraph_GetNodesCount (obj->g);
    
    //deletes constraints and params...
    for (i=0; i<n; i++)
    {
        rbfm_cons_param *theParam=obj->param_array[i];
        
        dc_del_cons (theParam->cons);
        
        free (theParam);
    }
    
    //deletes param_array...
    free (obj->param_array);
    
    //deletes object...
    free (obj);
}
