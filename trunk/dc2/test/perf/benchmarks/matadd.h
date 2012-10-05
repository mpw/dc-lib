/* =====================================================================
 *  matmat.h
 * =====================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 4, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/05 18:23:16 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#ifndef __matmat__
#define __matmat__


// typdefs
typedef struct matadd_s matadd;

// ---------------------------------------------------------------------
// matadd_new
// ---------------------------------------------------------------------
/** create new matrix-matrix adder
 *  \param N = matrix rows = matrix cols
 *  \param A input matrix 1 
 *  \param B input matrix 2 
 *  \param C output matrix 
*/
matadd* matadd_new(int N, int **A, int **B, int **C);


// ---------------------------------------------------------------------
// matadd_delete
// ---------------------------------------------------------------------
/** delete matrix-matrix adder
 *  \param m matrix-matrix adder object 
*/
void matadd_delete(matadd* m);

#endif


/* Copyright (C) 2011 Irene Finocchi

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
