/* ============================================================================
 *  qsorter.h
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 1, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/06 13:42:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.6 $
*/


#ifndef __qsorter__
#define __qsorter__

#include "rlist.h"

// typedefs
typedef struct qsorter_s qsorter;


// ---------------------------------------------------------------------
// qsorter_new
// ---------------------------------------------------------------------
/** create new sorter that maintains a sorted version of an input list.
 *  The algorithm maintains a sorting network based on the problem
 *  decomposition made by the classical quicksort algorithm.
 *  \param in input list 
 *  \param out output list
*/
qsorter* qsorter_new(rlist* in, rlist* out);


// ---------------------------------------------------------------------
// qsorter_delete
// ---------------------------------------------------------------------
/** delete list qsorter
 *  \param qsorter qsorter object 
*/
void qsorter_delete(qsorter* q);


// ---------------------------------------------------------------------
// qsorter_dump
// ---------------------------------------------------------------------
/** dump qsorter to stdout
 *  \param qsorter qsorter object 
*/
void qsorter_dump(qsorter* q);


// ---------------------------------------------------------------------
// qsorter_tree_dump
// ---------------------------------------------------------------------
/** dump recursion tree to stdout
 *  \param qsorter qsorter object 
*/
void qsorter_tree_dump(qsorter* q);


// ---------------------------------------------------------------------
//  qsorter_depth
// ---------------------------------------------------------------------
/** compute depth of the recursion tree associated with qsorter object
 *  \param qsorter qsorter object 
 *  \param num_leaves counter of the number of tree leaves 
 *  \param leaves_depth sum of depths of tree leaves 
*/
int qsorter_depth(qsorter* s, int *num_leaves, int *leaves_depth);


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
