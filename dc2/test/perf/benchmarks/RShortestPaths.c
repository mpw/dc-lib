/* =====================================================================
 *  RShortestPaths.c
 * =====================================================================

 *  Author:         (c) 2009 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        November 17, 2009
 *  Module:         dc/test

 *  Last changed:   $Date: 2012/09/25 15:04:35 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.5 $
*/

#include <stdio.h>
#include <stdlib.h>
 
#include <sys/time.h>
 
#include "dc.h"

#include "LGraphUtil.h"

//#define NUM_EDGES 3897636.0/2.0

//globals...
unsigned long g_NumAtt=0;

//protected mem stuff...
unsigned int theNumPagesForDistances, theNumPagesForWeights;
volatile unsigned int *theDistances, *theWeights;

//mirror distances...
unsigned int *theUnprotDistances;

//constraint prototype...
void theConstraint (void *inParam);

//comparator prototype...
char theComparator (unsigned long inParam1, unsigned long inParam2);

//decrease prototype...
void decreaseEdgeWeight (unsigned long inEdgeIndex, unsigned int inDelta);

int main (int argc, char **argv)
{	
	//aux stuff...
	int i;
	double randomNumber;
	unsigned long theNumEdgeChanged=0;
	unsigned long theProtReads, theProtWrites;
	unsigned long theWeight, theWeightNew;
	
	//time stuff...
	struct timeval tvBegin, tvEnd;
	double decrease_time;
	
	//LGraph stuff...
	LGraph *theGraph;
	LEdgeInfo *theEdgeInfo;
	ui4 n, m;
	
	LGraph_TNode *theFirstNode, *theThruNode;
	ui4 theFirstNodeIndex, theThruNodeIndex;
	
	LGraph_TEdge *theFirstEdge, *theThruEdge;
	ui4 theFirstEdgeIndex, theThruEdgeIndex, theThruWeight;
	
	/*generates random seed...*/
	srand((unsigned)time (NULL));
	
	//command line help...
	if (argc!=2)
	{
		printf ("Usage: RShortestPaths dimacsGraphFileName\n");
		return 0;
	}
	
	//generates directed LGraph from dimacs file...
	if (!LGraphUtil_LoadDimacs (argv[1], &theGraph, &theEdgeInfo, TRUE))
	{
		printf ("ERROR: could not load graph!\n");
		return 0;
	}
	else
		printf ("Successfully loaded file %s\n", argv[1]);
	
    //LGraphUtil_RemoveSelfLoops (&theGraph);
    
	n=LGraph_GetNodesCount (theGraph);
	m=LGraph_GetEdgesCount (theGraph);
	printf ("n = %u   m = %u\n", n, m);
	
	//printf ("LEdgeInfo_BaseType: %d\n", LEdgeInfo_GetBaseType (theEdgeInfo));
	
	dc_init ();
    
    theDistances=(unsigned int *)dc_malloc (n*sizeof (unsigned int));
	if (theDistances==NULL)
	{
		//nice to know...
		printf ("ERROR: could not allocate enough protected mem for distances!\n");
		return 0;
	}
	
	//allocates mem for unprot distances...
	theUnprotDistances=(unsigned int *)malloc (n*sizeof (unsigned int));
	
    theWeights=(unsigned int *)dc_malloc (m*sizeof (unsigned int));
	if (theWeights==NULL)
	{
		//nice to know...
		printf ("ERROR: could not allocate enough protected mem for edge weights!\n");
		return 0;
	}
	
	//initializes distances...
	
	//gets first node (and its index)...
	theFirstNode=LGraph_GetFirstNode (theGraph);
	theFirstNodeIndex=LGraph_GetNodeIndex (theFirstNode);
	printf ("FirstNode index is %u\n", theFirstNodeIndex);
    
    //theFirstNodeIndex=1;
    
    int debug_num_nodes=0;
    
	LGraph_ForAllNodes (theGraph, theThruNode)
	{
        debug_num_nodes++;
        
		theThruNodeIndex=LGraph_GetNodeIndex (theThruNode);
		
		if (theThruNodeIndex==theFirstNodeIndex)
		{
			theDistances[theThruNodeIndex]=0; //0 if source....
			theUnprotDistances[theThruNodeIndex]=0;
            printf ("distance initialized to 0\n");
		}
		else
		{
			theDistances[theThruNodeIndex]=0xffffffff; //-1 if any other node...
			theUnprotDistances[theThruNodeIndex]=0xffffffff;
		}
	}
	
	printf ("Number of initialized distances: %d\n", debug_num_nodes);
	
	//inizializes weights...
	
	//gets first edge (and its index)...
	theFirstEdge=LGraph_GetFirstEdge (theGraph);
	theFirstEdgeIndex=LGraph_GetEdgeIndex (theFirstEdge);
	//printf ("FirstEdge index is %u\n", theFirstEdgeIndex);
	
	LGraph_ForAllEdges (theGraph, theThruEdge)
	{
		theThruEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
		
		//retrieves weight...
		LEdgeInfo_FetchItemAt (theEdgeInfo, theThruEdge, &theThruWeight);
		//printf ("Weight: %u\n", theThruWeight);
		
        /*if (theThruWeight==0)
        {
            printf ("Setting a weight to 0\n");
            exit (1);
        }*/
        
		theWeights[theThruEdgeIndex]=theThruWeight;
	}
	
#if 1
    //prints distances...
    printf ("\n");
    for (i=0; i<n; i++)
        printf ("%d ", theDistances[i]);
    printf ("\n");
#endif	
	
#if 0
    printf ("\n");
    for (i=1; i<n; i++)
        if (theDistances[i]==0)
        {
            printf ("Zero distances beginning at index: %d\n", i);
            fflush (stdout);
            break;
        }
    printf ("\n");
#endif
	
	//installs constraints...
	printf ("Installing constraints...");
	i=0;
	LGraph_ForAllNodes (theGraph, theThruNode)
	{
        dc_new_cons (theConstraint, (void *)theThruNode, NULL);
		
		i++;
		if (i%1000==0)
		{
			printf ("%d ", i);
			fflush (stdout);
		}
	}
	printf ("\n");
	
	//now distances are correct...
	printf ("Constraints installed!\n");
	fflush (stdout);

#if 1
    //prints distances...
    printf ("\n");
    for (i=0; i<n; i++)
        printf ("%d ", theDistances[i]);
    printf ("\n");
#endif
    
#if 0
    printf ("\n");
    for (i=1; i<n; i++)
        if (theDistances[i]==0)
        {
            printf ("Zero distances beginning at index: %d\n", i);
            fflush (stdout);
            break;
        }
    printf ("\n");
#endif
	
	/*fetches initial time...*/
	gettimeofday (&tvBegin, NULL);
	
	//testing decrease...
	g_NumAtt=0;
	LGraph_ForAllEdges (theGraph, theThruEdge)
	{
		/*generates random number in (0,1)...*/
		randomNumber=rand ()/(RAND_MAX+1.0);	
		if (randomNumber > 0.1/*(NUM_EDGES/m)*/)
			continue;
		
		theThruEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
		
		theWeight=theWeights[theThruEdgeIndex];
		theWeightNew=theWeight-((theWeight*99)/100);
		if (theWeight==theWeightNew)
			continue;
		
		theNumEdgeChanged++;
		if (theNumEdgeChanged%1000==0)
		{
			printf ("%u ", theNumEdgeChanged);
			fflush (stdout);
		}
		
		decreaseEdgeWeight (theThruEdgeIndex, (theWeight*99)/100);
		//printf ("Decreased edge #%u\n", theThruEdgeIndex);
		//fflush (stdout);
	}
	printf ("\n");
#if 0
	LGraph_ForAllEdges (theGraph, theThruEdge)
	{
		/*generates random number in (0,1)...*/
		randomNumber=rand ()/(RAND_MAX+1.0);	
		if (randomNumber > 0.5/*(NUM_EDGES/m)*/)
			continue;
		
		theThruEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
		
		theWeight=theWeights[theThruEdgeIndex];
		theWeightNew=theWeight-((theWeight*99)/100);
		if (theWeight==theWeightNew)
			continue;
		
		theNumEdgeChanged++;
		if (theNumEdgeChanged%10000==0)
		{
			printf ("%u ", theNumEdgeChanged);
			fflush (stdout);
		}
		
		decreaseEdgeWeight (theThruEdgeIndex, (theWeight*99)/100);
		//printf ("Decreased edge #%u\n", theThruEdgeIndex);
		//fflush (stdout);
	}
	printf ("\n");
#endif
	
	/*fetches final time...*/
	gettimeofday (&tvEnd, NULL);
	decrease_time=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
			-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);	
	
	printf ("Total decrease time: %f secs\n", decrease_time);
	printf ("Total # of cons activations: %u\n", g_NumAtt);
	printf ("Total # of edge weight decrease ops: %u\n", theNumEdgeChanged);
	printf ("Avg cons activations per decrease op: %f\n", (double)g_NumAtt/theNumEdgeChanged);
	printf ("Avg decrease op time: %f secs\n", decrease_time/theNumEdgeChanged);
	
#if 0
	//prints distances...
	printf ("\n");
	for (i=0; i<n; i++)
		printf ("%d ", theDistances[i]);
	printf ("\n");
#endif
					
	//cleans up...
		
    dc_free (theDistances);
    dc_free (theWeights);
	
	free (theUnprotDistances);
	
	LEdgeInfo_Delete (&theEdgeInfo);
	LGraph_Delete (&theGraph);
	
	return 0;
}

void theConstraint (void *inParam)
{
    //printf ("Cons here!\n");
    
	g_NumAtt++;
	
	LGraph_TNode *theSourceNode=(LGraph_TNode *)inParam;
	ui4 theSourceNodeIndex=LGraph_GetNodeIndex (theSourceNode);
	
	LGraph_TEdge *theThruEdge;
	ui4 theEdgeIndex;
	ui4 theEdgeWeight;
	
	LGraph_TNode *theTargetNode;
	ui4 theTargetNodeIndex;
	
	ui4 theDistance=theDistances[theSourceNodeIndex];
	
	//if node is disconnected from source...
	if (theDistance==0xffffffff)
		return; //returns...
		
	LGraph_ForAllOut (theSourceNode, theThruEdge)
	{
        //printf ("Out edge!\n");
		theEdgeIndex=LGraph_GetEdgeIndex (theThruEdge);
		theEdgeWeight=theWeights[theEdgeIndex];
		
		theTargetNode=LGraph_GetTarget (theThruEdge);
		theTargetNodeIndex=LGraph_GetNodeIndex (theTargetNode);
		
		if ((theDistance+theEdgeWeight)<theDistances[theTargetNodeIndex])
		{
            /*if (theDistance+theEdgeWeight==0)
            {
                printf ("\nSetting a distance to 0!\n");
                exit (1);
            }*/
            
			theDistances[theTargetNodeIndex]=theDistance+theEdgeWeight;
			theUnprotDistances[theTargetNodeIndex]=theDistance+theEdgeWeight;
		}
	}
}

void decreaseEdgeWeight (unsigned long inEdgeIndex, unsigned int inDelta)
{
	if (inDelta<=0)
		return;
	
	theWeights[inEdgeIndex]=theWeights[inEdgeIndex]-inDelta;
}

char theComparator (unsigned long inParam1, unsigned long inParam2)
{
	//printf ("[COMPARATOR] Param1 = %p    Param2 = %p\n", inParam1, inParam2);
	//fflush (stdout);
	
	LGraph_TNode *theNode1=(LGraph_TNode *)inParam1;
	LGraph_TNode *theNode2=(LGraph_TNode *)inParam2;
	
	ui4 theNode1Index=LGraph_GetNodeIndex (theNode1);
	ui4 theNode2Index=LGraph_GetNodeIndex (theNode2);
	
	//return theDistances[theNode1Index]-theDistances[theNode2Index];
	return theUnprotDistances[theNode1Index]<theUnprotDistances[theNode2Index];
}

/*
# Copyright (C) 2009 Camil Demetrescu, Andrea Ribichini

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
*/
