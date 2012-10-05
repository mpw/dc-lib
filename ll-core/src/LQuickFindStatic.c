/* ============================================================================
 *  LQuickFindStatic.c
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        April 7, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2006/04/10 23:04:46 $  
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.1 $
*/

#include "LQuickFindStatic.h"
#include "LArray.h"
#include "LMemory.h"
#include "LHash.h"

/* PUBLIC METHODS */

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_New
*  ---------------------------------------------------------------------------------
*  Constructor */

LQuickFindStatic* LQuickFindStatic_New (ui4 inRange) 
{
	LQuickFindStatic theObject={0};
	ui4 i;

	/*parameter checking...*/
	if (inRange==0 || inRange==0xffffffff)
		return NULL;
	
	/*saves range information...*/
	/*items will be in 0..range-1 */
	theObject.mRange=inRange;
	
	/*allocates memory for master array...*/
	theObject.mMasterArray=(LQuickFindStatic_Item *)LMemory_Malloc (sizeof (LQuickFindStatic_Item)*inRange);
	
	/*sets table to empty...*/
	for (i=0; i<inRange; i++)
	{
		theObject.mMasterArray[i].mParent=LQuickFindStatic_BAD_ITEM;
	}
    
	return LMemory_NewObject (LQuickFindStatic, theObject);
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_Delete
*  ---------------------------------------------------------------------------------
*  Destructor */

void LQuickFindStatic_Delete (LQuickFindStatic** ThisA)
{		
	/*deallocates master array...*/
	LMemory_Free (&(*ThisA)->mMasterArray);
		
	/*deletes object...*/
	LMemory_DeleteObject (ThisA);
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_MakeSet
*  ---------------------------------------------------------------------------------
*  MakeSet */
void LQuickFindStatic_MakeSet (LQuickFindStatic* This, ui4 inItem)
{
	/*if set already exists...*/
	if (LQuickFindStatic_Find (This, inItem)!=LQuickFindStatic_BAD_ITEM)
		return; /*does nothing!*/
	
	/*set representative points to itself...*/
	This->mMasterArray[inItem].mParent=inItem;
	/*and initially doesn't have any children...*/
	This->mMasterArray[inItem].mFirstChild=LQuickFindStatic_BAD_ITEM;
	This->mMasterArray[inItem].mNumChildren=0;
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_Find
*  ---------------------------------------------------------------------------------
*  Find */
ui4 LQuickFindStatic_Find (LQuickFindStatic* This, ui4 inItem)
{
	/*parameter checking...*/
	if ( (inItem<0) || (inItem>((This->mRange)-1)) )
		return LQuickFindStatic_BAD_ITEM;
	
	/*returns parent's id...*/
	return This->mMasterArray[inItem].mParent;
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_Union
*  ---------------------------------------------------------------------------------
*  Union */
ui4 LQuickFindStatic_Union (LQuickFindStatic* This, ui4 inItem1, ui4 inItem2)
{
	ui4 theset1, theset2;
	ui4 theTempPtr, theCurPtr, theOldPtr;
	
	theset1=LQuickFindStatic_Find (This, inItem1);
	theset2=LQuickFindStatic_Find (This, inItem2);

	/*if either set doesn't exist...*/
	if ((theset1==LQuickFindStatic_BAD_ITEM) || (theset2==LQuickFindStatic_BAD_ITEM))
		return LQuickFindStatic_BAD_ITEM; /*returns an invalid id...*/
		
	/*if already in same set...*/
	if (theset1==theset2)
		return theset1; /*nothing to do...*/
	
	/*performs balanced union...*/
	if (This->mMasterArray[theset1].mNumChildren>=This->mMasterArray[theset2].mNumChildren)
	{
		/*saves pointer to first child of set 1...*/
		theTempPtr=This->mMasterArray[theset1].mFirstChild;
		/*updates num children...*/
		This->mMasterArray[theset1].mNumChildren=This->mMasterArray[theset1].mNumChildren+This->mMasterArray[theset2].mNumChildren+1;
		
		/*father of set 2 becomes first child of set 1...*/
		This->mMasterArray[theset1].mFirstChild=theset2;
		This->mMasterArray[theset2].mParent=theset1;
		This->mMasterArray[theset2].mNextSibling=This->mMasterArray[theset2].mFirstChild;
		theOldPtr=theset2;
		theCurPtr=This->mMasterArray[theset2].mFirstChild;
		This->mMasterArray[theset2].mFirstChild=LQuickFindStatic_BAD_ITEM;
		
		while (theCurPtr!=LQuickFindStatic_BAD_ITEM)
		{
			This->mMasterArray[theCurPtr].mParent=theset1;
			theOldPtr=theCurPtr;
			theCurPtr=This->mMasterArray[theCurPtr].mNextSibling;
		}
		This->mMasterArray[theOldPtr].mNextSibling=theTempPtr;
		
		return theset1;
	}
	else
	{
		/*saves pointer to first child of set 2...*/
		theTempPtr=This->mMasterArray[theset2].mFirstChild;
		/*updates num children...*/
		This->mMasterArray[theset2].mNumChildren=This->mMasterArray[theset2].mNumChildren+This->mMasterArray[theset1].mNumChildren+1;
		
		/*father of set 1 becomes first child of set 2...*/
		This->mMasterArray[theset2].mFirstChild=theset1;
		This->mMasterArray[theset1].mParent=theset2;
		This->mMasterArray[theset1].mNextSibling=This->mMasterArray[theset1].mFirstChild;
		theOldPtr=theset1;
		theCurPtr=This->mMasterArray[theset1].mFirstChild;
		This->mMasterArray[theset1].mFirstChild=LQuickFindStatic_BAD_ITEM;
		
		while (theCurPtr!=LQuickFindStatic_BAD_ITEM)
		{
			This->mMasterArray[theCurPtr].mParent=theset2;
			theOldPtr=theCurPtr;
			theCurPtr=This->mMasterArray[theCurPtr].mNextSibling;
		}
		This->mMasterArray[theOldPtr].mNextSibling=theTempPtr;
		
		return theset2;
	}
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_GetUsedMem
*  ---------------------------------------------------------------------------------
*  GetUsedMem */
ui4 LQuickFindStatic_GetUsedMem (LQuickFindStatic* This)
{
	return sizeof (LQuickFindStatic) + ((This->mRange)*sizeof (LQuickFindStatic_Item));
}

/* ---------------------------------------------------------------------------------
*  LQuickFindStatic_GetAllItems
*  ---------------------------------------------------------------------------------
*  GetAllItems */
LArray* LQuickFindStatic_GetAllItems (LQuickFindStatic* This)
{
	LArray *theRes;
	ui4 i;
	
	/*creates array of ui4...*/
	theRes=LArray_New (4);
	
	/*scans master array...*/
	for (i=0; i<This->mRange; i++)
	{
		/*if item present...*/
		if (This->mMasterArray[i].mParent!=LQuickFindStatic_BAD_ITEM)
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
