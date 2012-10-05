/* =====================================================================
 *  matmat.h
 * =====================================================================

 *  Author:         (C) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 4, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/13 16:47:07 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.3 $
*/


#ifndef __matmat__
#define __matmat__


// typdefs
typedef struct matmat_s matmat;


// ---------------------------------------------------------------------
// matmat_new
// ---------------------------------------------------------------------
/** create new square matrix multiplier
 *  \param n = matrix rows = matrix cols
 *  \param A input matrix 1
 *  \param B input matrix 2
 *  \param C output matrix 
*/
matmat* matmat_new(int n, int **A, int **B, int **C);


// ---------------------------------------------------------------------
// matmat_delete
// ---------------------------------------------------------------------
/** delete matrix-matrix multiplier
 *  \param m matrix-matrix multiplier object 
*/
void matmat_delete(matmat* m);

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
