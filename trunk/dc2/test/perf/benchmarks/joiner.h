/* ============================================================================
 *  joiner.h
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 11, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/01 20:45:41 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.2 $
*/


#ifndef __joiner__
#define __joiner__

#include "rlist.h"

// typdefs
typedef struct joiner_s joiner;


// ---------------------------------------------------------------------
// joiner_new
// ---------------------------------------------------------------------
/** create new list joiner that concatenates two input lists, separated 
 *  by one item, into a new output list - the input lists and the 
 *  separator are not changed.
 *  @param in1 input list 1
 *  @param mid head of a node holding the separator item
 *  @param in2 input list 2
 *  @param output list
*/
joiner* joiner_new(rlist* in1, rnode** mid, rlist* in2, rlist* out);


// ---------------------------------------------------------------------
// joiner_delete
// ---------------------------------------------------------------------
/** delete list joiner
 *  @param joiner joiner object 
*/
void joiner_delete(joiner* joiner);

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
