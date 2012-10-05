/* =====================================================================
 *  mapper-testapp.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/03/03 16:36:23 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.9 $
*/


#include <stdio.h>
#include <stdlib.h>

#include "rm.h"
#include "dc.h"
#include "dc_profile.h"
#include "rlist.h"
#include "mapper.h"


#define MAX_VAL         5
#define MAKE_DUMP_FILE  1


// ---------------------------------------------------------------------
//  map
// ---------------------------------------------------------------------
int map(int val) { return 2*val; } 


// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(rlist* in, rlist* out, unsigned long long* count) {
    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;
    printf("in:  "); rlist_print(in);
    printf("out: "); rlist_print(out);
    printf("exec cons: %llu\n", new_count - *count);
    *count = new_count;
}


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {

    unsigned long long count = 0; // number of executed constraints

    #if MAKE_DUMP_FILE
    dc_init();
    rm_make_dump_file("logs/mapper-start.dump");
    #endif

    puts("--starting program");

    srand(13);

    rlist* in  = rlist_new_rand(10, MAX_VAL);
    rlist* out = rlist_new();
    printf("list length=%d\n", rlist_length(in));

    puts("--initial list in/out");
    dump(in, out, &count);

    puts("\n--making mapper");
    mapper* h = mapper_new(in, out, map);
    if (h == NULL) {
        printf("error: cannot initialize mapper\n");
        exit(0);
    }
    dump(in, out, &count);

    puts("\n--changing to 7 first item of in");
    rlist_set_val(rlist_first(in), 7);
    dump(in, out, &count);

    puts("\n--removing first item from in");
    rlist_remove_first(in);
    dump(in, out, &count);

    puts("\n--removing three items from in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_remove_first(in);
    rlist_remove_first(in);
    dc_end_at();
    dump(in, out, &count);

    puts("\n--adding two items to in (batch)");
    dc_begin_at();
    rlist_insert_first(in, 23);
    rlist_insert_first(in, 19);
    dc_end_at();
    dump(in, out, &count);

    puts("\n--adding/removing items to list in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_insert_first(in, 17);
    rlist_insert_first(in, 41);
    dc_end_at();
    dump(in, out, &count);

    puts("\n--individually removing three items past the fourth node of in");
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    dump(in, out, &count);

    puts("\n--disposing of mapper");
    mapper_delete(h);
    dump(in, out, &count);

    puts("\n--clearing input list");
    rlist_remove_all(in);
    dump(in, out, &count);

    puts("\n--clearing output list");
    rlist_remove_all(out);
    dump(in, out, &count);

    puts("\n--deleting output list");
    rlist_delete(out);

    puts("\n--deleting input list");
    rlist_delete(in);

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/mapper-end.dump");
    #endif

    return 0;
}


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
