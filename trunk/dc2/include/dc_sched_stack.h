// =====================================================================
//  dc/include/dc_sched_stack.h
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 6, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 13:56:57 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.5 $


#ifndef __dc_sched_stack__
#define __dc_sched_stack__

#include "dc.h"

#ifdef __cplusplus
extern "C" {
#endif

// scheduler class
extern dc_scheduler_type* dc_sched_stack_g;

// scheduler functions
void*    dc_sched_stack_create   ();
int      dc_sched_stack_schedule (void* scheduler, dc_cons* cons);
dc_cons* dc_sched_stack_pick     (void* scheduler);
void     dc_sched_stack_destroy  (void* scheduler);

#ifdef __cplusplus
}
#endif
#endif


// Copyright (C) 2011 Camil Demetrescu

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
