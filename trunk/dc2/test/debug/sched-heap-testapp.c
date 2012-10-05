// =====================================================================
//  dc/test/test_sched_heap.c
// =====================================================================

//  Author:         (C) 2010-2011 Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        December 31, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/01/15 08:02:17 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.1 $


#include "dc_defs.h"
#include "dc_sched_heap.h"
#include <stdio.h>

dc_scheduler* dc_sched_heap_g;

#define print_extract_result  {                                     \
        p = dc_sched_heap_pick(dc_sched_heap_g);                    \
        if (p!=NULL) printf("extracted min = %d\n",p->time_stamp);  \
        else printf("empty heap\n");                                \
        }

#define add_key(k)  {                                               \
        p = (struct dc_cons*)malloc(sizeof(struct dc_cons));        \
        p->time_stamp = k;                                          \
        dc_sched_heap_schedule(dc_sched_heap_g,p);                  \
        }

int main() {
    int i;
    struct dc_cons* p;
    
    dc_sched_heap_g = dc_sched_heap_create();
    
    print_extract_result;

    for (i=1; i<=40; i++) add_key((100-i)*10);

    print_extract_result;
    print_extract_result;

    for (i=0; i<=40; i++) print_extract_result;

    dc_sched_heap_destroy(dc_sched_heap_g);
    return 0;
}


// Copyright (C) 2010-2011 Irene Finocchi

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
