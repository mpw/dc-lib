/* ============================================================================
 *  qsorter-testapp.c
 * ============================================================================

 *  Author:         (C) 2010-2011 Camil Demetrescu, Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        October 28, 2010
 *  Module:         dc/test/debug

 *  Last changed:   $Date: 2011/02/06 14:21:30 $
 *  Changed by:     $Author: irene $
 *  Revision:       $Revision: 1.1 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "dc.h"
#include "dc_inspect.h"
#include "dc_profile.h"
#include "rm.h"
#include "rlist.h"
#include "qsorter.h"

// macros
#ifndef DUMP_SORTER
#define DUMP_SORTER 0
#endif

#ifndef TREE_DUMP
#define TREE_DUMP 1
#endif

#define MAKE_DUMP_FILE  1

#ifdef LARGE_TEST
    //#include "rand_pm.h"
    #define CHECK_CORRECTNESS 1
    #define UPDATE(val) ((val)+1)
    #define INS_REM   0         // update type: 1=insertions/removals
                                //              0=value changes
    #define N         100000    // size of input list
    #define MAX_VAL   10000000  // max node value in input list
#else
    #define CHECK_CORRECTNESS 1
    #define TEST_SMALL  1       // 2*N sequential remove + insert 
    //#define TEST_SMALL  2       // UPD random remove operations
    //#define TEST_SMALL  3       // UPD random insert operations

    #define N         50        // size of input list
    #define MAX_VAL   1000      // max node value in input list
    #define UPD       2        // number of updates (<= N)
#endif


// ---------------------------------------------------------------------
//  is_correct
// ---------------------------------------------------------------------
int is_correct(rlist* in, rlist* out) {
    if (rlist_length(in) != rlist_length(out)) return 0;
    rnode* p;
    for (p = rlist_first(out); p != NULL; p = rlist_next(p))
        if (rlist_next(p) != NULL && 
            rlist_val(p) > rlist_val(rlist_next(p))) return 0;
    return 1;
}


// ---------------------------------------------------------------------
//  check_correctness
// ---------------------------------------------------------------------
void check_correctness(rlist* in, rlist* out) {
    #if CHECK_CORRECTNESS
    int res = is_correct(in, out);
    printf("correctness test: %s\n", 
        res ? "[passed]" : ">> [failed] <<");
    if (!res) exit(1);
    #endif
}


// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(rlist* in, rlist* out, unsigned long long* count) {
    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;
    printf("in:  "); rlist_print(in);
    printf("out: "); rlist_print(out);
    //check_correctness(in, out);
    printf("exec cons: %llu\n", new_count - *count);
    *count = new_count;
}

// ---------------------------------------------------------------------
//  debugging_dump
// ---------------------------------------------------------------------
void debugging_dump(qsorter* h, int dump_sorter_flag, 
                    dc_profile *start, dc_profile *end) {
    int num_leaves, leaves_depth;

    #if DUMP_SORTER > 0
    if (dump_sorter_flag) dump_sorter(h);
    #endif
    #if TREE_DUMP > 0
    if (dump_sorter_flag) tree_dump(h);
    #endif
    dc_dump_profile_diff(stdout, start, end);
    printf("max sorter depth = %d\n",
            sorter_depth(h,&num_leaves,&leaves_depth));
    printf("avg sorter depth = %.3f\n",1.0*leaves_depth/num_leaves);
}


#if TEST_SMALL == 1
// ---------------------------------------------------------------------
//  test_small 1
// ---------------------------------------------------------------------
void test_small(unsigned long long* count) {

    dc_profile init, start, end;
    rlist* in  = rlist_new_rand(N, MAX_VAL);
    rlist* out = rlist_new();

    printf("list length=%d\n", rlist_length(in));

    puts("--initial list in/out");
    dump(in, out, count);

    puts("--making qsorter");
    qsorter* h = qsorter_new(in, out);
    if (h == NULL) {
        printf("error: cannot initialize qsorter\n");
        exit(0);
    }
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--making ins/rem operations on input list");
    size_t num_updates = 0, i = 1;
    dc_fetch_profile_info(&init);
    int val;

    printf("\n--remove item %u of %u\n", i, N);
    dc_fetch_profile_info(&start);
    val = rlist_val(rlist_first(in));
    rlist_remove_first(in);
    dc_fetch_profile_info(&end);
    debugging_dump(h, 1, &start, &end);
    check_correctness(in, out);

    printf("\n--re-insert %u of %u\n", i, N);
    dc_fetch_profile_info(&start);
    rlist_insert_first(in, val);
    dc_fetch_profile_info(&end);
    debugging_dump(h, 1, &start, &end);
    check_correctness(in, out);

    rnode *prev;
    i++;
    for (prev = rlist_first(in);
         prev != NULL && rlist_next(prev) != NULL;
         prev = rlist_next(prev), i++) {

        val = rlist_val(rlist_next(prev));

        printf("\n--remove item %u of %u\n", i, N);
        dc_fetch_profile_info(&start);
        rlist_remove_next(in, prev);
        dc_fetch_profile_info(&end);
        debugging_dump(h, 1, &start, &end);
        check_correctness(in, out);

        printf("\n--re-insert %u of %u\n", i, N);
        dc_fetch_profile_info(&start);
        rlist_insert_next(in, prev, val);
        dc_fetch_profile_info(&end);
        debugging_dump(h, 1, &start, &end);
        check_correctness(in, out);

        num_updates += 2;
    }

    dc_fetch_profile_info(&end);
    printf("\n--sequence report\n");
    debugging_dump(h, 0, &init, &end);
    check_correctness(in, out);

    #if 0
    printf("\n--adding %d to in\n", MAX_VAL/2);
    rlist_insert_first(in, MAX_VAL/2);
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    printf("\n--adding %d to in\n", MAX_VAL/3);
    rlist_insert_first(in, MAX_VAL/3);
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    printf("\n--adding %d to in as second item\n", 2*MAX_VAL/3);
    rlist_insert_next(in, rlist_first(in), 2*MAX_VAL/3);
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--removing first item from in");
    rlist_remove_first(in);
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--removing second item from in");
    rlist_remove_next(in, rlist_first(in));
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--removing second item from in");
    rlist_remove_next(in, rlist_first(in));
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif
    #endif

    puts("\n--disposing of qsorter");
    qsorter_delete(h);
    dump(in, out, count);

    puts("\n--deleting lists");
    rlist_delete(in);
    rlist_delete(out);
}
#endif

#if TEST_SMALL == 2
// ---------------------------------------------------------------------
//  test_small 2
// ---------------------------------------------------------------------
void test_small(unsigned long long* count) {

    dc_profile init, start, end;
    rlist* in  = rlist_new_rand(N, MAX_VAL);
    rlist* out = rlist_new();

    printf("list length=%d\n", rlist_length(in));

    puts("--initial list in/out");
    dump(in, out, count);

    puts("--making qsorter");
    qsorter* h = qsorter_new(in, out);
    if (h == NULL) {
        printf("error: cannot initialize qsorter\n");
        exit(0);
    }
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--making rnd remove operations on input list");
    dc_fetch_profile_info(&init);
    int val;

    rnode *prev;    
    size_t num_update, i, j;
    int updates[UPD];

    srand(17);
    for (i=0; i<UPD; i++) updates[i] = rand();
    
    for (num_update=0; num_update<UPD; num_update++) {
        i = updates[num_update] % rlist_length(in);
        
        if (i==0) {
            printf("\n--update %u: remove item %u of %u\n", 
                    num_update+1, i+1, rlist_length(in));
            dc_fetch_profile_info(&start);
            val = rlist_val(rlist_first(in));
            rlist_remove_first(in);
            dc_fetch_profile_info(&end);
            debugging_dump(h, 1, &start, &end);
            check_correctness(in, out);
            continue;
        }
 
        // i>0: update is not on the first item 
        prev = rlist_first(in);
        for (j=1; j<i; j++) prev = rlist_next(prev);

        val = rlist_val(rlist_next(prev));

        printf("\n--update %u: remove item %u of %u\n", 
                num_update+1, i+1, rlist_length(in));
        dc_fetch_profile_info(&start);
        rlist_remove_next(in, prev);
        dc_fetch_profile_info(&end);
        debugging_dump(h, 1, &start, &end);
        check_correctness(in, out);
    }

    dc_fetch_profile_info(&end);
    printf("\n--sequence report\n");
    debugging_dump(h, 0, &init, &end);
    check_correctness(in, out);

    puts("\n--disposing of qsorter");
    qsorter_delete(h);
    dump(in, out, count);

    puts("\n--deleting lists");
    rlist_delete(in);
    rlist_delete(out);
}
#endif

#if TEST_SMALL == 3
// ---------------------------------------------------------------------
//  test_small 3
// ---------------------------------------------------------------------
void test_small(unsigned long long* count) {

    dc_profile init, start, end;
    rlist* in  = rlist_new_rand(N, MAX_VAL);
    rlist* out = rlist_new();

    printf("list length=%d\n", rlist_length(in));

    puts("--initial list in/out");
    dump(in, out, count);

    puts("--making qsorter");
    qsorter* h = qsorter_new(in, out);
    if (h == NULL) {
        printf("error: cannot initialize qsorter\n");
        exit(0);
    }
    dump(in, out, count);
    #if DUMP_SORTER > 0
    dump_sorter(h);
    #endif

    puts("\n--making rnd insert operations on input list");
    dc_fetch_profile_info(&init);
    int val;

    rnode *prev;    
    size_t num_update, i, j;
    int updates[UPD];

    srand(17);
    for (i=0; i<UPD; i++) updates[i] = rand();
    
    for (num_update=0; num_update<UPD; num_update++) {
        i = updates[num_update] % rlist_length(in);
        
        if (i==0) {
            val = rand() % MAX_VAL;
            printf("\n--update %u: insert value %u in position %u\n", 
                    num_update+1, val, i+1);
            dc_fetch_profile_info(&start);
            rlist_insert_first(in, val);
            dc_fetch_profile_info(&end);
            debugging_dump(h, 1, &start, &end);
            check_correctness(in, out);
            continue;
        }
 
        // i>0: update is not on the first item 
        prev = rlist_first(in);
        for (j=1; j<i; j++) prev = rlist_next(prev);

        val = rand() % MAX_VAL;
        printf("\n--update %u: insert value %u in position %u\n", 
                num_update+1, val, i+1);
        dc_fetch_profile_info(&start);
        rlist_insert_next(in, prev, val);
        dc_fetch_profile_info(&end);
        debugging_dump(h, 1, &start, &end);
        check_correctness(in, out);
    }

    dc_fetch_profile_info(&end);
    printf("\n--sequence report\n");
    debugging_dump(h, 0, &init, &end);
    check_correctness(in, out);

    puts("\n--disposing of qsorter");
    qsorter_delete(h);
    dump(in, out, count);

    puts("\n--deleting lists");
    rlist_delete(in);
    rlist_delete(out);
}
#endif

#ifdef LARGE_TEST
// ---------------------------------------------------------------------
//  test_large
// ---------------------------------------------------------------------
void test_large(unsigned long long* count) {

    dc_profile init, start, end;

    printf("--making initial list of %d items\n", N);
    rlist* in  = rlist_new_rand(N, MAX_VAL);
    rlist* out = rlist_new();

    puts("--making qsorter");
    dc_fetch_profile_info(&init);

    qsorter* q = qsorter_new(in, out);
    if (q == NULL) {
        printf("error: cannot initialize qsorter\n");
        exit(0);
    }

    dc_fetch_profile_info(&end);
    debugging_dump(q, 0, &init, &end);
    check_correctness(in, out);

    puts("\n--making ins/rem operations on input list");
    size_t num_updates = 0;
    int val;

    dc_fetch_profile_info(&init);
    dc_fetch_profile_info(&start);

    val = rlist_val(rlist_first(in));
    printf("<r"); fflush(stdout);
    #if INS_REM == 1
    rlist_remove_first(in);
    #else
    rlist_set_val(rlist_first(in), UPDATE(val));
    #endif
    printf("><i"); fflush(stdout);
    #if INS_REM == 1
    rlist_insert_first(in, val);
    #else
    rlist_set_val(rlist_first(in), val);
    #endif
    printf(">"); fflush(stdout);
    num_updates += 2;

    printf("\n");
    dc_fetch_profile_info(&end);
    printf("last session report:\n");
    dc_dump_profile_diff(stdout, &start, &end);
    printf("all sessions report:\n");
    debugging_dump(q, 0, &init, &end);
    check_correctness(in, out);

    rnode *prev;
    for (prev = rlist_first(in);
         prev != NULL && rlist_next(prev) != NULL;
         prev = rlist_next(prev)) {

        dc_fetch_profile_info(&start);

        val = rlist_val(rlist_next(prev));
        printf("<r"); fflush(stdout);
        #if INS_REM == 1
        rlist_remove_next(in, prev);
        #else
        rlist_set_val(rlist_next(prev), UPDATE(val));
        #endif
        printf("><i"); fflush(stdout);
        #if INS_REM == 1
        rlist_insert_next(in, prev, val);
        #else
        rlist_set_val(rlist_next(prev), val);
        #endif
        printf(">"); fflush(stdout);

        printf("\n");
        dc_fetch_profile_info(&end);
        printf("last session report:\n");
        dc_dump_profile_diff(stdout, &start, &end);
        printf("all sessions report:\n");
        debugging_dump(q, 0, &init, &end);
        check_correctness(in, out);

        num_updates += 2;
    }

    puts("\n--disposing of qsorter");
    qsorter_delete(q);

    puts("\n--deleting lists");
    rlist_delete(in);
    rlist_delete(out);
}
#endif


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {

    unsigned long long count = 0; // number of executed constraints

    puts("--starting program");
    
    dc_init();

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/qsorter-start.dump");
    #endif

    srand(131);
    
    #ifndef LARGE_TEST
    test_small(&count);
    #else
    test_large(&count);
    #endif

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/qsorter-end.dump");
    #endif

	printf ("--total number of cached instructions executed = %u\n", 
        g_stats.exec_cache_instr_count);

    return 0;
}


/* Copyright (C) 2011 Camil Demetrescu, Irene Finocchi

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
