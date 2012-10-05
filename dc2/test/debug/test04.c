// =====================================================================
//  dc/test/test04.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 12, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/13 21:43:25 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.3 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

void c(int* x) {
    printf("halving *x from %d to %d\n", *x, *x / 2);
    *x = *x / 2;
}

int main() {

    dc_init();

    rm_make_dump_file("test04-start-"OPT".log");

    int* x = (int*)dc_malloc(sizeof(int));

    printf("*x = 32\n");
    *x = 32;

    dc_group* group;
    dc_cons* cons;

    group = dc_new_group();
    dc_set_self_trigger(group, 1);
    cons = dc_new_cons((dc_cons_f)c, x, group);

    dc_dump_group(NULL, group);
    dc_dump_cons(NULL, cons);

    printf("*x = 65\n");
    *x = 65;

    dc_dump_group(NULL, group);
    dc_dump_cons(NULL, cons);

    dc_free(x);

    rm_make_dump_file("test04-end-"OPT".log");

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
