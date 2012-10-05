/* ============================================================================
 *  merger-test.c
 * ============================================================================

 *  Author:         (c) 2010 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        October 13, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/04/05 21:19:24 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/


#include <stdio.h>
#include <string.h>

#include "dc.h"
#include "list.h"
#include "rlist_updates.h"
#include "merger.h"
#include "test-driver.h"

#ifndef INS_REM
#define INS_REM 1
#endif

// maximum difference between consecutive values in the input list
#define MAX_DELTA   20

// size of a = input_size - input_size*SPLIT_RATIO
// size of b = input_size*SPLIT_RATIO
#define SPLIT(x,y)      ((double)(x)/((x)+(y)))
#define SPLIT_A(n,x,y)  ((size_t)((n)*SPLIT(x,y)))
#define SPLIT_B(n,x,y)  (((n)-(size_t)((n)*SPLIT(x,y))))


// specific test object
struct test_s {
    size_t      input_size;
    int         seed;
    size_t      num_updates;
    int         check;
    char*       input_family;
    char*       update_family;
    
    #if CONV || CHECK
    #endif

    rlist*   in1;
    rlist*   in2;
    rlist*   out;

    merger*  m;
};


// ---------------------------------------------------------------------
// make_test
// ---------------------------------------------------------------------
test_t* make_test(size_t input_size, int seed, 
                 char* input_family, char* update_family, int check_correctness) {

    test_t* test = (test_t*)malloc(sizeof(test_t));
    if (test == NULL) return NULL;

    test->input_size = input_size;
    test->seed = seed;
    test->num_updates = 0;
    test->input_family = input_family;
    test->update_family = update_family;
    test->check = check_correctness;

    #if CONV || CHECK
    #endif

    return test;
}


// ---------------------------------------------------------------------
// del_test
// ---------------------------------------------------------------------
void del_test(test_t* test) {

    merger_delete(test->m);

    rlist_delete(test->in1);
    rlist_delete(test->in2);
    rlist_delete(test->out);

    #if CONV || CHECK
    #endif

    free(test);
}


// ---------------------------------------------------------------------
// get_num_updates
// ---------------------------------------------------------------------
size_t get_num_updates(test_t* test) {
    return test->num_updates;
}


// ---------------------------------------------------------------------
// get_test_name
// ---------------------------------------------------------------------
char* get_test_name() {
    return "merger-dc";
}


// ---------------------------------------------------------------------
// make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) {

    int side_a, side_b;

    // initialize random generator
    srand(test->seed);

    if (!strcmp("1-1", test->input_family)) 
        side_a = side_b = 1;
    else if (!strcmp("1-2", test->input_family)) 
        side_a = 1, side_b = 2;
    else if (!strcmp("1-3", test->input_family)) 
        side_a = 1, side_b = 3;
    else return "unknown family type";

    // create random sorted lists with size ratio side_a:side_b
    test->in1 = 
        rlist_new_sorted_rand(
            SPLIT_A(test->input_size, side_a, side_b), MAX_DELTA);
    test->in2 = 
        rlist_new_sorted_rand(
            SPLIT_B(test->input_size, side_a, side_b), MAX_DELTA);
    test->out=rlist_new();
    if (test->in1 == NULL || test->in2 == NULL || test->out==NULL) 
        return "can't create lists";

    // make checks
    #if CHECK
    if (test->check) {
    }
    #endif

    return NULL;
}


// ---------------------------------------------------------------------
// do_from_scratch_eval
// ---------------------------------------------------------------------
char* do_from_scratch_eval(test_t* test) {

    // make merger
    test->m = merger_new(test->in1, test->in2, test->out);
    if (test->m == NULL) return "can't build merger";

    // make checks
    #if CHECK
    if (test->check) {
    }
    #endif

    return NULL;
}


// ---------------------------------------------------------------------
// do_updates
// ---------------------------------------------------------------------
char* do_updates(test_t* test) {

    test->num_updates = 0;
    test->num_updates += rlist_updates(test->in1, test->update_family, NULL, NULL);
    test->num_updates += rlist_updates(test->in2, test->update_family, NULL, NULL);

    // make checks
    #if CHECK
    if (test->check) {
    }
    #endif

    return NULL;
}


#if CONV || CHECK

// ---------------------------------------------------------------------
// make_conv_input
// ---------------------------------------------------------------------
char* make_conv_input(test_t* test) {
    return NULL;
}


// ---------------------------------------------------------------------
// do_conv_eval
// ---------------------------------------------------------------------
char* do_conv_eval(test_t* test) {
    return NULL;
}

#endif


/* Copyright (C) 2010 Camil Demetrescu

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
