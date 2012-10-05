// =====================================================================
//  dc/test/test02.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 11, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/13 09:23:50 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.1 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

typedef struct {
    int x, y, z;
} T;

void c(T* p) {
    printf("(x,y,z) = (%d,%d,%d)\n", p->x, p->y, p->z);
}

void d(T* p) {
    p->z = p->x + p->y;
}

int main() {

    dc_init();

    rm_make_dump_file("test02-start-"OPT".log");

    T* p = (T*)dc_malloc(sizeof(T));

    p->x = 1;

    dc_cons* cons_c = dc_new_cons((dc_cons_f)c, p, NULL);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_cons* cons_d = dc_new_cons((dc_cons_f)d, p, NULL);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);
    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_begin_at();

    p->x = 10;
    p->y = 7;

    dc_dump_wr_cell_list(stdout);

    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_end_at();

    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);

    p->x++;

    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);

    p->y-=5;

    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);

    p->x=50;

    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);

    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);

    dc_free(p);

    rm_make_dump_file("test02-end-"OPT".log");

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
