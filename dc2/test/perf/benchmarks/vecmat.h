/* =====================================================================
 *  vecmat.h
 * =====================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 3, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/03 14:34:19 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#ifndef __vecmat__
#define __vecmat__


// typdefs
typedef struct vecmat_s vecmat;

// ---------------------------------------------------------------------
// vecmat_new
// ---------------------------------------------------------------------
/** create new vector-matrix multiplier
 *  \param R vector length = matrix rows 
 *  \param C matrix columns
 *  \param B blocking parameter, must be in [1,R]. 
 *  With B=R there is one constraint per matrix column
 *  With B=1 there is one constraint per matrix entry
 *  \param V input vector 
 *  \param O output vector 
 *  \param M input matrix 
*/
vecmat* vecmat_new(int R, int C, int B, int *V, int *O, int **M);


// ---------------------------------------------------------------------
// vecmat_delete
// ---------------------------------------------------------------------
/** delete vector-matrix multiplier
 *  \param m vector-matrix multiplier object 
*/
void vecmat_delete(vecmat* m);

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
