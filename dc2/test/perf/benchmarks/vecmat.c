/* =====================================================================
 *  vecmat.c
 * =====================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 3, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/09 12:38:52 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.6 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "vecmat.h"
#include "dc.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// constraint object
typedef struct {
    dc_cons* cons;
    int min_i, max_i, j;
    int old_val;
    vecmat *m;
} T;

// vecmat object
struct vecmat_s {
    int R, C, Block;
    int* V;
    int* O;
    int **M;
    T** cons;
};

// private functions
static void panic(char* msg);
static void cons(T* obj);


// ---------------------------------------------------------------------
// vecmat_new
// ---------------------------------------------------------------------
vecmat* vecmat_new(int R, int C, int B, int *V, int *O, int **M) {

    vecmat* m = malloc(sizeof(vecmat));
    if (m == NULL) panic("out of memory");
    
    m->R = R;
    m->C = C;
    m->Block = B;
    m->V = V;
    m->O = O;
    m->M = M;
    
    int i,j;
    for (i=0; i<C; i++) O[i]=0;
    
    int cons_per_col = (R%B==0) ? R/B : R/B+1;

    // alloc and init constraint table
    m->cons = (T**)malloc(cons_per_col*sizeof(T*));
    if (!m->cons) return NULL;
    for (i=0; i<cons_per_col; i++) {
        m->cons[i] = (T*)malloc(C*sizeof(T));
        if (!(m->cons[i])) return NULL;
        for (j=0; j<C; j++) {
            m->cons[i][j].min_i = i*B;
            m->cons[i][j].max_i = -1 + ((i+1)*B>R ? R : (i+1)*B);
            m->cons[i][j].j = j;
            m->cons[i][j].m = m;
            m->cons[i][j].old_val = 0;
            #if DEBUG > 0
                printf("[vecmat] creating constraint <(%d,%d),%d>  \n",
                    m->cons[i][j].min_i, 
                    m->cons[i][j].max_i, 
                    m->cons[i][j].j);
            #endif
            m->cons[i][j].cons = 
            dc_new_cons((dc_cons_f)cons, &(m->cons[i][j]), NULL);
        }
    }    
 
    return m;
}


// ---------------------------------------------------------------------
// vecmat_delete
// ---------------------------------------------------------------------
void vecmat_delete(vecmat* m) {
    int i,j;
    int cons_per_col = 
        (m->R%m->Block==0) ? m->R/m->Block : m->R/m->Block+1;

    // Free constraint table
    for (i=0; i<cons_per_col; i++) {
        for (j=0; j<m->C; j++) dc_del_cons(m->cons[i][j].cons);
        free(m->cons[i]);
    }
    free(m->cons);

    free(m);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
// Constraint function
void cons(T* obj) {
    int i;
    int j = obj->j;
    int old = obj->old_val;
    int new = 0;
    
    #if DEBUG > 0
    dc_begin_no_log();
    printf("[vecmat] evaluating constraint <i=[%d,%d], j=%d, val=%d>  ",
            obj->min_i, obj->max_i, obj->j, old);
    dc_end_no_log();
    #endif

    for (i=obj->min_i; i<=obj->max_i; i++) {
        volatile int a = obj->m->V[i];
        volatile int b = obj->m->M[i][j];
        new += a*b;
    }
    
    obj->old_val = new;

    dc_begin_no_log();
    obj->m->O[j] = obj->m->O[j] - old + new;

    #if DEBUG > 0
    printf("(O[%d]=%d)\n",obj->j, obj->m->O[j]);
    #endif
    dc_end_no_log();
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
void panic(char* msg) {
    fprintf(stderr, "[vecmat] %s\n", msg);
    exit(1);
}


/* Copyright (C) 2011 Irene Finocchi

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
 * USA
*/
