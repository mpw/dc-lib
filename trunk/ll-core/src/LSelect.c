/* ============================================================================
 *  LSelect.c
 * ============================================================================

 *  Author:         (C) 2005 Irene Finocchi, Francesco Iovine
 *  License:        See the end of this file for license information
 *  Created:        Feb 7, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2007/03/31 10:04:32 $
 *  Changed by:     $Author: irene $
 *  Revision:       $Revision: 1.2 $
*/


#include "LSelect.h"
#include "LException.h"
#include "LRandSource.h"
#include "LSort.h"
#include "LTime.h"


/* MACROS */
#define LSelect_Swap_(inItems, inIdx1, inIdx2, inSize) {            \
    i1 *u, *v, *a = (i1*)(inItems);   i1 temp;   ui4 s = (inSize);  \
    u = a + (inIdx1)*(inSize);   v = a + (inIdx2)*(inSize);         \
    while (s--) { temp = *u;   *u++ = *v;   *v++ = temp; }          \
}


/* PRIVATE FUNCTIONS PROTOTYPES */
void _LSelect_Rand(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inSelect, 
				   LRandSource* inRandSource);
ui4 _LSelect_MedianOfThreePivot(void* inBase, ui4 inItemsCount, LType_TType inItemType, 
								LRandSource* inRandSource);
void _LSelect_MedianOfFive(void* inItems, LType_TType inItemType);
ui4 _LSelect_Partition(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx);


/* PUBLIC FUNCTIONS */

/* ---------------------------------------------------------------------------------
 *  LSelect_Rand
 * ---------------------------------------------------------------------------------
 * Randomized selection: at the end of execution, the item with rank inSelect can be
 * found in inItems[inSelect]
*/
void LSelect_Rand(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inSelect)
{
    const ui4 theSeed = 1 + (ui4)LTime_GetUserTime();
    LRandSource* theRandSource = LRandSource_New(theSeed);

    _LSelect_Rand(inItems, inItemsCount, inItemType, inSelect, theRandSource);
    LRandSource_Delete(&theRandSource);
}


/* ---------------------------------------------------------------------------------
 *  LSelect_Determ
 * ---------------------------------------------------------------------------------
 * Deterministic selection: at the end of execution, the item with rank inSelect can be
 * found in inItems[inSelect]
*/
void LSelect_Determ(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inSelect)
{
    ui4 theGroupID, theNumOfMedians, thePivotIdx;

    /* throw exception BADSELECT */
    if (inSelect < 1 || inSelect > inItemsCount) Throw(LSelect_BAD_SELECT);

    /* base case */
    if (inItemsCount < 10) {
        LSort_Selection(inItems, inItemsCount, inItemType);
        return;
    }

    /* find the median of each group of five items and store the medians at the */ 
	/* beginning of the array                                                   */
    for (theGroupID = 0; theGroupID * 5 <= inItemsCount - 5; theGroupID++) {
        _LSelect_MedianOfFive((i1 *)inItems + 5*theGroupID*inItemType.mSize, inItemType);
        LSelect_Swap_(inItems, theGroupID*5 + 2, theGroupID, inItemType.mSize);
    }

    /* find the median of medians */
    theNumOfMedians = inItemsCount / 5;
    thePivotIdx = theNumOfMedians / 2;
    LSelect_Determ(inItems, theNumOfMedians, inItemType, thePivotIdx);

    /* partition around the median of medians */
    thePivotIdx = _LSelect_Partition(inItems, inItemsCount, inItemType, thePivotIdx);

	/* the item of rank inSelect is the pivot itself */
    if (inSelect-1 == thePivotIdx) return;
	
    /* appropriate recursive call */
	if (inSelect-1 < thePivotIdx)
		LSelect_Determ(inItems, thePivotIdx, inItemType, inSelect);
	else	/* if (inSelect-1 > thePivotIdx) */
		LSelect_Determ((i1 *)inItems + (thePivotIdx+1)*inItemType.mSize,
                      inItemsCount - thePivotIdx - 1,
                      inItemType,
                      inSelect - thePivotIdx - 1);
}


/* PRIVATE FUNCTIONS */

/* ---------------------------------------------------------------------------------
 *  _LSelect_Rand
 * ---------------------------------------------------------------------------------
 * Randomized selection: at the end of execution, the item with rank inSelect can be
 * found in inItems[inSelect]
*/
void _LSelect_Rand(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
                   ui4 inSelect, LRandSource* inRandSource)
{
    ui4 thePivotIdx;

    /* throw exception BADSELECT */
    if (inSelect < 1 || inSelect > inItemsCount) Throw(LSelect_BAD_SELECT);

    /* base case */
    if (inItemsCount < 10) {
        LSort_Selection(inItems, inItemsCount, inItemType);
        return;
    }

    /* find a pivot */
    thePivotIdx = _LSelect_MedianOfThreePivot(inItems, inItemsCount, inItemType, 
											  inRandSource);
    
    /* partition around pivot */
    thePivotIdx = _LSelect_Partition(inItems, inItemsCount, inItemType, thePivotIdx);

	/* the item of rank inSelect is the pivot itself */
    if (inSelect-1 == thePivotIdx) return;
	
    /* appropriate recursive call */
	if (inSelect-1 < thePivotIdx)
		_LSelect_Rand(inItems, thePivotIdx, inItemType, inSelect, inRandSource);
	else	/* if (inSelect-1 > thePivotIdx) */
		_LSelect_Rand((i1 *)inItems + (thePivotIdx+1)*inItemType.mSize,
                      inItemsCount - thePivotIdx - 1,
                      inItemType,
                      inSelect - thePivotIdx - 1,
                      inRandSource);
}


/* ---------------------------------------------------------------------------------
 *  _LSelect_MedianOfThreePivot
 * ---------------------------------------------------------------------------------
 * Find the pivot as the median of three randomly chosen items
*/
ui4 _LSelect_MedianOfThreePivot(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
							    LRandSource* inRandSource)
{
    ui4 theRandPivot1, thePivotIdx, theRandPivot3;
    void *theRandPivot1Ptr, *thePivotIdxPtr, *theRandPivot3Ptr;

    /* find three random numbers in the range [0, inItemsCount-1] */
    theRandPivot1 = LRandSource_GetRandUI4(inRandSource, 0, inItemsCount-1);
    thePivotIdx = LRandSource_GetRandUI4(inRandSource, 0, inItemsCount-1);
    theRandPivot3 = LRandSource_GetRandUI4(inRandSource, 0, inItemsCount-1);

    /* the three numbers are the indexes for inItems */
    theRandPivot1Ptr = (i1 *)inItems + theRandPivot1*inItemType.mSize;
    thePivotIdxPtr = (i1 *)inItems + thePivotIdx*inItemType.mSize;
    theRandPivot3Ptr = (i1 *)inItems + theRandPivot3*inItemType.mSize;

    /* sort the three elements */
    if ((*inItemType.mCompar)(thePivotIdxPtr, theRandPivot1Ptr) < 0)
        LSelect_Swap_(inItems, theRandPivot1, thePivotIdx, inItemType.mSize)
    if ((*inItemType.mCompar)(theRandPivot3Ptr, theRandPivot1Ptr) < 0)
        LSelect_Swap_(inItems, theRandPivot1, theRandPivot3, inItemType.mSize)
    if ((*inItemType.mCompar)(theRandPivot3Ptr, thePivotIdxPtr) < 0)
        LSelect_Swap_(inItems, thePivotIdx, theRandPivot3, inItemType.mSize)

    return thePivotIdx;  /* returns the index of the middle element */
}


/* ---------------------------------------------------------------------------------
 *  _LSelect_MedianOfFive
 * ---------------------------------------------------------------------------------
 * Put the median item to the position 2 of the group of 5 elements
 * Uses the optimal number of comparisons (6 comparisons) and the same number of swaps
*/
void _LSelect_MedianOfFive(void* inItems, LType_TType inItemType)
{
    void* theItem0 = inItems;
    void* theItem1 = (i1 *)inItems + inItemType.mSize;
    void* theItem2 = (i1 *)inItems + 2*inItemType.mSize;
    void* theItem3 = (i1 *)inItems + 3*inItemType.mSize;
    void* theItem4 = (i1 *)inItems + 4*inItemType.mSize;

    if ((*inItemType.mCompar)(theItem0, theItem1) > 0) {
        LSelect_Swap_(inItems, 0, 1, inItemType.mSize);
    }
	/* Now x0 <= x1 */

    if ((*inItemType.mCompar)(theItem2, theItem3) > 0) {
        LSelect_Swap_(inItems, 2, 3, inItemType.mSize);
    }
	/* Now x2 <= x3 */

    if ((*inItemType.mCompar)(theItem0, theItem2) > 0)  {
		/* exchange the role of the two pairs */
        LSelect_Swap_(inItems, 0, 2, inItemType.mSize);
        LSelect_Swap_(inItems, 1, 3, inItemType.mSize);
    }
	/* Now x0 <= x2, and thus x0 cannot be the median */

    if ((*inItemType.mCompar)(theItem1, theItem4) > 0) {
        LSelect_Swap_(inItems, 1, 4, inItemType.mSize);
    }
	/* Now x1 <= x4 */

    if ((*inItemType.mCompar)(theItem1, theItem2) < 0) {
		/* Now x1 <= x2, and thus x1 cannot be the median */
	
		if ((*inItemType.mCompar)(theItem2, theItem4) > 0) {
			/* in this case x4 is the median */
			LSelect_Swap_(inItems, 2, 4, inItemType.mSize);
		}
		/* else x2 is the median, and is already in the proper position */
    }
	else {
		/* Now x2 <= x1, and thus x2 cannot be the median */

		if ((*inItemType.mCompar)(theItem1, theItem3) < 0) {
			/* in this case x1 is the median */
			LSelect_Swap_(inItems, 2, 1, inItemType.mSize);
		}
		else {
			/* in this case x3 is the median */
			LSelect_Swap_(inItems, 2, 3, inItemType.mSize);
		}
	}
}


/* ---------------------------------------------------------------------------------
 *  _LSelect_Partition
 * ---------------------------------------------------------------------------------
*/
ui4 _LSelect_Partition(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx)
{
    ui4 theLoadPos, theStorePos;
    void *thePivotIdxPtr, *theLoadPosPtr;

    /* Put the pivot at the end of the array */
    LSelect_Swap_(inItems, inPivotIdx, inItemsCount - 1, inItemType.mSize);
    thePivotIdxPtr = (i1 *)inItems + (inItemsCount - 1)*inItemType.mSize;

    /* Compare each item with pivot: if smaller, put it at the beginning of the array */
	/* The algorithm maintains the following invariant: at any time                   */ 
	/*      - theStorePos <= theLoadPos                                               */
	/*      - inItems[0] ... inItems[theStorePos-1] contains items < pivot            */
	/*      - inItems[theStorePos] ... inItems[theLoadPos-1] contains items >= pivot  */
    for (theLoadPos = 0, theStorePos = 0; theLoadPos < inItemsCount - 1; theLoadPos++) {
        theLoadPosPtr = (i1 *)inItems + theLoadPos*inItemType.mSize;
        if ((*inItemType.mCompar)(theLoadPosPtr, thePivotIdxPtr) < 0) {
            LSelect_Swap_(inItems, theLoadPos, theStorePos, inItemType.mSize);
            theStorePos++;
        }
    }

    /* Put pivot at the end of the swapped items and return its position */
    LSelect_Swap_(inItems, theStorePos, inItemsCount - 1, inItemType.mSize);
    return theStorePos;
}


/* Copyright (C) 2005 Irene Finocchi and Francesco Iovine

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
