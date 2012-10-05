/* ============================================================================
 *  LHashStatic.c
 * ============================================================================

 *  Author:         (C) 2006 Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        April 12, 2006
 *  Module:         LL

 *  Last changed:   $Date: 2009/11/09 17:27:08 $  
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.4 $
*/

#include <stdio.h>

#include "LHashStatic.h"
#include "LArray.h"
#include "LMemory.h"

/*hash function #1...*/
#define h1_(k,m) (k % m)

/*hash function #2...*/
#define h2_(k,m) ((k % (m-1))+1)

/*index selection function...*/
#define c_(k,i,m) ((h1_(k,m)+(i*h2_(k,m))) % m)

/* PUBLIC METHODS */

/* ---------------------------------------------------------------------------------
*  LHashStatic_New
*  ---------------------------------------------------------------------------------
*  Constructor */

LHashStatic* LHashStatic_New(ui4 inMinNumEntries) 
{
    LHashStatic theObject = {0};
    ui4 i;
    ui4 thePrimes[]={53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
        196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653,
        100663319, 201326611, 402653189, 805306457, 1610612741};
    
    /*parameter checking...*/
    if (inMinNumEntries==0 || inMinNumEntries>1610612741)
        return NULL;
    
    /*selects prime...*/
    for (i=0; inMinNumEntries > thePrimes[i]; i++);
        
    theObject.mNumEntries=thePrimes[i];
    
    /*allocates mem for hash table...*/
    theObject.mHashTable=(struct TSlot*)LMemory_Malloc (sizeof (struct TSlot)*theObject.mNumEntries);
    
    /*sets table to empty (all keys to -1)...*/
    for (i=0; i<theObject.mNumEntries; i++)
    {
        theObject.mHashTable[i].mKey=0xffffffff;
    }

    return LMemory_NewObject(LHashStatic, theObject);
}



/* ---------------------------------------------------------------------------------
*  LHashStatic_Delete
*  ---------------------------------------------------------------------------------
*  Destructor */

void LHashStatic_Delete(LHashStatic** ThisA)
{
    /*deallocates hash table...*/
    LMemory_Free (&(*ThisA)->mHashTable);
        
    /*deletes object...*/
    LMemory_DeleteObject (ThisA);
}


/* ---------------------------------------------------------------------------------
 *  InsertItem
 *  ---------------------------------------------------------------------------------
 *  Inserts item inItem at entry obtained by hashing inKey */

void LHashStatic_InsertItem (LHashStatic* This, void* inItem, ui4 inKey)
{
    ui4 i;

    for (i = 0; i < This->mNumEntries; i++)
    {
        ui4 theIdx = c_(inKey, i, This->mNumEntries);
        if (This->mHashTable[theIdx].mKey==0xffffffff)
        {
            This->mHashTable[theIdx].mKey=inKey;
            This->mHashTable[theIdx].mItemRef=inItem;
            return;    
        }
    }
    printf ("\n[LHashStatic] ERROR: Hash table is full!!!\n");
}


/* ---------------------------------------------------------------------------------
 *  IsInTable
 * ---------------------------------------------------------------------------------
 * Returns TRUE <=> item with key inKey is in the table */
Bool LHashStatic_IsInTable (LHashStatic* This, ui4 inKey)
{
    ui4 i;
     
    for (i = 0; i < This->mNumEntries; i++)
    {    
        if (This->mHashTable[c_(inKey, i, This->mNumEntries)].mKey==0xffffffff)
            return FALSE; /*not found...*/
        
        if (This->mHashTable[c_(inKey, i, This->mNumEntries)].mKey==inKey)
            return TRUE; /*found....*/
     }
     return FALSE; /*not found...*/
}


/*---------------------------------------------------------------------------------
 *  GetItemByKey
 * ---------------------------------------------------------------------------------
 * Returns (if present) a pointer to the item with key inKey, NULL otherwise */
void* LHashStatic_GetItemByKey (LHashStatic* This, ui4 inKey)
{
    ui4 i;
     
    for (i = 0; i < This->mNumEntries; i++)
    {    
        if (This->mHashTable[c_(inKey, i, This->mNumEntries)].mKey==0xffffffff)
            return NULL; /*not found...*/
        
        if (This->mHashTable[c_(inKey, i, This->mNumEntries)].mKey==inKey)
            return This->mHashTable[c_(inKey, i, This->mNumEntries)].mItemRef; /*found....*/
    }
    return NULL; /*not found...*/
}
    
/*---------------------------------------------------------------------------------
 *  GetUsedMem
 * ---------------------------------------------------------------------------------
 *  */
ui4 LHashStatic_GetUsedMem (LHashStatic* This)
{ 
    return sizeof (LHashStatic) + ((This->mNumEntries)*sizeof (struct TSlot));
}

/*---------------------------------------------------------------------------------
 *  GetNumEntries
 * ---------------------------------------------------------------------------------
 *  */
ui4        LHashStatic_GetNumEntries    (LHashStatic* This)
{
    return This->mNumEntries;
}

/*---------------------------------------------------------------------------------
 *  GetAllItems
 * ---------------------------------------------------------------------------------
 *  */
LArray* LHashStatic_GetAllItems (LHashStatic* This)
{
    LArray *theRes;
    ui4 i;
    
    /*creates array of ui4...*/
    theRes=LArray_New (4);
    
    /*scans hash table...*/
    for (i=0; i<This->mNumEntries; i++)
    {
        /*if key present...*/
        if (This->mHashTable[i].mKey!=0xffffffff)
        {
            /*appends item ref...*/
            LArray_AppendItem (theRes, &(This->mHashTable[i].mItemRef));
        }
    }
    
    /*returns pointer to larray...*/
    return theRes;
}

/*---------------------------------------------------------------------------------
 *  GetAllKeys
 * ---------------------------------------------------------------------------------
 *  */
LArray* LHashStatic_GetAllKeys (LHashStatic* This)
{
    LArray *theRes;
    ui4 i;
    
    /*creates array of ui4...*/
    theRes = LArray_New (4);

    /*scans hash table...*/
    for (i = 0; i < This->mNumEntries; i++)
    {
        /*if key present...*/
        if (This->mHashTable[i].mKey!=0xffffffff)
        {
            /*appends it...*/
            LArray_AppendItem (theRes, &(This->mHashTable[i].mKey));
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
