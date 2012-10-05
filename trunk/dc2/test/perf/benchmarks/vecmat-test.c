/* =====================================================================
 *  vecmat-test.c
 * =====================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 3, 2011
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/03/09 12:38:52 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.8 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "dc.h"
#include "vecmat.h"
#include "test-driver.h"


// specific test object
struct test_s {
    int     seed;
    size_t  num_updates;
    int     check;
    char*   input_family;
    char*   update_family;
    
    #if CONV || CHECK
    #endif

    int R, C, Block;
    int* V;
    int* O;
    int **M;

    int* cV;
    int* cO;
    int **cM;

    vecmat* m;
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

    if (cmp_param(input_family, "_", 0, "square")) {
        if (!get_int_param(input_family, "_", 1, &test->R)) 
            return NULL;
        test->C = test->R;
    }
    else return NULL;

    test->Block = input_size;

    #if CONV || CHECK
    #endif

    return test;
}


// ---------------------------------------------------------------------
// del_test
// ---------------------------------------------------------------------
void del_test(test_t* test) {
    int i;

    if (test->m) vecmat_delete(test->m);

    for (i=0; i<test->R; i++) dc_free(test->M[i]);
    free(test->M);
    free(test->V);
    dc_free(test->O);

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
    return "vecmat-dc";
}


// ---------------------------------------------------------------------
//  make_updatable_input
// ---------------------------------------------------------------------
char* make_updatable_input(test_t* test) {
    int i,j;

    // allocate non-reactive input vector
    test->V = (int*)malloc(test->R*sizeof(int));
    if (!test->V) return "cannot initialize input vector";

    // allocate input matrix with normal rows vector and reactive rows
    test->M = (int**)malloc(test->R*sizeof(int*));
    if (!test->M) return "cannot initialize input matrix";
    for (i=0; i<test->R; i++) {
        test->M[i] = (int*)dc_malloc(test->C*sizeof(int));
        if (!test->M[i]) return "cannot initialize input matrix";
    }

    // allocate reactive output vector
    test->O = (int*)dc_malloc(test->C*sizeof(int));
    if (!test->O) return "cannot initialize output vector";

    // init V and M: at the beginning, V and M contain 1 everywhere;
    // the output vector O is initialized in vecmat_new
    for (i=0; i<test->R; i++) {
        test->V[i] = 1;
        for (j=0; j<test->C; j++) test->M[i][j] = 1;
    }

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

    // makes vector-matrix multiplier object
    test->m = vecmat_new(test->R, test->C, test->Block, 
                         test->V, test->O, test->M);
    if (test->m == NULL) return "can't build vector-matrix multiplier";

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
    int i, j, num_updates;

    srand(test->seed);

    // updates of individual random cells of the matrix
    if (cmp_param(test->update_family, "_", 0, "mrnd")) {
        if (!get_int_param(test->update_family, "_", 1, &num_updates)) 
            return "wrong update parameter";

        // perform random updates
        for (j=0; j<num_updates; j++) {
            int r = rand()%test->R;
            int c = rand()%test->C;
            test->M[r][c]++;
            test->num_updates++;
        }    
    }

    // updates of entire random columns of the matrix
    else if (cmp_param(test->update_family, "_", 0, "crnd")) {
        if (!get_int_param(test->update_family, "_", 1, &num_updates)) 
            return "wrong update parameter";

        // perform random updates
        for (j=0; j<num_updates; j++) {

            int c = rand()%test->C;

            dc_begin_at();
            for (i=0; i<test->R; i++) test->M[i][c]++;
            dc_end_at();
            test->num_updates++;
        }    
    }
    else return "unknown update family";
   
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

    // allocate non-reactive conventional input vector
    test->cV = (int*)malloc(test->R*sizeof(int));
    if (!test->cV) return "cannot initialize conventional input vector";

    // allocate non-reactive conventional input matrix
    test->cM = (int**)malloc(test->R*sizeof(int*));
    if (!test->cM) return "cannot initialize conventional input matrix";
    for (i=0; i<test->R; i++) {
        test->cM[i] = (int*)malloc(test->C*sizeof(int));
        if (!test->cM[i]) 
            return "cannot initialize conventional input matrix";
    }

    // allocate non-reactive conventional output vector
    test->cO = (int*)malloc(test->C*sizeof(int));
    if (!test->cO) 
        return "cannot initialize conventional output vector";

    // init V and M: at the beginning, V and M contain 1 everywhere;
    // the output vector O is computed by do_conv_eval
    for (i=0; i<test->R; i++) {
        test->cV[i] = 1;
        for (j=0; j<test->C; j++) test->cM[i][j] = 1;
    }

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

    int i,j;

    for (i=0; i<test->C; i++) test->cO[i]=0;

    for (i=0; i<test->C; i++)
        for (j=0; j<test->R; j++) 
            test->cO[i] += test->cV[j]*test->cM[j][i];

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  
 * USA
*/
