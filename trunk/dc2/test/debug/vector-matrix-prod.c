// =====================================================================
//  dc/test/vector-matrix-prod.c
// =====================================================================

//  Author:         (C) 2011 Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        March 2, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/03/02 11:05:15 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.1 $


//#include <ctype.h>
//#include <string.h>
#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

#define MONITOR_UPDATES
#define TEST1

// Constants
#define R       6
#define C       10
#define Block   2  // Must be in [1,R]

// Types
typedef struct {
    dc_cons* cons;
    int i,j;
    int old_val;
} T;

// Reactive data structures
int* V;
int* O;
int **M;
T** cons;

// Constraint function
void productRule(T* obj) {
    int i;
    int j = obj->j;
    int old = obj->old_val;
    int new = 0;
    
    int r = (obj->i+1)*Block;
    if (r>R) r = R;
    for (i=obj->i*Block; i<r; i++) {
        volatile int a = V[i];
        volatile int b = M[i][j];
        new += a*b;
    }
    
    obj->old_val = new;
    dc_begin_no_log();
    O[j] = O[j]-old+new;
    dc_end_no_log();

    //O[j] = dc_inactive(O[j])-old+new;

    #ifdef MONITOR_UPDATES
    printf("Evaluating constraint <%d,%d>  ",obj->i,obj->j);
    printf("(matrix rows %d to %d)  ",obj->i*Block,r-1);
    printf("(O[%d]=%d)\n",obj->j, O[j]);
    #endif
}

int main() {
    int i,j;

    // Allocate reactive input vector
    V = (int*)dc_malloc(R*sizeof(int));
    if (!V) return -1;

    // Allocate reactive input matrix
    M = (int**)dc_malloc(R*sizeof(int*));
    if (!M) return -1;
    for (i=0; i<R; i++) {
        M[i] = (int*)dc_malloc(C*sizeof(int));
        if (!M[i]) return -1;
    }

    // Allocate reactive output vector
    O = (int*)dc_malloc(C*sizeof(int));
    if (!O) return -1;

    // Init V, M, and O
    // At the beginning, V contains 1 everywhere; M and O contain 0 
    // Constraint field old_val is set to 0, correspondingly
    for (i=0; i<R; i++) {
        V[i] = 1;
        for (j=0; j<C; j++) M[i][j] = 0;
    }
    for (j=0; j<C; j++) O[j] = 0;

    int cons_per_col = (R%Block==0) ? R/Block : R/Block+1;

    // Alloc and init constraint table
    cons = (T**)malloc(cons_per_col*sizeof(T*));
    if (!cons) return -1;
    for (i=0; i<cons_per_col; i++) {
        cons[i] = (T*)malloc(C*sizeof(T));
        if (!cons[i]) return -1;
        for (j=0; j<C; j++) {
            cons[i][j].i = i;
            cons[i][j].j = j;
            cons[i][j].old_val = 0;
            cons[i][j].cons = dc_new_cons((dc_cons_f)productRule, 
                                            &(cons[i][j]), NULL);
        }
    }    
 
    // Update matrix M
    #ifdef TEST1
    printf("Update M[0][0] = 10\n");
    M[0][0] = 10;
    printf("Update M[1][5] = 100\n");
    M[1][5] = 100;
    printf("Update M[4][5] = 200\n");
    M[4][5] = 200;
    //M[5][8] = 1000;
    #endif

    // Free constraints and data structures
    for (i=0; i<cons_per_col; i++) {
        for (j=0; j<C; j++) dc_del_cons(cons[i][j].cons);
        free(cons[i]);
    }
    free(cons);

    for (i=0; i<R; i++) dc_free(M[i]);
    dc_free(M);
    dc_free(O);
    dc_free(V);
    return 0;
}


// Copyright (C) 2011 Irene Finocchi

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
