/* =====================================================================
 *  matmat.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        March 8, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/03/13 16:47:07 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.3 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "matmat.h"
#include "dc.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif


// cons object
typedef struct {
    dc_cons *cons_a, *cons_b;
    matmat* m;
    int i;
} cons_t;


// matmat object
struct matmat_s {
    int n, **A, **B, **C;
    cons_t *cons;
};


// private functions
static void panic(char* msg);
void cons_arow(cons_t* obj);
void cons_bcol(cons_t* obj);


// ---------------------------------------------------------------------
//  cons_arow
// ---------------------------------------------------------------------
void cons_arow(cons_t* p) {
    int j;
    for (j = 0; j < p->m->n; j++) {
        int k, c = 0;
        for (k = 0; k < p->m->n; k++) {
            volatile int a = p->m->A[p->i][k];
            volatile int b = dc_inactive(p->m->B[k][j]);
            c += a * b;
        }
        p->m->C[p->i][j] = c;
    }
}


// ---------------------------------------------------------------------
//  cons_bcol
// ---------------------------------------------------------------------
void cons_bcol(cons_t* p) {
    int i;
    for (i = 0; i < p->m->n; i++) {
        int k, c = 0;
        for (k = 0; k < p->m->n; k++) {
            volatile int a = dc_inactive(p->m->A[i][k]);
            volatile int b = p->m->B[k][p->i];
            c += a * b;
        }
        p->m->C[i][p->i] = c;
    }
}


// ---------------------------------------------------------------------
// matmat_new
// ---------------------------------------------------------------------
matmat* matmat_new(int n, int **A, int **B, int **C) {

    int i;
    matmat* m = malloc(sizeof(matmat));
    if (m == NULL) panic("out of memory");

    m->n = n;
    m->A = A;
    m->B = B;
    m->C = C;
    m->cons = (cons_t*)malloc(m->n * sizeof(cons_t));
    if (m->cons == NULL) panic("out of memory");

    for (i = 0; i < n; i++) {
        m->cons[i].m = m;
        m->cons[i].i = i;
        m->cons[i].cons_a = dc_is_reactive(A[0]) ? 
            dc_new_cons((dc_cons_f)cons_arow, &m->cons[i], NULL) : NULL;
        m->cons[i].cons_b = dc_is_reactive(B[0]) ? 
            dc_new_cons((dc_cons_f)cons_bcol, &m->cons[i], NULL) : NULL;
    }

    return m;
}


// ---------------------------------------------------------------------
// matmat_delete
// ---------------------------------------------------------------------
void matmat_delete(matmat* m) {
    int i;
    for (i = 0; i < m->n; i++) {
        if (m->cons[i].cons_a != NULL) dc_del_cons(m->cons[i].cons_a);
        if (m->cons[i].cons_b != NULL) dc_del_cons(m->cons[i].cons_b);
    }
    free(m->cons);
    free(m);
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
void panic(char* msg) {
    fprintf(stderr, "[matmat] %s\n", msg);
    exit(1);
}


/* Copyright (C) 2011 Camil Demetrescu

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
