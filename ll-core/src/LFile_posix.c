/* ============================================================================
 *  LFile_posix.c
 * ============================================================================

 *  Author:         (c) 2001 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 29, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2007/03/16 11:40:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.4 $
*/

#include "LConfig.h"

#if __LL_OS_CORE__ == __LL_POSIX__

#include <stdio.h>
#include <unistd.h>
#include "LFile.h"
#include "LException.h"
#include "LMemory.h"
#include "LSystem.h"


/* MEMBER VARIABLES */
struct LFile {
    FILE* File;
};

/* MACROS */
#define OpenMode_(x) ((x==LFile_READ)? "rb" : (x==LFile_WRITE)? "wb" : "rb+")
#define SeekMode_(x) ((x==LFile_START)? SEEK_SET : (x==LFile_CURR)? SEEK_CUR : SEEK_END)


/* ----------------------------------------------------------------------------
 *  Open
 * ----------------------------------------------------------------------------
*/
LFile* LFile_Open(const i1* inFileName, LFile_TOpenMode inMode){

    LFile theObject;

    /* Create file if READ_WRITE mode and file does not exist */
    if (inMode==LFile_READ_WRITE) {
        if (!LFile_Exists(inFileName)){
            FILE* theTempFile;
            theTempFile = fopen64(inFileName,"wb");
            if (theTempFile==NULL ) {
                Throw(LFile_CANT_OPEN_FILE);
            }
            fclose(theTempFile);
        }
    }

    /* Open file */
    theObject.File = fopen64(inFileName, OpenMode_(inMode));
    if (theObject.File==NULL) {
        Throw(LFile_CANT_OPEN_FILE);
    }

    return (LFile*)LMemory_NewObject(LFile,theObject);
}


/* ----------------------------------------------------------------------------
 *  Close
 * ----------------------------------------------------------------------------
*/
void LFile_Close(LFile** ThisA){
    fclose((*ThisA)->File);
    LMemory_DeleteObject(ThisA);
}


/* ----------------------------------------------------------------------------
 *  Write
 * ----------------------------------------------------------------------------
*/
void LFile_Write(LFile* This, const void* inData, ui4 inSize){
    if (fwrite(inData,1,inSize,This->File)!=inSize) Throw(LFile_IO_ERROR);
}


/* ----------------------------------------------------------------------------
 *  Read
 * ----------------------------------------------------------------------------
*/
ui4 LFile_Read(LFile* This, void* outData, ui4 inSize){
    return (ui4)fread(outData,1,inSize,This->File);
}

    
/* ----------------------------------------------------------------------------
 *  Seek
 * ----------------------------------------------------------------------------
*/
Bool LFile_Seek(LFile* This, i4 inOffset, LFile_TSeekMode inMode){
    return (Bool)!fseek(This->File,inOffset,SeekMode_(inMode));
}


/* ----------------------------------------------------------------------------
 *  Tell
 * ----------------------------------------------------------------------------
*/
ui4 LFile_Tell(LFile* This){
    long int theOffset=ftell(This->File);
    if (theOffset==-1L) Throw(LFile_IO_ERROR);
    return (ui4)theOffset;
}

/* ----------------------------------------------------------------------------
 *  GetSize
 * ----------------------------------------------------------------------------
*/
ui4 LFile_GetSize(LFile* This){
    ui4 theFileSize;
    ui4 theCurrPos = ftell(This->File);
    fseek(This->File,0,SEEK_END);
    theFileSize = ftell(This->File);
    fseek(This->File,theCurrPos,SEEK_SET);
    return theFileSize;
}

/* ----------------------------------------------------------------------------
 *  GetSize64
 * ----------------------------------------------------------------------------
*/
ui8 LFile_GetSize64(LFile* This){
	ui8 theFileSize;
	ui8 theCurrPos = ftello(This->File);
	fseeko(This->File,0,SEEK_END);
	theFileSize = ftello(This->File);
	fseeko(This->File,theCurrPos,SEEK_SET);
	return theFileSize;
}

/* ----------------------------------------------------------------------------
 *  Exists
 * ----------------------------------------------------------------------------
*/
Bool LFile_Exists(const i1* inFileName){
    FILE* theFile = fopen(inFileName,"rb");
    if (theFile==NULL) return FALSE;
    fclose(theFile);
    return TRUE;
}


/* ----------------------------------------------------------------------------
 *  Remove
 * ----------------------------------------------------------------------------
*/
void LFile_Remove(const i1* inFileName){
    #if !defined(LEONARDO) && !defined(__LVM__)
    if (remove(inFileName)) Throw(LFile_CANT_REMOVE_FILE);
    #endif
}


/* ----------------------------------------------------------------------------
 *  Rename
 * ----------------------------------------------------------------------------
*/
void LFile_Rename(const i1* inOldFileName, const i1* inNewFileName){
    #if !defined(LEONARDO) && !defined(__LVM__)
    if (rename(inOldFileName,inNewFileName)) Throw(LFile_CANT_RENAME_FILE);
    #endif
}


/* ----------------------------------------------------------------------------
 *  GetTempName
 * ----------------------------------------------------------------------------
*/
void LFile_GetTempName(i1 outFileName[LFile_MAX_PATHNAME_LEN]){
    #if !defined(LEONARDO) && !defined(__LVM__)
    if (L_tmpnam>LFile_MAX_PATHNAME_LEN) Throw(LSystem_INTERNAL_ERROR);
    tmpnam(outFileName);
    #endif
}

#endif


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
