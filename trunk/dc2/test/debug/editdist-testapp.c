// =====================================================================
//  dc/test/editdist-testapp.c
// =====================================================================

//  Author:         (C) 2011 Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        January 22, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/23 15:48:40 $
//  Changed by:     $Author: irene $
//  Revision:       $Revision: 1.5 $


#include <string.h>
#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

#define PRINT_DP_TABLE
#define PRINT_UPDATE_TABLE

#define TEST2

// Constants
#define X_SIZE    6
#define Y_SIZE    6

// Types
typedef struct {
    dc_cons* cons;
    int i,j;
} T;

// Reactive data structures
int* X;
int* Y;
int X_len=0, Y_len=0;
int **M;
T** cons;

// Auxilary data structures to work with chars
char Xc[X_SIZE];
char Yc[Y_SIZE];

// Auxilary data structures to check which constraints are evaluated
int **updates;
int update_count;


// Constraint function
void dynProgRule(T* obj) {
    int i=obj->i;
    int j=obj->j;
    if (X[i-1]==Y[j-1]) M[i][j]=M[i-1][j-1];
    else {
        int tmp;
        if (M[i-1][j-1]<M[i][j-1]) tmp=M[i-1][j-1];
        else tmp=M[i][j-1];
        if (tmp>M[i-1][j]) tmp=M[i-1][j];
        M[i][j]=tmp+1;
    }
    updates[i][j]=++update_count;
    //printf("Evaluating constraint (%d,%d)\n",i,j);
}

// Clear update table
void clear_update_table() {
    int i,j;
    update_count=0;
    for (i=0; i<=X_SIZE; i++) 
        for (j=0; j<=Y_SIZE; j++) updates[i][j]=0;
}

// Print update table
void print_update_table() {
    #ifdef PRINT_UPDATE_TABLE
        int i,j;
        printf("Update table:\n");
        for (i=1; i<=X_SIZE; i++) {
            for (j=1; j<=Y_SIZE; j++) printf("%-2d ",updates[i][j]);
            printf("\n");
        }
    #endif
}

// Auxiliary print function
void print_DP_table() {
    printf("\nEdit distance (\"%s\",\"%s\") = %d\n",Xc, Yc, M[X_len][Y_len]);
    //printf("%c !!!!!!!!!!!",(char)X[0]);
    #ifdef PRINT_DP_TABLE
        int i,j;
        printf("DP table:\n");
        for (i=1; i<=X_SIZE; i++) {
            for (j=1; j<=Y_SIZE; j++) printf("%-2d ", M[i][j]);
            printf("\n");
        }
    #endif
}

// Update input strings and create/destroy table constraints
int string_update(char *sx, char *sy) {
    int i,j,lx,ly;

    clear_update_table();

    dc_begin_at();

    lx = strlen(sx);
    ly = strlen(sy);
    if (lx>X_SIZE || ly>Y_SIZE) return -1;

    // Update strings
    for (i=0; i<=lx; i++) {
        Xc[i]=sx[i];   
        X[i]=(int) sx[i];   
    }
    for (i=0; i<=ly; i++) {
        Yc[i]=sy[i];   
        Y[i]=(int) sy[i];
    }
    
    // Update constraints
    if (ly>Y_len) {
        for (i=1; i<=X_len; i++)
            for (j=Y_len+1; j<=ly; j++)
                cons[i][j].cons = dc_new_cons((dc_cons_f)dynProgRule, 
                                               &(cons[i][j]), NULL);
        Y_len=ly;
    }
    else if (ly<Y_len) {
        for (i=1; i<=X_len; i++)
            for (j=ly+1; j<=Y_len; j++) {
                dc_del_cons(cons[i][j].cons);
                M[i][j] = 0;
            }
        Y_len=ly;
    }

    if (lx>X_len) {
        for (i=X_len+1; i<=lx; i++)
            for (j=1; j<=Y_len; j++)
                cons[i][j].cons = dc_new_cons((dc_cons_f)dynProgRule, 
                                               &(cons[i][j]), NULL);
        X_len=lx;
    }
    else if (lx<X_len) {
        for (i=lx+1; i<=X_len; i++)
            for (j=1; j<=Y_len; j++) {
                dc_del_cons(cons[i][j].cons);
                M[i][j] = 0;
            }
        X_len=lx;
    }

    dc_end_at();

    print_DP_table();
    print_update_table();
    
    return 0;
}


int main() {
    int i,j;

    X = (int*)dc_malloc(X_SIZE*sizeof(int));
    if (!X) return -1;

    Y = (int*)dc_malloc(Y_SIZE*sizeof(int));
    if (!Y) return -1;

    // Allocate DP table (dynamic programming table)
    M = (int**)dc_malloc((X_SIZE+1)*sizeof(int*));
    if (!M) return -1;
    for (i=0; i<=X_SIZE; i++) {
        M[i] = (int*)dc_malloc((Y_SIZE+1)*sizeof(int));
        if (!M[i]) return -1;
    }

    // Allocate update table
    updates = (int**)malloc((X_SIZE+1)*sizeof(int*));
    if (!updates) return -1;
    for (i=0; i<=X_SIZE; i++) {
        updates[i] = (int*)malloc((Y_SIZE+1)*sizeof(int));
        if (!updates[i]) return -1;
        for (j=0; j<=Y_SIZE; j++) updates[i][j]=0;
    }

    // Init DP table
    for (i=0; i<=X_SIZE; i++) M[i][0]=i;
    for (j=0; j<=Y_SIZE; j++) M[0][j]=j;

    // Alloc and init constraint table
    cons = (T**)malloc((X_SIZE+1)*sizeof(T*));
    if (!cons) return -1;
    for (i=0; i<=X_SIZE; i++) {
        cons[i] = (T*)malloc((Y_SIZE+1)*sizeof(T));
        if (!cons[i]) return -1;
        if (i>0) for (j=1; j<=Y_SIZE; j++) {
            cons[i][j].i = i;
            cons[i][j].j = j;
            M[i][j] = 0;
        }
    }

    // Update input strings
    
    #ifdef TEST1
    if (string_update("aaaa","bbbb")==-1) return -1;
    if (string_update("aaaa","bbab")==-1) return -1;
    if (string_update("ac","bbab")==-1) return -1;
    if (string_update("dc","bbab")==-1) return -1;
    #endif

    #ifdef TEST2
    if (string_update("irene","iene")==-1) return -1;
    if (string_update("irene","iena")==-1) return -1;
    if (string_update("treno","iena")==-1) return -1;
    if (string_update("freno","cenare")==-1) return -1;
    if (string_update("re","cenare")==-1) return -1;
    #endif

    // Free constraints and data structures
    for (i=0; i<=X_len; i++) {
        for (j=1; j<=Y_len; j++) 
            if (i>0) dc_del_cons(cons[i][j].cons);
        free(cons[i]);
    }
    free(cons);
    for (i=0; i<=X_SIZE; i++) dc_free(M[i]);
    dc_free(M);
    dc_free(Y);
    dc_free(X);
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
