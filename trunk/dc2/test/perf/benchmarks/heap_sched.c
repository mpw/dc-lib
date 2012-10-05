// =====================================================================
//  heap_sched.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        April 2, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 17:41:55 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.7 $


#include "heap_sched.h"
#include <stdio.h>

#define INIT_SIZE 4
#define DEBUG     0

// prototypes
static void*    heap_create   ();
static int      heap_schedule (void* scheduler, dc_cons* cons);
static dc_cons* heap_pick     (void* scheduler);
static void     heap_destroy  (void* scheduler);

// setup scheduler class structure
static dc_scheduler_type _sched_s = {
    heap_create,
    heap_schedule,
    heap_pick,
    heap_destroy
};

// export pointer to structure
dc_scheduler_type* heap_sched_g = &_sched_s;


typedef struct binheap {
    dc_cons** v;
    unsigned n;
    unsigned max;
    int (*comp)(dc_cons*, dc_cons*);
} binheap;


#if DEBUG
// ---------------------------------------------------------------------
//  dump
// ---------------------------------------------------------------------
void dump(binheap* b) {
    int i;
    printf("heap dump (n=%u, max=%u): ", b->n, b->max);
    for (i = 1; i <= b->n; i++) printf("%d ", (unsigned)b->v[i]);
    printf("\n");
}
#endif


// ---------------------------------------------------------------------
//  heap_create
// ---------------------------------------------------------------------
void* heap_create() {

    binheap* b = (binheap*)malloc(sizeof(binheap));
    if (b == NULL) exit((printf("heap creation error\n"), 1));

    b->v = (dc_cons**)malloc(INIT_SIZE*sizeof(dc_cons*));
    if (b->v == NULL) exit((printf("heap creation error\n"), 1));

    b->comp = NULL;
    b->n = 0;
    b->max = INIT_SIZE-1;

    return b;
}


// ---------------------------------------------------------------------
//  heap_schedule
// ---------------------------------------------------------------------
int heap_schedule(void* scheduler, dc_cons* cons) {

    binheap* b = (binheap*)scheduler;
    unsigned i, p;

    // possibly resize the heap
    if (b->n == b->max) {
        b->max = ((b->max+1) << 1) - 1;  // double array size
        b->v = (dc_cons**)realloc(b->v, (b->max+1)*sizeof(dc_cons*));
        if (b->v == NULL) exit((printf("heap realloc error\n"), 1));
    }

    // increment item counter
    b->n++;

    // fix heap ordering by percolating the item up the tree
    for (i = b->n; i > 1; i = p) {
        p = i / 2;
        if (b->comp(b->v[p], cons) <= 0) break;
        b->v[i] = b->v[p];
    }

    // insert item at proper position
    b->v[i] = cons;

    #if DEBUG
    dump(b);
    #endif

    return 0;
}


// ---------------------------------------------------------------------
//  heap_pick
// ---------------------------------------------------------------------
dc_cons* heap_pick(void* scheduler) {
    binheap* b = (binheap*)scheduler;
    dc_cons* cons;
    dc_cons* repl;
    unsigned i;

    // heap is empty: return NULL
    if (b->n < 1) return NULL;

    // pick min from heap root
    cons = b->v[1];

    // replace root with last item in heap
    repl = b->v[b->n];

    // decrease number of items
    b->n--;

    // percolate item down the tree
    for (i = 2; i <= b->n; i <<= 1) {
        if (i+1 < b->n && b->comp(b->v[i], b->v[i+1]) > 0) i++;
        if (b->comp(repl, b->v[i]) <= 0) break;
        b->v[i/2] = b->v[i];
    }

    b->v[i/2] = repl;

    #if DEBUG
    dump(b);
    #endif

    return cons;
}


// ---------------------------------------------------------------------
//  heap_destroy
// ---------------------------------------------------------------------
void heap_destroy(void* scheduler) {
    binheap* b = (binheap*)scheduler;
    free(b->v);
    free(b);
}


// ---------------------------------------------------------------------
//  heap_destroy
// ---------------------------------------------------------------------
void heap_set_comp(void* scheduler, int (*comp)(dc_cons*, dc_cons*)) {
    binheap* b = (binheap*)scheduler;
    b->comp = comp;
}


// Copyright (C) 2011 Camil Demetrescu

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
