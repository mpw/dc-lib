/* =====================================================================
 *  merger.h
 * =====================================================================

 *  Author:         (c) 2010 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 6, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/06 13:42:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.2 $
*/


#ifndef __merger__
#define __merger__

#include "rlist.h"

// typedefs
typedef struct merger_s merger;


// ---------------------------------------------------------------------
// merger_new
// ---------------------------------------------------------------------
/** create new reactive list merger that merges two sorted input lists 
 *  into one sorted output list
 *  \param a sorted input list 1
 *  \param b sorted input list 2
 *  \param out sorted output list
*/
merger* merger_new(rlist* a, rlist* b, rlist* out);


// ---------------------------------------------------------------------
// merger_delete
// ---------------------------------------------------------------------
/** delete list merger
 *  \param m merger object 
*/
void merger_delete(merger* m);

#endif


/* Copyright (C) 2011 Camil Demetrescu

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
