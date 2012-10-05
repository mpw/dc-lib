// =====================================================================
//  dc/src/dc_sched_heap.c
// =====================================================================

//  Author:         (C) 2011 Irene Finocchi
//  License:        See the end of this file for license information
//  Created:        January 6, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 13:56:57 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.15 $


#include "dc_defs.h"
#include "dc_sched_heap.h"

// initial size of the heap (number of constraints)
// must be at least 4
#define MIN_HEAP_SIZE 16


// setup scheduler class structure
static dc_scheduler_type _sched_s = {
    dc_sched_heap_create,
    dc_sched_heap_schedule,
    dc_sched_heap_pick,
    dc_sched_heap_destroy
};

// export pointer to structure
dc_scheduler_type* dc_sched_heap_g = &_sched_s;

typedef struct {
    dc_ui32  key;               // key (timestamp)
    dc_cons* cons;              // value (constraint)
} _heap_item_t;

// scheduling data structure
typedef struct {
    _heap_item_t*   array;    // array of pointers to constraints
    size_t          size;     // size of the array
    size_t          num;      // number of constraints in the array   
} _sched_t;


// ---------------------------------------------------------------------
//  dc_sched_heap_create
// ---------------------------------------------------------------------
void* dc_sched_heap_create() {

    // create scheduler object
    _sched_t* sched = (_sched_t*)malloc(sizeof(_sched_t));
    if (sched == NULL) return NULL;

    // allocate array
    sched->array = 
        (_heap_item_t*)malloc(MIN_HEAP_SIZE*sizeof(_heap_item_t));
    if (sched->array == NULL) {
        free(sched);
        return NULL;
    }

    // write left sentinel
    sched->array[0].key = 0;

    // setup remaining fields
    sched->size = MIN_HEAP_SIZE;
    sched->num  = 0;
    
    return sched;
}


// ---------------------------------------------------------------------
//  dc_sched_heap_schedule
// ---------------------------------------------------------------------
int dc_sched_heap_schedule(void* scheduler, dc_cons* cons) {
    _sched_t* sched = (_sched_t*)scheduler;
    size_t num = ++sched->num;

    // possibly resize the array
    if (num+1 == sched->size) {
        // increase size by 50%
        sched->size += (sched->size >> 1);

        // reallocate array
        sched->array = 
            (_heap_item_t*)realloc(sched->array, 
                               sched->size*sizeof(_heap_item_t));
        if (sched->array == NULL) return -1;
    }

    _heap_item_t* heap = sched->array;

    // Move items dows to compute hole (use left sentinel = 0)
    size_t pos = num;
    dc_ui32 new_key = cons->time_stamp;
    while (heap[pos >> 1].key > new_key) { 
        heap[pos] = heap[pos >> 1];
        pos >>= 1; 
    }

    // Write new item in the hole
    heap[pos].key   = new_key;
    heap[pos].cons  = cons;

    // write right sentinel
    heap[num+1].key = DC_MAX_TIME_STAMP;

    return 0;
}


// ---------------------------------------------------------------------
//  dc_sched_heap_pick
// ---------------------------------------------------------------------
dc_cons* dc_sched_heap_pick(void* scheduler) {
    _sched_t* sched = (_sched_t*)scheduler;
    size_t  sz = sched->num;

    // if heap is empty then return NULL
    if (sz == 0) return NULL;

    size_t  hole = 1, 
            succ = 2, 
            pred;
    _heap_item_t* heap = sched->array;
 
    dc_cons *picked_cons = heap[1].cons;

    while (succ < sz) {
        dc_ui32 key1 = heap[succ].key;
        dc_ui32 key2 = heap[succ+1].key;
        if (key1 > key2) {
            succ++;
            heap[hole].key = key2;
        }
        else heap[hole].key = key1;
        heap[hole].cons = heap[succ].cons;
        hole = succ;
        succ <<= 1;    
    }
    
    dc_ui32 bubble = heap[sz].key;
    pred = hole >> 1;
    while (heap[pred].key > bubble) { 
        heap[hole] = heap[pred];
        hole = pred;
        pred >>= 1; 
    }
    
    heap[hole].key = bubble;
    heap[hole].cons = heap[sz].cons;
    
    // write right sentinel
    heap[sz].key = DC_MAX_TIME_STAMP;
    
    sched->num = sz-1;

    return picked_cons;
}


// ---------------------------------------------------------------------
//  dc_sched_heap_destroy
// ---------------------------------------------------------------------
void dc_sched_heap_destroy(void* scheduler) {
    _sched_t* sched = (_sched_t*)scheduler;
    free(sched->array);
    free(sched);
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
