/* =====================================================================
 *  splitter-testapp.c
 * =====================================================================

 *  Author:         (c) 2010-2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 5, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/02/05 18:37:33 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.13 $
*/


#include <stdio.h>
#include <stdlib.h>
#include "dc.h"
#include "dc_inspect.h"
#include "dc_profile.h"
#include "rlist.h"
#include "splitter.h"

#define MAKE_DUMP_FILE  0

#ifdef LARGE_TEST
#define N         10000
#define MAX_VAL   1
#else
#define N         20
#define MAX_VAL   1000
#endif


// ---------------------------------------------------------------------
//  check_splitter
// ---------------------------------------------------------------------
int check_splitter(rlist* inl, rlist* out1l, rlist* out2l, int verb) {

    volatile rnode* p;
    rnode *in, *out1, *out2;
    int pivot = 0;
    
    in   = rlist_first(inl);
    out1 = rlist_first(out1l);
    out2 = rlist_first(out2l);

    if (in) pivot = rlist_val(in);

    // check if list lengths are correct
    if (rlist_length(inl) != 
            rlist_length(out1l) + 1 + rlist_length(out2l)) {
        printf("error: incorrect list lengths\n");
        return 0;
    }

    // check if all nodes in out1 are correct
    for (p = out1; p != NULL; p = p->next) {
        if (p->val > pivot) {
            printf("error: one item in small list is "
                   "larger than pivot\n");
            return 0;
        }
    }

    // check if all nodes in out2 are correct
    for (p = out2; p != NULL; p = p->next) {
        if (p->val < pivot) {
            printf("error: one item in small list is "
                   "larger than pivot\n");
            return 0;
        }
    }

    if (verb) printf("all checks ok\n");

    return 1;
}


// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(rlist* in, rlist* out1, rlist* out2, 
          unsigned long long* count) {

    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;

    printf("in:    "); rlist_print(in);
    printf("small: "); rlist_print(out1);
    if (rlist_first(in)) 
        printf("pivot: %d\n", rlist_val(rlist_first(in)));
    printf("large: "); rlist_print(out2);

    if (count == NULL) return;

    if (check_splitter(in, out1, out2, 1))
         puts("operation ok");
    else {
        puts("*** operation failed");
        exit(1);
    }

    printf("exec cons: %llu\n", new_count - *count);

    *count = new_count;
}


// ---------------------------------------------------------------------
//  check_profile
// ---------------------------------------------------------------------
void check_profile(rlist* in, rlist *out1, rlist *out2,
                   int* count, int verb) {
    dc_profile p;
    dc_fetch_profile_info(&p);
    unsigned long long new_count = p.num_exec_cons;
    if (verb) printf("exec cons: %llu\n", new_count - *count);
    *count = new_count;
    if (!check_splitter(in, out1, out2, verb)) exit(1);
}


// ---------------------------------------------------------------------
//  test_large
// ---------------------------------------------------------------------
void test_large(rlist* in, rlist *out1, rlist *out2,
                unsigned long long* count) {
    
    #if 0
    
    printf("--making initial list of %d items\n", N);
    in = rlist_new_rand(N, MAX_VAL);
    *out1 = *out2 = NULL;

    puts("--making splitter");
    splitter* h = splitter_new(in, out1, out2);
    if (h == NULL) {
        printf("error: cannot initialize splitter\n");
        exit(0);
    }

    check_profile(in, out1, out2, count, 1);

    puts("--making random operations on input list");
    size_t num_updates = 0;
    unsigned part_count = dc_get_num_exec_cons();
    rnode_t** head;
    for (head = in; *head != NULL; head = &(*head)->next) {
        int val = (*head)->val;
        //dc_begin_at();
        rlist_remove_first(head);
        check_profile(in, out1, out2, count, 1);
        rlist_add_first(head, val);
        check_profile(in, out1, out2, count, 1);
        //dc_end_at();
        num_updates+=2;
    }

    printf("exec cons: %u\n", dc_get_num_exec_cons() - part_count);
    printf("avg exec cons: %f\n", 
        (double)(dc_get_num_exec_cons() - part_count)/num_updates);

    check_profile(in, out1, out2, count, 1);

    puts("\n--disposing of splitter");
    splitter_delete(h);
    #endif
}


// ---------------------------------------------------------------------
//  test_small
// ---------------------------------------------------------------------
void test_small(unsigned long long* count) {

    rlist* in   = rlist_new_rand(N, MAX_VAL);
    rlist* out1 = rlist_new();
    rlist* out2 = rlist_new();

    puts("--initial list in");
    dump(in, out1, out2, NULL);

    puts("--making splitter");
    splitter* h = splitter_new(in, out1, out2);
    if (h == NULL) {
        printf("error: cannot initialize splitter\n");
        exit(0);
    }
    dump(in, out1, out2, count);

    puts("\n--adding 1000 as first item of in");
    rlist_insert_first(in, 1000);
    dump(in, out1, out2, count);

    puts("\n--changing to 400 first item of in");
    rlist_set_val(rlist_first(in), 400);
    dump(in, out1, out2, count);

    puts("\n--removing first item from in (pivot)");
    rlist_remove_first(in);
    dump(in, out1, out2, count);

    puts("\n--removing second item from in");
    rlist_remove_next(in, rlist_first(in));
    dump(in, out1, out2, count);

    puts("\n--removing three items from in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_remove_first(in);
    rlist_remove_first(in);
    dc_end_at();
    dump(in, out1, out2, count);

    puts("\n--adding four items {300, 450, 350, 900} to in past "
         "the third item (batch)");
    dc_begin_at();
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 300);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 450);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 350);
    rlist_insert_next(in, rlist_next(rlist_next(rlist_first(in))), 900);
    dc_end_at();
    dump(in, out1, out2, count);

    puts("\n--removing 1st item + adding {10,1} to in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_insert_first(in, 10);
    rlist_insert_first(in, 1);
    dc_end_at();
    dump(in, out1, out2, count);

    puts("\n--individually removing three items past the "
         "fourth node of in");
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    rlist_remove_next(in, 
        rlist_next(rlist_next(rlist_next(rlist_first(in)))));
    dump(in, out1, out2, count);

    puts("\n--incrementing by 10 first item of in");
    rlist_set_val(rlist_first(in), rlist_val(rlist_first(in)) + 10);
    dump(in, out1, out2, count);

    puts("\n--disposing of splitter");
    splitter_delete(h);

    puts("\n--disposing of lists");
    rlist_delete(in);
    rlist_delete(out1);
    rlist_delete(out2);
}


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {

    unsigned long long count = 0;  // number of executed constraints

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/splitter-start.dump");
    #endif

    srand(3);

    #ifndef LARGE_TEST
    test_small(&count);
    #else
    test_large(&count);
    #endif

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/splitter-end.dump");
    #endif

    return 0;
}


/* Copyright (C) 2010-2011 Camil Demetrescu

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
