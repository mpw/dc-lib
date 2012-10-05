/* =====================================================================
 *  merger-testapp.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 1, 2011
 *  Module:         dc/test/debug

 *  Last changed:   $Date: 2011/02/07 12:11:22 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.4 $
*/


#include <stdio.h>
#include <stdlib.h>

#include "dc.h"
#include "dc_profile.h"
#include "rm.h"
#include "rlist.h"
#include "merger.h"


#define MAX_DELTA       30
#define MAKE_DUMP_FILE  0


// ---------------------------------------------------------------------
//  is_correct
// ---------------------------------------------------------------------
int is_correct(rlist* in1, rlist* in2, rlist* out) {
    if (rlist_length(in1) + rlist_length(in2) != 
        rlist_length(out)) return 0;
    rnode* p;
    for (p = rlist_first(out); p != NULL; p = rlist_next(p))
        if (rlist_next(p) != NULL && 
            rlist_val(p) > rlist_val(rlist_next(p))) return 0;
    return 1;
}


// ---------------------------------------------------------------------
//  check_correctness
// ---------------------------------------------------------------------
void check_correctness(rlist* in1, rlist* in2, rlist* out) {
    int res = is_correct(in1, in2, out);
    printf("correctness test: %s\n", 
        res ? "[passed]" : "[**failed**]");
    //if (!res) exit(1);
}


// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(rlist* in1, rlist* in2, rlist* out,
          unsigned long long* count) {

    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;

    printf("in1:  "); rlist_print(in1);
    printf("in2:  "); rlist_print(in2);
    printf("out: ");  rlist_print(out);
    printf("exec cons: %llu\n", new_count - *count);
    check_correctness(in1, in2, out);

    *count = new_count;
}


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {

    unsigned long long count = 0; // number of executed constraints

    puts("--starting program");

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/merger-start.dump");
    #endif

    srand(1371);

    rlist* in1 = rlist_new_sorted_rand(15, MAX_DELTA);
    rlist* in2 = rlist_new_sorted_rand(15, MAX_DELTA);
    rlist* out = rlist_new();

    puts("--initial list in/out");
    dump(in1, in2, out, &count);

    puts("--making merger");
    merger* m = merger_new(in1, in2, out);
    if (m == NULL) {
        printf("error: cannot initialize merger\n");
        exit(0);
    }
    dump(in1, in2, out, &count);

    puts("\n--adding item in front of list 1");
    rlist_insert_first(in1, rlist_val(rlist_first(in1))-1);
    dump(in1, in2, out, &count);

    puts("\n--adding item in front of list 2");
    rlist_insert_first(in2, rlist_val(rlist_first(in2))-1);
    dump(in1, in2, out, &count);

    puts("\n--adding item at tail of list 1");
    rlist_insert_next(in1, rlist_last(in1), 
                      rlist_val(rlist_last(in1))+1);
    dump(in1, in2, out, &count);

    puts("\n--removing second item of list 2");
    rlist_remove_next(in2, rlist_first(in2));
    dump(in1, in2, out, &count);    

    puts("\n--adding item at tail of list 2");
    rlist_insert_next(in2, rlist_last(in2), 
                      rlist_val(rlist_last(in2))+1);
    dump(in1, in2, out, &count);

    puts("\n--adding item as second node of list 1");
    rlist_insert_next(in1, rlist_first(in1), 
        (rlist_val(rlist_first(in1)) + 
         rlist_val(rlist_next(rlist_first(in1)))) / 2);
    dump(in1, in2, out, &count);

    puts("\n--removing first item of list 1");
    rlist_remove_first(in1);
    dump(in1, in2, out, &count);    

    puts("\n--disposing of merger");
    merger_delete(m);
    dump(in1, in2, out, &count);

    puts("\n--clearing input lists in1 and in2");
    rlist_delete(in1);
    rlist_delete(in2);
    rlist_delete(out);

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/merger-end.dump");
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
