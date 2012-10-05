/* =====================================================================
 *  halver-testapp.c
 * =====================================================================

 *  Author:         (c) 2010 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 5, 2011
 *  Module:         dc/test/debug

 *  Last changed:   $Date: 2011/02/05 18:46:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.4 $
*/


#include <stdio.h>
#include <stdlib.h>
#include "dc.h"
#include "dc_profile.h"
#include "rlist.h"
#include "halver.h"

#define MAX_VAL         100
#define MAKE_DUMP_FILE  0


int check_halve(rlist* in, rlist* out1, rlist* out2) {
    if (rlist_length(in) != rlist_length(out1)+rlist_length(out2))
        return 0;
    return 1;
}

void dump(rlist* in, rlist* out1, rlist* out2, 
          unsigned long long* count) {

    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;

    printf("in, out1, out2:\n");
    rlist_print(in);
    rlist_print(out1);
    rlist_print(out2);

    if (check_halve(in, out1, out2))
         puts("operation ok");
    else puts("*** operation failed");

    printf("exec cons: %llu\n", new_count - *count);
    *count = new_count;
}


int main() {

    unsigned long long count = 0; // number of executed constraints

    rlist *in   = rlist_new_rand(20, MAX_VAL);
    rlist *out1 = rlist_new();
    rlist *out2 = rlist_new();

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/halver-start.dump");
    #endif

    srand(11);

    puts("--initial list in");
    dump(in, out1, out2, &count);

    puts("--making halver");
    halver* h = halver_new(in, out1, out2);
    if (h == NULL) {
        printf("error: cannot initialize halver\n");
        exit(0);
    }
    dump(in, out1, out2, &count);

    puts("\n--adding 1000 as first item of in");
    rlist_insert_first(in, 1000);
    dump(in, out1, out2, &count);

    puts("\n--changing to 400 first item of in");
    rlist_set_val(rlist_first(in), 400);
    dump(in, out1, out2, &count);

    puts("\n--removing first item from in");
    rlist_remove_first(in);
    dump(in, out1, out2, &count);

    puts("\n--removing second item from in");
    rlist_remove_next(in, rlist_first(in));
    dump(in, out1, out2, &count);

    puts("\n--removing three items from in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_remove_first(in);
    rlist_remove_first(in);
    dc_end_at();
    dump(in, out1, out2, &count);

    puts("\n--adding four items {300, 450, 350, 900} to in past "
         "the third item (batch)");
    dc_begin_at();
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 300);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 450);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 350);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 900);
    dc_end_at();
    dump(in, out1, out2, &count);

    puts("\n--removing 1st item + adding {10,1} to in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_insert_first(in, 10);
    rlist_insert_first(in, 1);
    dc_end_at();
    dump(in, out1, out2, &count);

    puts("\n--individually removing three items past "
         "the fourth node of in");
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    dump(in, out1, out2, &count);

    puts("\n--incrementing by 10 first item of in");
    rlist_set_val(rlist_first(in), rlist_val(rlist_first(in)) + 10);
    dump(in, out1, out2, &count);

    puts("\n--deleting halver");
    halver_delete(h);

    puts("\n--deleting input list");
    rlist_delete(in);
    rlist_delete(out1);
    rlist_delete(out2);

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/halver-end.dump");
    #endif

	printf ("total number of cached instructions "
            "executed so far = %u\n", g_stats.exec_cache_instr_count);

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
