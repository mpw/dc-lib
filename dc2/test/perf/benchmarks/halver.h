/* =====================================================================
 *  halver.h
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 5, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/05 18:37:33 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.2 $
*/


#ifndef __halver__
#define __halver__

#include "rlist.h"

// typdefs
typedef struct halver_s halver;


// ---------------------------------------------------------------------
// halver_new
// ---------------------------------------------------------------------
/** create new randomized list halver that maintains a balanced partition 
 *  of an input list into two output lists
 *  \param in input list 
 *  \param out1 output list 1
 *  \param out2 output list 2
*/
halver* halver_new(rlist* in, rlist* out1, rlist* out2);


// ---------------------------------------------------------------------
// halver_delete
// ---------------------------------------------------------------------
/** delete list halver
 *  \param halver halver object 
*/
void halver_delete(halver* h);

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
