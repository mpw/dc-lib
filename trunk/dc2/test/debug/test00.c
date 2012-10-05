// =====================================================================
//  dc/test/test00.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 31, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/01/13 09:23:49 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.1 $


#include "dc.h"
#include "dc_inspect.h"
#include "rm.h"
#include <stdio.h>

int main() {

    dc_init();

    rm_make_dump_file("test00-start-"OPT".log");

    int* p = (int*)dc_malloc(sizeof(int));

    dc_begin_at();

    dc_dump_cell(stdout, p);

    *p = 10;        // write reactive cell

    dc_dump_cell(stdout, p);

    dc_end_at();

/*
    int x = *p;     // read reactive cell

    printf("x = %d\n", x);
*/

    //dc_free(p);

    //rm_make_dump_file("test00-end-"OPT".log");

    return 0;
}


// Copyright (C) 2010-2011 Camil Demetrescu

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
