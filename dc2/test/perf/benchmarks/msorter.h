/* ============================================================================
 *  msorter.h
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 6, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/06 13:42:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.3 $
*/


#ifndef __msorter__
#define __msorter__

#include "rlist.h"

// typedefs
typedef struct msorter_s msorter;


// ---------------------------------------------------------------------
// msorter_new
// ---------------------------------------------------------------------
/** create new sorter that maintains a sorted version of an input list.
 *  The algorithm maintains a sorting network based on the problem
 *  decomposition made by the classical mergesort algorithm.
 *  \param in input list 
 *  \param out output list
*/
msorter* msorter_new(rlist* in, rlist* out);


// ---------------------------------------------------------------------
// msorter_delete
// ---------------------------------------------------------------------
/** delete list msorter
 *  \param msorter msorter object 
*/
void msorter_delete(msorter* m);


// ---------------------------------------------------------------------
// msorter_dump
// ---------------------------------------------------------------------
/** dump msorter to stdout
 *  \param msorter msorter object 
*/
void msorter_dump(msorter* m);


// ---------------------------------------------------------------------
// tree_dump
// ---------------------------------------------------------------------
/** dump recursion tree to stdout
 *  \param msorter msorter object 
*/
void msorter_tree_dump(msorter* m);


// ---------------------------------------------------------------------
//  sorter_depth
// ---------------------------------------------------------------------
/** compute depth of the recursion tree associated with msorter object
 *  \param msorter msorter object 
 *  \param num_leaves counter of the number of tree leaves 
 *  \param leaves_depth sum of depths of tree leaves 
*/
int msorter_depth(msorter* s, int *num_leaves, int *leaves_depth);


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
