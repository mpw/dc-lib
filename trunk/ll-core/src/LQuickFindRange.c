/* ============================================================================
 *  LQuickFindRange.c
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        March 25, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2006/03/28 17:04:11 $  
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.4 $
*/

#include "LQuickFindRange.h"
#include "LArray.h"
#include "LMemory.h"
#include "LHash.h"

/* PUBLIC METHODS */

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_New
*  ---------------------------------------------------------------------------------
*  Constructor */

LQuickFindRange* LQuickFindRange_New (ui4 inRange) 
{
	LQuickFindRange theObject={0};

	/*parameter checking...*/
	if (inRange==0)
		return NULL;
	
	/*saves range information...*/
	/*items will be in 0..range-1 */
	theObject.mRange=inRange;
	
	/*allocates memory for master array...*/
	theObject.mMasterArray=(ui4 *)LMemory_Malloc (4*inRange);
	
	/*sets table to empty (all entries equal to NULL)...*/
	LMemory_Clear (theObject.mMasterArray, 4*inRange);
    
	return LMemory_NewObject (LQuickFindRange, theObject);
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_Delete
*  ---------------------------------------------------------------------------------
*  Destructor */

void LQuickFindRange_Delete (LQuickFindRange** ThisA)
{		
	ui4 i;
	LArray* theTree;	
	LHash* theTmpHashTable;
	ui4* theMasterArray;
	
	theMasterArray=(*ThisA)->mMasterArray;
	
	/*creates tmp hash table...*/
	theTmpHashTable=LHash_New ();
	
	/*deletes all larrays...*/
	for (i=0; i<((*ThisA)->mRange); i++)
	{
		/*retrieves pointer to tree...*/
		theTree=(LArray *)theMasterArray[i];
		/*if item not present, jump to next iteration...*/
		if (theTree==NULL)
			continue;
		/*if not in tmp hash table...*/
		if (!LHash_IsInTable (theTmpHashTable, (ui4)theTree))
		{
			/*insert in tmp hash table...*/
			LHash_InsertItem (theTmpHashTable, NULL, (ui4)theTree);
			/*delete tree...*/
			LArray_Delete (&theTree);
		}		
	}
	
	/*deletes tmp hash table...*/
	LHash_Delete (&theTmpHashTable);
	
	/*deallocates master array...*/
	LMemory_Free (&((*ThisA)->mMasterArray));
		
	/*deletes object...*/
	LMemory_DeleteObject (ThisA);
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_MakeSet
*  ---------------------------------------------------------------------------------
*  MakeSet */
void LQuickFindRange_MakeSet (LQuickFindRange* This, ui4 inItem)
{
	LArray* theNewTree;	
	ui4* theMasterArray;
	
	/*parameter checking...*/
	if ( (inItem<0) || (inItem>((This->mRange)-1)) )
		return;
	
	/*if set already exists...*/
	if (LQuickFindRange_Find (This, inItem)!=LQuickFindRange_BAD_ITEM)
		return; /*does nothing!*/
	
	/*creates new 1-level tree...size of each leaf is 4 bytes...*/
	theNewTree=LArray_New (4);
	
	/*inserts first item in new tree (this also identifies the tree)...*/
	LArray_AppendItem (theNewTree, &inItem);
	
	/*inserts pointer to tree (LArray) into master array...*/
	theMasterArray=This->mMasterArray;
	theMasterArray[inItem]=(ui4)theNewTree;
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_Find
*  ---------------------------------------------------------------------------------
*  Find */
ui4 LQuickFindRange_Find (LQuickFindRange* This, ui4 inItem)
{
	LArray *thelarray;
	ui4 *theMasterArray;
	
	theMasterArray=This->mMasterArray;
	
	/*parameter checking...*/
	if ( (inItem<0) || (inItem>((This->mRange)-1)) )
		return LQuickFindRange_BAD_ITEM;
	
	/*retrieves tree (LArray) from node id...*/
	thelarray=(LArray *)theMasterArray[inItem];
	
	/*if not found...*/
	if (thelarray==NULL)
		return LQuickFindRange_BAD_ITEM;	
	
	/*returns node id...*/
	return *((ui4 *)LArray_ItemAt (thelarray, 0));
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_Union
*  ---------------------------------------------------------------------------------
*  Union */
ui4 LQuickFindRange_Union (LQuickFindRange* This, ui4 inItem1, ui4 inItem2)
{
	ui4 thelength1, thelength2;
	LArray *thelarray1, *thelarray2;
	ui4 i;
	ui4 *thecur_item;
	ui4 theset1, theset2;
	
	void *theSource, *theDest;
	
	ui4* theMasterArray;
	
	/*parameter checking...*/
	if ( (inItem1<0) || (inItem1>((This->mRange)-1)) )
		return LQuickFindRange_BAD_ITEM;
	
	if ( (inItem2<0) || (inItem2>((This->mRange)-1)) )
		return LQuickFindRange_BAD_ITEM;
	
	theset1=LQuickFindRange_Find (This, inItem1);
	theset2=LQuickFindRange_Find (This, inItem2);

	/*if either set doesn't exist...*/
	if ((theset1==LQuickFindRange_BAD_ITEM) || (theset2==LQuickFindRange_BAD_ITEM))
		return LQuickFindRange_BAD_ITEM; /*returns an invalid id...*/
		
	/*if already in same set...*/
	if (theset1==theset2)
		return theset1; //nothing to do...
	
	theMasterArray=This->mMasterArray;
	
	/*retrieves 1st tree (LArray)...*/
	thelarray1=(LArray *)theMasterArray[inItem1];
	
	/*retrieves 2nd tree (LArray)...*/
	thelarray2=(LArray *)theMasterArray[inItem2];
	
	/*gets lengths...*/
	thelength1=LArray_GetItemsCount (thelarray1);
	thelength2=LArray_GetItemsCount (thelarray2);
	
	/*performs (balanced) union...*/
	if (thelength1 >= thelength2)
	{
		/*resizes larray1...*/
		LArray_ResizeBy (thelarray1, thelength2);
		/*gets address of first item of larray2...*/
		theSource=LArray_ItemAt (thelarray2, 0);
		/*gets first empty slot in larray1...*/
		theDest=LArray_ItemAt (thelarray1, thelength1);
		/*copies larray2 onto larray1...*/
		LMemory_Copy (theSource, theDest, thelength2*4);
		
		for (i=0; i<thelength2; i++)
		{
			thecur_item=(ui4 *)LArray_ItemAt (thelarray2, i);
			/*replaces item in master array...*/
			//LHash_InsertItem (This->mHashTable, (void *)thelarray1, *thecur_item);
			//LHash_ReplaceItemByKey (This->mHashTable, *thecur_item, (void *)thelarray1);
			theMasterArray[*thecur_item]=(ui4)thelarray1;
		}	
		LArray_Delete (&thelarray2);
	}
	else
	{
		/*renames tree as well...*/
		thecur_item=(ui4 *)LArray_ItemAt (thelarray1, 0);
		LArray_InsertItemAt (thelarray2, thecur_item, 0);
		/*replaces item in hash table...*/
		theMasterArray[*thecur_item]=(ui4)thelarray2;
		//LHash_InsertItem (This->mHashTable, (void *)thelarray2, *thecur_item);
		
		if (thelength1>1)
		{
			/*resizes larray2...*/
			LArray_ResizeBy (thelarray2, thelength1-1);
			/*gets address of second item of larray1...*/
			theSource=LArray_ItemAt (thelarray1, 1);
			/*gets first empty slot in larray1...*/
			theDest=LArray_ItemAt (thelarray2, thelength2+1);
			/*copies larray1 onto larray2...*/
			LMemory_Copy (theSource, theDest, (thelength1-1)*4);
		}
		
		for (i=1; i<thelength1; i++)
		{
			thecur_item=(ui4 *)LArray_ItemAt (thelarray1, i);
			//LArray_AppendItem (thelarray2, thecur_item);
			/*replaces item in hash table...*/
			//LHash_InsertItem (This->mHashTable, (void *)thelarray2, *thecur_item);
			//LHash_ReplaceItemByKey (This->mHashTable, *thecur_item, (void *)thelarray2);
			theMasterArray[*thecur_item]=(ui4)thelarray2;
		}
		LArray_Delete (&thelarray1);
	}

	/*returns appropriate set name...*/
	if (thelength1 >= thelength2)
		return theset1;
	else
		return theset2;
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_GetUsedMem
*  ---------------------------------------------------------------------------------
*  GetUsedMem */
ui4 LQuickFindRange_GetUsedMem (LQuickFindRange* This)
{
	ui4 theMasterArrayUsedMem=0;
	ui4 theQuickFindRangeStructUsedMem=0;
	ui4 theLArraysUsedMem=0;
	
	LArray* theTree;
	ui4 i;
	LHash* theTmpHashTable;
	
	ui4* theMasterArray;
	
	/*creates tmp hash table...*/
	theTmpHashTable=LHash_New ();
	
	/*size of master array...*/
	theMasterArrayUsedMem=(This->mRange)*4;
	
	/*size of struct LQuickFindRange...*/
	theQuickFindRangeStructUsedMem=sizeof (struct LQuickFindRange);
	
	theMasterArray=This->mMasterArray;
	
	/*size of all larrays...*/
	for (i=0; i<(This->mRange); i++)
	{	
		/*retrieves pointer to tree...*/
		theTree=(LArray *)theMasterArray[i];
		
		if (theTree==NULL)
			continue;
		
		/*if not in tmp hash table...*/
		if (!LHash_IsInTable (theTmpHashTable, (ui4)theTree))
		{
			/*insert in tmp hash table...*/
			LHash_InsertItem (theTmpHashTable, NULL, (ui4)theTree);
			/*take size into account...*/
			theLArraysUsedMem=theLArraysUsedMem + LArray_GetUsedMem(theTree);
		}	
	}
	
	/*deletes tmp hash table...*/
	LHash_Delete (&theTmpHashTable);
		
	return (theMasterArrayUsedMem + theQuickFindRangeStructUsedMem + theLArraysUsedMem);
}

/* ---------------------------------------------------------------------------------
*  LQuickFindRange_GetAllItems
*  ---------------------------------------------------------------------------------
*  GetAllItems */
LArray* LQuickFindRange_GetAllItems (LQuickFindRange* This)
{
	LArray *theRes;
	ui4 i;
	LArray *tmp;
	ui4* theMasterArray;
	
	theMasterArray=This->mMasterArray;
	
	/*creates array of ui4...*/
	theRes=LArray_New (4);
	
	/*scans master array...*/
	for (i=0; i<This->mRange; i++)
	{
		/*if item present...*/
		tmp=(LArray *)theMasterArray[i];
		if (tmp!=NULL)
		{
			/*appends index...*/
			LArray_AppendItem (theRes, &i);
		}
	}
	
	/*returns pointer to larray...*/
	return theRes;
}

/* Copyright (C) 2006 Andrea Ribichini

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
