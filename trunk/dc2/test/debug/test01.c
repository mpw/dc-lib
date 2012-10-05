// =====================================================================
//  dc/test/test01.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 6, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/13 09:23:50 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.1 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

void c(int* p) {
    printf("### executing cons: %p --> val = %d\n", 
        dc_get_curr_cons(), *p);
}

int main() {

    dc_init();

    rm_make_dump_file("test01-start-"OPT".log");

    int* p = (int*)dc_malloc(sizeof(int));
    int* q = (int*)dc_malloc(sizeof(int));
    
    dc_cons* cons_p = dc_new_cons((dc_cons_f)c, p, NULL);
    dc_cons* cons_q = dc_new_cons((dc_cons_f)c, q, NULL);

    dc_dump_cons(stdout, cons_p);
    dc_dump_cons(stdout, cons_q);

    dc_begin_at();

    *p = 32;

    dc_dump_cons(stdout, cons_p);
    dc_dump_cons(stdout, cons_q);
    dc_dump_cell(stdout, p);
    dc_dump_cell(stdout, q);

    *p = 27;
    *q = 16;

    dc_dump_wr_cell_list(stdout);

    dc_end_at();

    dc_dump_cons(stdout, cons_p);
    dc_dump_cons(stdout, cons_q);
    dc_dump_cell(stdout, p);
    dc_dump_cell(stdout, q);

    dc_free(p);
    dc_free(q);

    rm_make_dump_file("test01-end-"OPT".log");

    return 0;
}


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
