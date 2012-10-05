/* ============================================================================
 *  sorter-test.c
 * ============================================================================

 *  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        February 7, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/08 14:26:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "dc.h"
#include "list.h"
#include "rlist.h"
#include "rlist_updates.h"
#include "test-driver.h"


#ifndef INS_REM
#define INS_REM 1
#endif

#if !defined(QSORTER) && !defined(MSORTER)
#define QSORTER
#endif

#if defined(MSORTER)
#include "msorter.h"
typedef msorter sorter;
#define _sorter(fun) msorter_##fun
#define _sorter_name "msorter"

#elif defined(QSORTER)
#include "qsorter.h"
typedef qsorter sorter;
#define _sorter(fun) qsorter_##fun
#define _sorter_name "qsorter"
#endif

// specific test object
struct test_s {
    size_t  input_size;
    int     seed;
    size_t  num_updates;
    int     check;
    char*   input_family;
    char*   update_family;
    
    #if CONV || CHECK
    #endif

    rlist*  in;
    rlist*  out;

    sorter* m;
};


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
// make_test
// ---------------------------------------------------------------------
test_t* make_test(size_t input_size, int seed, 
                  char* input_family, char* update_family,
                  int check_correctness) {

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

    _sorter(delete)(test->m);
    rlist_delete(test->in);
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
    return _sorter_name"-dc";
}


// ---------------------------------------------------------------------
//  make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) {

    // initialize random generator
    srand(test->seed);

    // allocate reactive lists
    test->in  = rlist_new_rand(test->input_size, INT_MAX);
    test->out = rlist_new();

    if (test->in == NULL || test->out == NULL) 
        return "cannot initialize lists";

    // make checks
    #if CHECK
    if (test->check) {
    }
    #endif

    return NULL;
}


// ---------------------------------------------------------------------
//  do_from_scratch_eval
// ---------------------------------------------------------------------
char* do_from_scratch_eval(test_t* test) {

    // make sorter
    test->m = _sorter(new)(test->in, test->out);
    if (test->m == NULL) return "can't build sorter";

    // make checks
    #if CHECK
    if (test->check) {
        if (!is_correct(test->in, test->out))
            return "correctness test failed";
    }
    #endif

    return NULL;
}


// ---------------------------------------------------------------------
// do_updates
// ---------------------------------------------------------------------
char* do_updates(test_t* test) {

    // perform update sequence
    test->num_updates = 
        rlist_updates(test->in, test->update_family, NULL, NULL);

    // make correctness checks
    #if CHECK
    if (test->check) {
        if (!is_correct(test->in, test->out))
            return "correctness test failed";
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


/* Copyright (C) 2010-2011 Camil Demetrescu, Andrea Ribichini

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
