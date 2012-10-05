/* ============================================================================
 *  rbfm.h
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        February 6, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/28 10:37:26 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.1 $
*/


#ifndef __rbfm__
#define __rbfm__

#include "LGraphUtil.h"

// typedefs
typedef struct rbfm_s rbfm;


// ---------------------------------------------------------------------
// rbfm_new
// ---------------------------------------------------------------------
/** initialize distances and install constraints.
 *  \param g input graph 
 *  \param edge_weights array of edge weights
 *  \param dist array of distances
*/
rbfm* rbfm_new (LGraph* g, ui4* edge_weights, ui4* dist);


// ---------------------------------------------------------------------
// rbfm_delete
// ---------------------------------------------------------------------
/** delete rbmf object
 *  \param obj rbmf object 
*/
void rbfm_delete (rbfm* obj);

#endif


/* Copyright (C) 2011 Camil Demetrescu, Andrea Ribichini

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
