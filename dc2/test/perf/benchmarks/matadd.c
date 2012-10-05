/* =====================================================================
 *  matadd.c
 * =====================================================================

 *  Author:         (c) 2011 Irene Finocchi
 *  License:        See the end of this file for license information
 *  Created:        March 5, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/05 18:23:16 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "matadd.h"
#include "dc.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// constraint object
typedef struct {
    dc_cons* cons;
    int i, j;
    matadd *m;
} T;

// matadd object
struct matadd_s {
    int N;
    int **A;
    int **B;
    int **C;
    T** cons;
};

// private functions
static void panic(char* msg);
static void cons(T* obj);


// ---------------------------------------------------------------------
// matadd_new
// ---------------------------------------------------------------------
matadd* matadd_new(int N, int **A, int **B, int **C) {

    matadd* m = malloc(sizeof(matadd));
    if (m == NULL) panic("out of memory");
    
    m->N = N;
    m->A = A;
    m->B = B;
    m->C = C;    
    
    // Alloc and init constraint table
    int i,j;
    m->cons = (T**)malloc(N*sizeof(T*));
    if (!m->cons) return NULL;
    for (i=0; i<N; i++) {
        m->cons[i] = (T*)malloc(N*sizeof(T));
        if (!(m->cons[i])) return NULL;
        for (j=0; j<N; j++) {
            m->cons[i][j].i = i;
            m->cons[i][j].j = j;
            m->cons[i][j].m = m;
            #if DEBUG > 0
                printf("[matadd] creating constraint <%d,%d>  \n",
                    m->cons[i][j].i, m->cons[i][j].j);
            #endif
            m->cons[i][j].cons = dc_new_cons((dc_cons_f)cons, &(m->cons[i][j]), NULL);
        }
    }    
 
    return m;
}


// ---------------------------------------------------------------------
// matadd_delete
// ---------------------------------------------------------------------
void matadd_delete(matadd* m) {
    // Free constraint table
    int i,j;
    for (i=0; i<m->N; i++) {
        for (j=0; j<m->N; j++) dc_del_cons(m->cons[i][j].cons);
        free(m->cons[i]);
    }
    free(m->cons);
    free(m);
}



// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
void panic(char* msg) {
    fprintf(stderr, "[matadd] %s\n", msg);
    exit(1);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
// Constraint function
void cons(T* obj) {
    int i = obj->i;
    int j = obj->j;
    
    #if DEBUG > 0
    printf("[vecmat] evaluating constraint <%d,%d>  ", i, j);
    #endif

    obj->m->C[i][j] = obj->m->A[i][j] + obj->m->B[i][j];
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
