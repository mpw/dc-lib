// =====================================================================
//  dc/test/test03.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 12, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/14 12:03:09 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.5 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

typedef struct {
    int x, y, z, w;
} T;

void c(T* p) {
    p->z = p->x + p->y;
}

void d(T* p) {
    p->w = 2*p->z;
}

void e(T* p) {
    printf("(z,w)=(%d,%d)\n", p->z, p->w);
}

int main() {

    dc_init();

    rm_make_dump_file("test03-start-"OPT".log");

    T* p = (T*)dc_malloc(sizeof(T));

    p->x = 1;

    dc_group* group = dc_new_group();
    dc_cons* cons_d = dc_new_cons((dc_cons_f)d, p, group);
    dc_cons* cons_c = dc_new_cons((dc_cons_f)c, p, group);
    dc_cons* cons_e = dc_new_cons((dc_cons_f)e, p, NULL);

    dc_dump_group(stdout, group);
    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);
    dc_dump_cons(stdout, cons_e);
    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);
    dc_dump_cell(stdout, &p->w);

    p->y = 5;

    dc_dump_group(stdout, group);
    dc_dump_cons(stdout, cons_c);
    dc_dump_cons(stdout, cons_d);
    dc_dump_cons(stdout, cons_e);
    dc_dump_cell(stdout, &p->x);
    dc_dump_cell(stdout, &p->y);
    dc_dump_cell(stdout, &p->z);
    dc_dump_cell(stdout, &p->w);

    dc_del_cons(cons_c);
    dc_del_cons(cons_d);
    dc_del_cons(cons_e);
    dc_del_group(group);
    dc_free(p);

    rm_make_dump_file("test03-end-"OPT".log");

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
