// =====================================================================
//  dc/test/fibonacci-testapp.c
// =====================================================================

//  Author:         (C) 2011 Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        January 22, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/22 20:53:02 $
//  Changed by:     $Author: irene $
//  Revision:       $Revision: 1.4 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

#define FIB_SIZE    10

typedef struct {
    dc_cons* cons;
    int index;
} T;

int* fib;
T* cons;

void c(T* obj) {
    int j=obj->index;
    fib[j]=fib[j-1]+fib[j-2];
}

int main() {
    
    int i;

    fib = (int*)dc_malloc(FIB_SIZE*sizeof(int));
    if (!fib) return -1;

    cons = (T*)malloc(FIB_SIZE*sizeof(T));
    if (!cons) return -1;

    for (i=2; i<FIB_SIZE; i++) {
        cons[i].index = i;
        cons[i].cons =
            dc_new_cons((dc_cons_f)c, &cons[i], NULL);
    }
    printf("\n");
    for (i=0; i<FIB_SIZE; i++) printf("fib[%d]=%d\n",i,fib[i]);

    fib[0]=fib[1]=1;
    printf("\n");
    for (i=0; i<FIB_SIZE; i++) printf("fib[%d]=%d\n",i,fib[i]);

    fib[1]=3;
    printf("\n");
    for (i=0; i<FIB_SIZE; i++) printf("fib[%d]=%d\n",i,fib[i]);

    dc_begin_at();
    fib[1]=2;
    fib[0]=2;
    dc_end_at();
    printf("\n");
    for (i=0; i<FIB_SIZE; i++) printf("fib[%d]=%d\n",i,fib[i]);

    for (i=2; i<FIB_SIZE; i++) dc_del_cons(cons[i].cons);
    free(cons);
    dc_free(fib);
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
