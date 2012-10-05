// =====================================================================
//  heap_sched.h
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu and Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        January 6, 2010
//  Module:         dc

//  Last changed:   $Date: 2012/09/25 14:45:00 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.4 $


#ifndef __heap_sched__
#define __heap_sched__

#include "dc.h"

// scheduler class
extern dc_scheduler_type* heap_sched_g;

// scheduler functions
void heap_set_comp(void* scheduler, int (*comp)(dc_cons*, dc_cons*));

#endif


// Copyright (C) 2011 Camil Demetrescu and Irene Finocchi

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
