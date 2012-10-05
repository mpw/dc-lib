/* =====================================================================
 *  mapper.h
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/01/31 08:42:56 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.2 $
*/


#ifndef __mapper__
#define __mapper__

#include "rlist.h"

// typdefs
typedef struct mapper_s mapper;
typedef int (*mapper_map_t)(int val);

// ---------------------------------------------------------------------
// mapper_new
// ---------------------------------------------------------------------
/** create new list mapper 
 *  of an input list into two output lists
 *  \param in reactive input list 
 *  \param out reactive output list
 *  \param map function that maps items of in to items of out
*/
mapper* mapper_new(rlist* in, rlist* out, mapper_map_t map);


// ---------------------------------------------------------------------
// mapper_delete
// ---------------------------------------------------------------------
/** delete list mapper
 *  \param mapper mapper object 
*/
void mapper_delete(mapper* m);

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
