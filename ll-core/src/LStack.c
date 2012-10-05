/* ============================================================================
 *  LStack.c
 * ============================================================================

 *  Author:         (C) 2003 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        Nov 12, 2003
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/

#include "LStack.h"
#include "LArray.h"
#include "LMemory.h"
#include "LException.h"

/* TYPES */
struct LStack {
    LArray*      mArray;
    LType_TType  mItemType;
};


/* PUBLIC FUNCTIONS */

/* ---------------------------------------------------------------------------------
 *  New
 * ---------------------------------------------------------------------------------
 * creates a new stack with item type inType  */
LStack* LStack_New(LType_TType inType){
    LStack  theObject = { 0 };

    Try {
        theObject.mItemType = inType;
        theObject.mArray    = LArray_New(inType.mSize);
    }
    CatchAny {
        if (theObject.mArray != NULL) LArray_Delete(&theObject.mArray);
        Rethrow;
    }

    return LMemory_NewObject(LStack, theObject);
}


/* ---------------------------------------------------------------------------------
 *  Delete
 * ---------------------------------------------------------------------------------
 * deallocates the stack pointed to by *ThisA */
void LStack_Delete(LStack** ThisA){
    LArray_Delete(&(*ThisA)->mArray);
    LMemory_DeleteObject(ThisA);
}


/* ---------------------------------------------------------------------------------
 *  IsEmpty
 * ---------------------------------------------------------------------------------
 * returns TRUE if the stack is empty, and FALSE otherwise */
Bool LStack_IsEmpty(LStack* This){
    return (Bool)(!LArray_GetItemsCount(This->mArray));
}


/* ---------------------------------------------------------------------------------
 *  GetItemsCount
 * ---------------------------------------------------------------------------------
 * returns the number of items in the stack */
ui4 LStack_GetItemsCount(LStack* This){
    return LArray_GetItemsCount(This->mArray);
}


/* ---------------------------------------------------------------------------------
 *  Push
 * ---------------------------------------------------------------------------------
 * puts the item pointed to by inItem on top of the stack */
void LStack_Push(LStack* This, const void* inItem){
    LArray_AppendItem(This->mArray, inItem);
}


/* ---------------------------------------------------------------------------------
 *  Pop
 * ---------------------------------------------------------------------------------
 * removes the top item from the stack */
void LStack_Pop(LStack* This) {
    if (LArray_GetItemsCount(This->mArray) < 1) Throw(LStack_EMPTY_STACK);
    LArray_RemoveLastItem(This->mArray);
}


/* ---------------------------------------------------------------------------------
 *  Top
 * ---------------------------------------------------------------------------------
 * copies to address outItem the item on top of the stack */
void LStack_Top(LStack* This, void* outItem){
    ui4 theCount = LArray_GetItemsCount(This->mArray);
    if (theCount < 1) Throw(LStack_EMPTY_STACK);
    LArray_FetchItemAt(This->mArray, theCount-1, outItem);
}


/* ---------------------------------------------------------------------------------
 *  MultiPop
 * ---------------------------------------------------------------------------------
 * pops the inItemsCount topmost items from the stack, 
 * if inItemsCount is greater than the size of the stack, the stack gets empty */
void LStack_MultiPop(LStack* This, ui4 inItemsCount) {
    if (LArray_GetItemsCount(This->mArray) < inItemsCount) 
        inItemsCount = LArray_GetItemsCount(This->mArray);
    LArray_ResizeBy(This->mArray, -(i4)inItemsCount);
}


/* ---------------------------------------------------------------------------------
 *  GetUsedMem
 * ---------------------------------------------------------------------------------
 * returns the total numner of bytes required for storing the stack */
ui4 LStack_GetUsedMem(LStack* This){
    return sizeof(LStack) + LArray_GetUsedMem(This->mArray);
}


/* ---------------------------------------------------------------------------------
 *  GetItemType
 * ---------------------------------------------------------------------------------
 * returns the type descriptor of stack items */
LType_TType LStack_GetItemType(LStack* This){
    return This->mItemType;
}


/* Copyright (C) 2003 Irene Finocchi

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
