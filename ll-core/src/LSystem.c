/* ============================================================================
 *  LSystem.c
 * ============================================================================

 *  Author:         (c) 2001 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 28, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/


#include "LSystem.h"
#include "LMemory.h"
#include "LArray.h"
#include "LString.h"
#include "LException.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __LL_DEBUG__

/* globals */
static LSystem_THandler sPrintH = NULL;
static LArray* sStack = NULL;

/* typedefs */
typedef struct {
    LSystem_THandler mPrintH;
    LArray* mBlock;
} _TStackEntry;

/* function prototypes */
static void _PrintH(const i1* inMsg);

/* ---------------------------------------------------------------------------------
 *  GetString
 * ---------------------------------------------------------------------------------
*/
void LSystem_GetString(i1* outStr, ui4 inSize){
    fgets(outStr,inSize,stdin);
    if (strlen(outStr)>0) outStr[strlen(outStr)-1]='\0';
}


/* ---------------------------------------------------------------------------------
 *  Print
 * ---------------------------------------------------------------------------------
*/
void LSystem_Print(const i1* inMsg, ...) {
    va_list theArgList;
    va_start(theArgList,inMsg);
    if (sPrintH==NULL)
        vfprintf(stderr,inMsg,theArgList);
    else {
        i1 theBuf[8096];
        vsprintf(theBuf,inMsg,theArgList);
        sPrintH(theBuf);
    }
    va_end(theArgList);
}


/* ---------------------------------------------------------------------------------
 *  Write
 * ---------------------------------------------------------------------------------
*/
void LSystem_Write(const i1* inMsg, ui4 inSize){
    if (sPrintH==NULL)
        fwrite(inMsg,1,inSize,stderr);
    else {
        i1* theBuf = (i1*)LMemory_Malloc(inSize+1);
        LMemory_Copy(inMsg,theBuf,inSize);
        theBuf[inSize]='\0';
        sPrintH(theBuf);
        LMemory_Free(&theBuf);
    }
}


/* ---------------------------------------------------------------------------------
 *  Exit
 * ---------------------------------------------------------------------------------
*/
void LSystem_Exit() {
    exit(1);
}


/* ---------------------------------------------------------------------------------
 *  InstallPrintHandler
 * ---------------------------------------------------------------------------------
*/
void LSystem_InstallPrintHandler(LSystem_THandler inHandler){
    sPrintH = inHandler;
}


/* ---------------------------------------------------------------------------------
 *  OpenBlock
 * ---------------------------------------------------------------------------------
*/
void LSystem_OpenBlock(){
    _TStackEntry theEntry = { 0 };
    Try {
        if (sStack==NULL) 
            sStack = LArray_New(sizeof(_TStackEntry));
        theEntry.mPrintH = sPrintH;
        theEntry.mBlock = LArray_New(1);
        LArray_AppendItem(sStack,&theEntry);
        sPrintH = (LSystem_THandler)_PrintH;
    }
    CatchAny {
        if (theEntry.mBlock!=NULL) LArray_Delete(&theEntry.mBlock);
        if (sStack!=NULL && LArray_GetItemsCount(sStack)==0)
            LArray_Delete(&sStack);
        Rethrow;
    }
}


/* ---------------------------------------------------------------------------------
 *  CloseBlock
 * ---------------------------------------------------------------------------------
*/
void LSystem_CloseBlock(i1** outBlock,ui4* outSize){
    ui4 theItemsCount;
    _TStackEntry theEntry;
    if (sStack==NULL) Throw(LSystem_INTERNAL_ERROR);
    theItemsCount = LArray_GetItemsCount(sStack);
    if (theItemsCount==0) Throw(LSystem_INTERNAL_ERROR);

    LArray_FetchItemAt(sStack,theItemsCount-1,&theEntry);
    LArray_RemoveLastItem(sStack);
    *outSize = LArray_GetItemsCount(theEntry.mBlock);
    *outBlock = (i1*)LMemory_Malloc(*outSize);
    LMemory_Copy(LArray_GetData(theEntry.mBlock),(void*)*outBlock,*outSize);
    LArray_Delete(&theEntry.mBlock);
    sPrintH = theEntry.mPrintH;

    if (theItemsCount==1) LArray_Delete(&sStack);
}

#else

void LSystem_GetString(i1* outStr, ui4 inSize){}
void LSystem_Print(const i1* inMsg, ...) {}
void LSystem_Exit() {}
void LSystem_InstallPrintHandler(LSystem_THandler inHandler){}
void LSystem_OpenBlock(){}
void LSystem_CloseBlock(i1** outBlock,ui4* outSize){ *outBlock=NULL; }

#endif

/* private functions */

static void _PrintH(const i1* inMsg){
    ui4 theCount, theMsgLen, thePrevBlockLen;
    _TStackEntry theEntry;
    if (sStack==NULL) Throw(LSystem_INTERNAL_ERROR);
    theCount = LArray_GetItemsCount(sStack);
    if (theCount==0) Throw(LSystem_INTERNAL_ERROR);
    LArray_FetchItemAt(sStack,theCount-1,&theEntry);
    theMsgLen = LString_Len(inMsg);
    thePrevBlockLen = LArray_GetItemsCount(theEntry.mBlock);
    LArray_ResizeBy(theEntry.mBlock,(i4)theMsgLen);
    LMemory_Copy(inMsg,
        (i1*)LArray_GetData(theEntry.mBlock)+thePrevBlockLen,
        theMsgLen);
}


/* Copyright (C) 2001 Camil Demetrescu

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
