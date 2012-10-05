/* ============================================================================
 *  LConnectivity.c
 * ============================================================================

 *  Author:         (C) 2005 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        September 30, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $  
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/

#include "LConnectivity.h"
#include "LGraphUtil.h"
#include <stdio.h>

/* PROTOTYPES */
LGraph* _visitaDSF (LGraph* inGraph, LGraph_TNode* inStartNode);
void _visitaDSFRicorsiva (LGraph* inGraph, LNodeInfo* inNodeInfo, LNodeInfo* inNodeInfo2, LGraph* inTree, LGraph_TNode* inNode);

/* PUBLIC METHODS */

/* ---------------------------------------------------------------------------------
*  LConnectivity_IsConnected
*  ---------------------------------------------------------------------------------
*  IsConnected */

Bool LConnectivity_IsConnected (LGraph* inGraph) 
{
	LGraph_TNode* theNode;
	LGraph* theTree;
	
	ui4 theNumGraphNodes, theNumTreeNodes;
	
	theNode=LGraph_GetFirstNode (inGraph);
	theTree=_visitaDSF (inGraph, theNode);
	
	theNumGraphNodes=LGraph_GetNodesCount (inGraph);
	theNumTreeNodes=LGraph_GetNodesCount (theTree);
	
	LGraph_Delete (&theTree);
	
	return ( theNumGraphNodes == theNumTreeNodes );
}

/* PRIVATE METHODS */

LGraph* _visitaDSF (LGraph* inGraph, LGraph_TNode* inStartNode)
{
	LGraph* theTree;
	LNodeInfo* theNodeInfo;
	LNodeInfo* theNodeInfo2;
	LGraph_TNode* theNode;
	
	i1 theDummyFalse=0;
	void* theDummyNULL=NULL;
	
	/*creates new undirected graph*/
	theTree=LGraph_New (FALSE);
	
	/*creates LNodeInfo data struct*/
	theNodeInfo=LNodeInfo_NewCustom (inGraph, 1);
	/*sets all nodes to 'unmarked' status*/
	theNode=LGraph_GetFirstNode (inGraph);
	while (theNode!=NULL)
	{
		LNodeInfo_AssignItemAt (theNodeInfo, theNode, &theDummyFalse);	
		theNode=LGraph_GetNextNode (theNode);
	}
	
	/*creates 2nd LNodeInfo data struct*/
	theNodeInfo2=LNodeInfo_NewCustom (inGraph, 4);
	/*sets all node references to NULL*/
	theNode=LGraph_GetFirstNode (inGraph);
	while (theNode!=NULL)
	{
		LNodeInfo_AssignItemAt (theNodeInfo2, theNode, &theDummyNULL);	
		theNode=LGraph_GetNextNode (theNode);
	}
	
	printf ("About to perform recursive walk...\n");
	/*performs recursive walk*/
	_visitaDSFRicorsiva (inGraph, theNodeInfo, theNodeInfo2, theTree, inStartNode);
	
	/*deletes LNodeInfo data structs*/
	LNodeInfo_Delete (&theNodeInfo);
	LNodeInfo_Delete (&theNodeInfo2);
	
	return theTree;
}

void _visitaDSFRicorsiva (LGraph* inGraph, LNodeInfo* inNodeInfo, LNodeInfo* inNodeInfo2, LGraph* inTree, LGraph_TNode* inNode)
{
	i1 theDummyTrue=1;
	LArray* theAdjNodes;
	ui4 theNumAdjNodes, i;
	LGraph_TNode* theAuxNode;
	i1 theFlag;
	
	LGraph_TNode* theRefinNode;
	LGraph_TNode* theRefAuxNode;
	LGraph_TNode* theNode0;
	LGraph_TNode* theNode1;
	
	/*marks inNode*/
	LNodeInfo_AssignItemAt (inNodeInfo, inNode, &theDummyTrue);
	printf ("Node marked!\n");
	
	/*gets adjacent nodes*/
	theAdjNodes=LGraph_GetAdjNodes (inGraph, inNode);
	theNumAdjNodes=LArray_GetItemsCount (theAdjNodes);
	printf ("# of adjacent nodes: %d\n", theNumAdjNodes);
	/*for all adj nodes...*/
	for (i=0; i<theNumAdjNodes; i++)
	{
		theAuxNode=*(LGraph_TNode **)LArray_ItemAt (theAdjNodes, i);
		/*if node not marked*/
		LNodeInfo_FetchItemAt (inNodeInfo, theAuxNode, &theFlag);
		if (theFlag==0)
		{
			printf ("Adjacent node NOT marked!\n");
			/*adds arc to tree*/
			LNodeInfo_FetchItemAt (inNodeInfo2, inNode, &theRefinNode);
			LNodeInfo_FetchItemAt (inNodeInfo2, theAuxNode, &theRefAuxNode);
			if (theRefinNode==NULL)
			{
				theNode0=LGraph_NewNode (inTree);
				LNodeInfo_AssignItemAt (inNodeInfo2, inNode, &theNode0);
			}
			else
				theNode0=theRefinNode;
				
			if (theRefAuxNode==NULL)
			{
				theNode1=LGraph_NewNode (inTree);
				LNodeInfo_AssignItemAt (inNodeInfo2, theAuxNode, &theNode1);
			}
			else
				theNode1=theRefAuxNode;
				
			LGraph_NewEdge (inTree, theNode0, theNode1);
			printf ("New edge added to tree!\n");
			/*recursive call*/
			_visitaDSFRicorsiva (inGraph, inNodeInfo, inNodeInfo2, inTree, theAuxNode);
		}
	}
	
	LArray_Delete (&theAdjNodes);
	
}

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
