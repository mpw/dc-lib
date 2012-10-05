/* =====================================================================
 *  joiner-testapp.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 1, 2011
 *  Module:         dc/test/debug

 *  Last changed:   $Date: 2011/02/05 18:37:33 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.6 $
*/


#include <stdio.h>
#include <stdlib.h>

#include "dc.h"
#include "dc_profile.h"
#include "rm.h"
#include "rlist.h"
#include "joiner.h"


#define MAX_VAL         5
#define MAKE_DUMP_FILE  0


// ---------------------------------------------------------------------
//  check
// ---------------------------------------------------------------------
int check(rlist* in1l, rlist* in2l, rnode** sep, rlist* outl) {

    rnode *in1, *in2, *out;

    in1 = rlist_first(in1l);
    in2 = rlist_first(in2l);
    out = rlist_first(outl);

    while (out != NULL && in1 != NULL) {
        if (out->val != in1->val) return 0;
        out = out->next;
        in1 = in1->next;
    }

    if (out != NULL) {
        if (out->val != (*sep)->val) return 0;
        out = out->next;
    }

    while (out != NULL && in2 != NULL) {
        if (out->val != in2->val) return 0;
        out = out->next;
        in2 = in2->next;
    }

    return out == NULL && in1 == NULL && in2 == NULL;
}


// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(rlist* in1, rlist* in2, rlist* out, rnode** sep, 
          unsigned long long* count) {

    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;

    printf("in1:  "); rlist_print(in1);
    printf("sep: %d\n", rlist_val(*sep));
    printf("in2:  "); rlist_print(in2);
    printf("out: ");  rlist_print(out);
    printf("exec cons: %llu\n", new_count - *count);
    printf("correctness: %s\n", 
        check(in1, in2, sep, out) ? "ok" : "***failed***");

    *count = new_count;
}


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {

    unsigned long long count = 0; // number of executed constraints

    puts("--starting program");

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/joiner-start.dump");
    #endif

    srand(1371);

    rlist* in1 = rlist_new_rand(10, MAX_VAL);
    rlist* in2 = rlist_new_rand(20, MAX_VAL);
    rlist* out = rlist_new();
    rlist* sep = rlist_new();
    rlist_insert_first(sep, 1000);

    puts("--initial list in/out");
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("--making joiner");
    joiner* h = joiner_new(in1, &rlist_first(sep), in2, out);
    if (h == NULL) {
        printf("error: cannot initialize joiner\n");
        exit(0);
    }
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--adding first item to in1");
    rlist_insert_first(in1, 55);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--adding first item to in2");
    rlist_insert_first(in2, 77);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--changing first item of in1");
    rlist_set_val(rlist_first(in1), 10);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--changing first item of in2");
    rlist_set_val(rlist_first(in2), 15);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--appending 90 at the end of in1");
    rnode* last = rlist_last(in1);
    rlist_insert_next(in1, last, 90);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--removing last node of in1");
    rlist_remove_next(in1, last);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--removing first item from in2");
    rlist_remove_first(in2);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--adding first item to in2");
    rlist_insert_first(in2, 40);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--removing three items from in1 (batch)");
    dc_begin_at();
    rlist_remove_first(in1);
    rlist_remove_first(in1);
    rlist_remove_first(in1);
    dc_end_at();
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--clearing list in1");
    rlist_remove_all(in1);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--clearing list in2");
    rlist_remove_all(in2);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--adding 5 to list in2");
    rlist_insert_first(in2, 5);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--changing first item of in2");
    rlist_set_val(rlist_first(in2), 10+rlist_val(rlist_first(in2)));
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--adding/removing items to list in1 (batch)");
    dc_begin_at();
    rlist_remove_first(in2);
    rlist_insert_first(in2, rand()%MAX_VAL);
    rlist_insert_first(in2, rand()%MAX_VAL);
    dc_end_at();
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--disposing of joiner");
    joiner_delete(h);
    dump(in1, in2, out, &rlist_first(sep), &count);

    puts("\n--clearing input lists in1 and in2");
    rlist_delete(in1);
    rlist_delete(in2);
    rlist_delete(out);
    rlist_delete(sep);

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/joiner-end.dump");
    #endif

	printf ("--total number of cached instructions executed = %u\n", 
        g_stats.exec_cache_instr_count);

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
