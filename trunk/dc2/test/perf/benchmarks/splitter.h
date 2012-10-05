/* =====================================================================
 *  splitter.h
 * =====================================================================

 *  Author:         (C) 2010-2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 6, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/01/31 19:40:47 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.5 $
*/


#ifndef __splitter__
#define __splitter__

#include "rlist.h"

// typdefs
typedef struct splitter_s splitter;


// ---------------------------------------------------------------------
// splitter_new
// ---------------------------------------------------------------------
/** create new list splitter that partitions the input list, except the
 * first item, into two sublists where all items are smaller/larger 
 * than the value of the first item of the input list, which is used as 
 * a pivot.
 *  \param in input list
 *  \param small output list of items smaller than the pivot
 *  \param large output list of items larger or equal than the pivot
*/
splitter* splitter_new(rlist* in, rlist* small, rlist* large);


// ---------------------------------------------------------------------
// splitter_delete
// ---------------------------------------------------------------------
/** delete list splitter
 *  \param splitter splitter object 
*/
void splitter_delete(splitter* s);

#endif


/* Copyright (C) 2010-2011 Camil Demetrescu

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  
 * USA
*/
