/* ============================================================================
 *  rand_pm.h
 * ============================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 13, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/01/23 10:41:38 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#ifndef __rand_pm__
#define __rand_pm__

static unsigned int __seed_pm = 1;

#define rand_pm() (__seed_pm = (__seed_pm * 16807) % 2147483647)

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
