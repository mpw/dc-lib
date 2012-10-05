/* ============================================================================
 *  LSort.c
 * ============================================================================

 *  Author:         (C) 2005 Camil Demetrescu, Irene Finocchi, Francesco Iovine
 *  License:        See the end of this file for license information
 *  Created:        Feb 7, 2005
 *  Module:         LL

 *  Last changed:   $Date: 2007/03/31 10:04:32 $
 *  Changed by:     $Author: irene $
 *  Revision:       $Revision: 1.3 $
*/


#include "LSort.h"
#include "LArray.h"
#include "LMemory.h"
#include "LRandSource.h"
#include "LTime.h"
#include "LException.h"


/* MACROS */
#define LSort_Swap_(inItems, inIdx1, inIdx2, inSize) {              \
    i1 *u, *v, *a = (i1*)(inItems);   i1 temp;   ui4 s = (inSize);  \
    u = a + (inIdx1)*(inSize);   v = a + (inIdx2)*(inSize);         \
    while (s--) { temp = *u;   *u++ = *v;   *v++ = temp; }          \
}


/* PRIVATE FUNCTIONS PROTOTYPES */
void _LSort_Merge(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx);
void _LSort_RecQuick(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
					 LRandSource* inRandSource);
ui4 _LSort_MedianOfThreePivot(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
							  LRandSource* inRandSource);
ui4 _LSort_Partition(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx);


/* PUBLIC FUNCTIONS */

/* ---------------------------------------------------------------------------------
 *  Selection
 * ---------------------------------------------------------------------------------
 * Selection Sort
*/
void LSort_Selection(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    ui4 i, j, minIdx;        
    
    for (i = 0; i < inItemsCount-1; i++) {
		minIdx = i;
        for (j = i + 1; j < inItemsCount; j++) {
            if ((*inItemType.mCompar)((i1 *)inItems + j*inItemType.mSize, 
									  (i1 *)inItems + minIdx*inItemType.mSize) < 0) 
				minIdx = j;
		}
		if (minIdx != i) LSort_Swap_(inItems, minIdx, i, inItemType.mSize)
    }
}


/* ---------------------------------------------------------------------------------
 *  Insertion
 * ---------------------------------------------------------------------------------
 * Insertion Sort
*/
void LSort_Insertion(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    ui4 i, j;        
    void *jPtr, *j1Ptr;
    
    for (i = 1; i < inItemsCount; i++) {
        for ( j = i; j >= 1; j-- ) {
            jPtr = (i1 *)inItems + j*inItemType.mSize;
            j1Ptr = (i1 *)inItems + (j-1)*inItemType.mSize;
            if ((*inItemType.mCompar)(jPtr, j1Ptr) < 0)			/* Not in place yet  */
                LSort_Swap_(inItems, j, j-1, inItemType.mSize)  /* so swap back once */
            else break;						  /* The current element is now in place */
        }
    }
}


/* ---------------------------------------------------------------------------------
 *  Bubble
 * ---------------------------------------------------------------------------------
 * Bubble Sort
*/
void LSort_Bubble(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    ui4 i, j;        
    void *jPtr, *j1Ptr;
    Bool noSwaps;
    
    for (i = 0; i < inItemsCount-1; i++) {
        noSwaps = TRUE;					 /* Checks to see if any swaps happened */

        for (j = 1; j < inItemsCount-i; j++) {
            jPtr = (i1 *)inItems + j*inItemType.mSize;
            j1Ptr = (i1 *)inItems + (j-1)*inItemType.mSize;
            if ((*inItemType.mCompar)(jPtr, j1Ptr) < 0) {
                LSort_Swap_(inItems, j, j-1, inItemType.mSize)
                if (noSwaps) noSwaps = FALSE;				/* There was a swap */
            }
        }

        /* If the sort didn't do anything (i.e. if the array is sorted), 
         * then stop the algorithm. */
        if (noSwaps) break;
    }
}


/* ---------------------------------------------------------------------------------
 *  RecMerge
 * ---------------------------------------------------------------------------------
 * Recursive implementation of Merge Sort
*/
void LSort_RecMerge(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    ui4 theHalfIdx;
        
    if (inItemsCount == 1) return;

    theHalfIdx = inItemsCount / 2;
        
    LSort_RecMerge(inItems, theHalfIdx, inItemType);
    LSort_RecMerge((i1 *)inItems + theHalfIdx*inItemType.mSize,
                   inItemsCount - theHalfIdx,
                   inItemType);
    _LSort_Merge(inItems, inItemsCount, inItemType, theHalfIdx);
}


/* ---------------------------------------------------------------------------------
 *  IterMerge
 * ---------------------------------------------------------------------------------
 * Iterative bottom up implementation of Merge Sort
*/
void LSort_IterMerge(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    ui4 theSegmentSize = 1;
    i4 theFirstIdx, theLastIdx;
    void* theFirstIdxPtr;
    
    while (theSegmentSize < inItemsCount) {
        theFirstIdx = 0;
        theLastIdx = inItemsCount - 2*theSegmentSize;

        /* while there are two segments with the same size to be merged */
        while (theFirstIdx <= theLastIdx) {
            theFirstIdxPtr = (i1 *)inItems + theFirstIdx*inItemType.mSize;            
           _LSort_Merge(theFirstIdxPtr, 2*theSegmentSize, inItemType, theSegmentSize);
            theFirstIdx = theFirstIdx + 2*theSegmentSize;
        }

        /* there are two segments of different size to be merged */
        if (theFirstIdx + theSegmentSize < inItemsCount) {
            theFirstIdxPtr = (i1 *)inItems + theFirstIdx*inItemType.mSize;
            _LSort_Merge(theFirstIdxPtr, inItemsCount-theFirstIdx, inItemType, theSegmentSize);
        }

        theSegmentSize += theSegmentSize;  /* double the segment size */
    }
}


/* ---------------------------------------------------------------------------------
 *  RecQuick
 * ---------------------------------------------------------------------------------
 * Recursive Quick Sort
*/
void LSort_RecQuick(void* inItems, ui4 inItemsCount, LType_TType inItemType)
{
    const ui4 theSeed = 1 + (ui4)LTime_GetUserTime();
    LRandSource* theRandSource = LRandSource_New(theSeed);

    _LSort_RecQuick(inItems, inItemsCount, inItemType, theRandSource);
    LRandSource_Delete(&theRandSource);
}


/* ---------------------------------------------------------------------------------
 *  Shell
 * ---------------------------------------------------------------------------------
 * Shell Sort with decreasing step size n/2, n/4, n/8, ..., 2, 1
*/
void LSort_Shell(void* inItems, ui4 inItemsCount, LType_TType inItemType){
    i1 *a, *b = (i1*)inItems;
    i4 x, y, g, ng, i, j;
    i = inItemsCount*inItemType.mSize;

    for (ng = inItemsCount >> 1; ng > 0; ng >>= 1){
        g = ng * inItemType.mSize;
        j = g + inItemType.mSize;
        a = b + g;
        for (x = j; x <= i; x += inItemType.mSize)
            for (y = x-j; y >= 0; y -= g)
                if ((*inItemType.mCompar)(b+y, a+y) <= 0) break;
                else {
                    i4 w = inItemType.mSize;
                    i1 *u = b+y, *v = a+y;
                    while (w--) {
                        i1 temp = *u;
                        *u++ = *v;
                        *v++ = temp;
                    }
                }
    }
}



/* PRIVATE FUNCTIONS */

/* ---------------------------------------------------------------------------------
 *  _LSort_Merge
 * ---------------------------------------------------------------------------------
 * Merge two sorted segments, possibly of different size, into one sorted segment.
 * The first segment starts at position 0, the second segment starts at position
 * inPivotIdx. The total number of elements to be merged is inItemsCount.
*/
void _LSort_Merge(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx)
{
    ui4 theIdx1, theIdx2;
    void *theIdx1Ptr, *theIdx2Ptr;
	LArray *theTempArray;
	
	Try {
		theTempArray = LArray_New(inItemType.mSize);
	}
	CatchAny {
		Rethrow;
	}
    
    theIdx1 = 0;           /* index of the first  segment */
    theIdx2 = inPivotIdx;  /* index of the second segment */

    /* put sorted items into theTempArray until theIdx1 or theIdx2 exits its segment */
    while (theIdx1 < inPivotIdx && theIdx2 < inItemsCount) {
        theIdx1Ptr = (i1 *)inItems + theIdx1*inItemType.mSize;
        theIdx2Ptr = (i1 *)inItems + theIdx2*inItemType.mSize;
        if ((*inItemType.mCompar)(theIdx1Ptr, theIdx2Ptr) <= 0) {
            LArray_AppendItem(theTempArray, theIdx1Ptr);
            theIdx1++;
        }
        else {
            LArray_AppendItem(theTempArray, theIdx2Ptr);
            theIdx2++;    
        }
    }
    
    /* put remaining sorted items into theTempArray */
    while (theIdx1 < inPivotIdx) {
        theIdx1Ptr = (i1 *)inItems + theIdx1*inItemType.mSize;
        LArray_AppendItem(theTempArray, theIdx1Ptr);
        theIdx1++;
    }
    while (theIdx2 < inItemsCount) {
        theIdx2Ptr = (i1 *)inItems + theIdx2*inItemType.mSize;
        LArray_AppendItem(theTempArray, theIdx2Ptr);
        theIdx2++;
    }
    
    /* copy theTempArray into inItems */
    for (theIdx1 = 0; theIdx1 < inItemsCount; theIdx1++) {
        theIdx1Ptr = (i1 *)inItems + theIdx1*inItemType.mSize;
        LArray_FetchItemAt(theTempArray, theIdx1, theIdx1Ptr);
    }
    
    if (theTempArray) LArray_Delete(&theTempArray);
}


/* ---------------------------------------------------------------------------------
 *  _LSort_RecQuick
 * ---------------------------------------------------------------------------------
 * Recursive implementation of Quick Sort with random pivot selection
*/
void _LSort_RecQuick(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
					 LRandSource* inRandSource)
{
    ui4 thePivotIdx;

    /* base case */
    if (inItemsCount < 10) {
        LSort_Shell(inItems, inItemsCount, inItemType);
        return;
    }
          
    /* find a pivot */
    thePivotIdx = _LSort_MedianOfThreePivot(inItems, inItemsCount, 
											inItemType, inRandSource);
    
    /* partition around pivot */
    thePivotIdx = _LSort_Partition(inItems, inItemsCount, inItemType, thePivotIdx);

    /* sort with randomized pivot */
    _LSort_RecQuick(inItems, thePivotIdx, inItemType, inRandSource);
    _LSort_RecQuick((i1 *)inItems + (thePivotIdx+1)*inItemType.mSize,
                    inItemsCount - thePivotIdx - 1, inItemType, inRandSource);
}


/* ---------------------------------------------------------------------------------
 *  _LSort_MedianOfThreePivot
 * ---------------------------------------------------------------------------------
 * Find the pivot as the median of three randomly chosen items
*/
ui4 _LSort_MedianOfThreePivot(void* inItems, ui4 inItemsCount, LType_TType inItemType, 
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
        LSort_Swap_(inItems, theRandPivot1, thePivotIdx, inItemType.mSize)
    if ((*inItemType.mCompar)(theRandPivot3Ptr, theRandPivot1Ptr) < 0)
        LSort_Swap_(inItems, theRandPivot1, theRandPivot3, inItemType.mSize)
    if ((*inItemType.mCompar)(theRandPivot3Ptr, thePivotIdxPtr) < 0)
        LSort_Swap_(inItems, thePivotIdx, theRandPivot3, inItemType.mSize)

    return thePivotIdx;  /* returns the index of the middle element */
}


/* ---------------------------------------------------------------------------------
 *  _LSort_Partition
 * ---------------------------------------------------------------------------------
*/
ui4 _LSort_Partition(void* inItems, ui4 inItemsCount, LType_TType inItemType, ui4 inPivotIdx)
{
    ui4 theLoadPos, theStorePos;
    void *thePivotIdxPtr, *theLoadPosPtr;

    /* Put the pivot at the end of the array */
    LSort_Swap_(inItems, inPivotIdx, inItemsCount - 1, inItemType.mSize)
    thePivotIdxPtr = (i1 *)inItems + (inItemsCount - 1)*inItemType.mSize;

    /* Compare each item with pivot: if smaller, put it at the beginning of the array */
	/* The algorithm maintains the following invariant: at any time                   */ 
	/*      - theStorePos <= theLoadPos                                               */
	/*      - inItems[0] ... inItems[theStorePos-1] contains items < pivot            */
	/*      - inItems[theStorePos] ... inItems[theLoadPos-1] contains items >= pivot  */
    for (theLoadPos = 0, theStorePos = 0; theLoadPos < inItemsCount - 1; theLoadPos++) {
        theLoadPosPtr = (i1 *)inItems + theLoadPos*inItemType.mSize;
        if ((*inItemType.mCompar)(theLoadPosPtr, thePivotIdxPtr) < 0) {
            LSort_Swap_(inItems, theLoadPos, theStorePos, inItemType.mSize)
            theStorePos++;
        }
    }

    /* Put pivot at the end of the swapped items and return its position */
    LSort_Swap_(inItems, theStorePos, inItemsCount - 1, inItemType.mSize)
    return theStorePos;
}


/* Copyright (C) 2005  C. Demetrescu, I. Finocchi, F. Iovine

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
