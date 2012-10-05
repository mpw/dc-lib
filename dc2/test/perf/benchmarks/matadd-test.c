/* ============================================================================
 *  matadd-test.c
 * ============================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 3, 2011
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/03/05 18:23:16 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "dc.h"
#include "matadd.h"
#include "test-driver.h"
#include "rand_pm.h"

#define TEST    1

// specific test object
struct test_s {
    int     seed;
    size_t  num_updates;
    int     check;
    char*   input_family;
    char*   update_family;
    
    #if CONV || CHECK
    #endif

    int N;
    int **A;
    int **B;
    int **C;

    int **cA;
    int **cB;
    int **cC;

    matadd* m;
};


// ---------------------------------------------------------------------
// make_test
// ---------------------------------------------------------------------
test_t* make_test(size_t input_size, 
                  int seed, 
                  char* input_family, 
                  char* update_family, 
                  int check_correctness) {

    test_t* test = (test_t*)malloc(sizeof(test_t));
    if (test == NULL) return NULL;

    test->seed = seed;
    test->num_updates = 0;
    test->input_family = input_family;
    test->update_family = update_family;
    test->check = check_correctness;

    if (!cmp_param(input_family, "_", 0, "square")) return NULL;

    test->N = input_size;

    #if CONV || CHECK
    #endif

    return test;
}


// ---------------------------------------------------------------------
// del_test
// ---------------------------------------------------------------------
void del_test(test_t* test) {
    int i;

    if (test->m) matadd_delete(test->m);

    for (i=0; i<test->N; i++) {
        dc_free(test->A[i]);
        dc_free(test->B[i]);
        dc_free(test->C[i]);
        free(test->cA[i]);
        free(test->cB[i]);
        free(test->cC[i]);
    }

    dc_free(test->A);
    dc_free(test->B);
    dc_free(test->C);
    free(test->cA);
    free(test->cB);
    free(test->cC);

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
    return "matadd-dc";
}


// ---------------------------------------------------------------------
//  make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) {
    int i,j;

    // Allocate reactive input matrices
    test->A = (int**)dc_malloc(test->N*sizeof(int*));
    test->B = (int**)dc_malloc(test->N*sizeof(int*));
    test->C = (int**)dc_malloc(test->N*sizeof(int*));
    if (!test->A || !test->B || !test->C) 
        return "cannot initialize reactive input matrices";
    for (i=0; i<test->N; i++) {
        test->A[i] = (int*)dc_malloc(test->N*sizeof(int));
        test->B[i] = (int*)dc_malloc(test->N*sizeof(int));
        test->C[i] = (int*)dc_malloc(test->N*sizeof(int));
        if (!test->A[i] || !test->B[i] || !test->C[i]) 
            return "cannot initialize reactive input matrices";
    }

    // Init A and B: at the beginning, A and B contain 1 everywhere
    // The output matrix C is initialized in matadd_new
    for (i=0; i<test->N; i++) 
        for (j=0; j<test->N; j++) 
            test->A[i][j] = test->B[i][j] = 1;

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

    // makes matrix-matrix adder object
    test->m = matadd_new(test->N, test->A, test->B, test->C);
    if (test->m == NULL) return "can't build matrix-matrix adder";

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

    // perform update sequence on matrix M
    
    #if TEST==1
    int j, num_updates;

    srand(test->seed);

    if (cmp_param(test->update_family, "_", 0, "ABrnd")) {
        if (!get_int_param(test->update_family, "_", 1, &num_updates)) 
            return "wrong update parameter";

        // Perform random updates
        for (j=0; j<num_updates; j++) {
            int r = rand()%test->N;
            int c = rand()%test->N;
            int isA = rand()%2;
            if (isA) test->A[r][c]++;
            else test->B[r][c]++;
            test->num_updates++;
        }    
    }
    else return "unknown update family";

    #endif

     // make correctness checks
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
    int i,j;

    // Allocate non-reactive conventional input matrices and output matrix
    test->cA = (int**)malloc(test->N*sizeof(int*));
    test->cB = (int**)malloc(test->N*sizeof(int*));
    test->cC = (int**)malloc(test->N*sizeof(int*));
    if (!test->cA || !test->cB || !test->cC) 
        return "cannot initialize conventional matrices";
    for (i=0; i<test->N; i++) {
        test->cA[i] = (int*)malloc(test->N*sizeof(int));
        test->cB[i] = (int*)malloc(test->N*sizeof(int));
        test->cC[i] = (int*)malloc(test->N*sizeof(int));
        if (!test->cA[i] || !test->cB[i] || !test->cC[i]) 
            return "cannot initialize conventional input matrices";;
    }

    // Init A and B: at the beginning, A and B contain 1 everywhere
    // The output matrix C is computed by do_conv_eval
    for (i=0; i<test->N; i++)
        for (j=0; j<test->N; j++) 
            test->cA[i][j] = test->cB[i][j] = 1;

    // make checks
    #if CHECK
    if (test->check) {
    }
    #endif

    return NULL;
}


// ---------------------------------------------------------------------
// do_conv_eval
// ---------------------------------------------------------------------
char* do_conv_eval(test_t* test) {
    int i,j,k;
    for (i=0; i<test->N; i++)
        for (j=0; j<test->N; j++) 
            test->cC[i][k] = test->cA[i][j] + test->cB[i][j];
    return NULL;
}

#endif


/* Copyright (C) 2011 Irene Finocchi

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
 * USA
*/
