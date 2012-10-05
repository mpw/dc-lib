// =====================================================================
//  dc/src/dc_sched_stack.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 6, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 13:56:57 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.8 $


#include "dc_sched_stack.h"

// initial size of the stack (number of constraints)
// must be at least 2
#define MIN_STACK_SIZE 16


// setup scheduler class structure
static dc_scheduler_type _sched_s = {
    dc_sched_stack_create,
    dc_sched_stack_schedule,
    dc_sched_stack_pick,
    dc_sched_stack_destroy
};

// export pointer to structure
dc_scheduler_type* dc_sched_stack_g = &_sched_s;

// scheduling data structure
typedef struct {
    dc_cons** array;    // array of pointers to constraints
    size_t    size;     // size of the array
    size_t    num;      // number of constraints in the array
} _sched_t;


// ---------------------------------------------------------------------
//  dc_sched_stack_create
// ---------------------------------------------------------------------
void* dc_sched_stack_create() {

    // create scheduler object
    _sched_t* sched = (_sched_t*)malloc(sizeof(_sched_t));
    if (sched == NULL) return NULL;

    // allocate array
    sched->array = (dc_cons**)malloc(MIN_STACK_SIZE*sizeof(dc_cons*));
    if (sched->array == NULL) {
        free(sched);
        return NULL;
    }

    // setup remaining fields
    sched->size = MIN_STACK_SIZE;
    sched->num  = 0;

    return sched;
}


// ---------------------------------------------------------------------
//  dc_sched_stack_schedule
// ---------------------------------------------------------------------
int dc_sched_stack_schedule(void* scheduler, dc_cons* cons) {
    _sched_t* sched = (_sched_t*)scheduler;

    // possibly resize the array
    if (sched->num == sched->size) {
        
        // increase size by 50%
        sched->size += (sched->size >> 1);

        // reallocate array
        sched->array = 
            (dc_cons**)realloc(sched->array, 
                               sched->size*sizeof(dc_cons*));
        if (sched->array == NULL) return -1;
    }

    // push constraint
    sched->array[sched->num++] = cons;

    return 0;
}


// ---------------------------------------------------------------------
//  dc_sched_stack_pick
// ---------------------------------------------------------------------
dc_cons* dc_sched_stack_pick(void* scheduler) {
    _sched_t* sched = (_sched_t*)scheduler;

    // if stack is empty then return NULL
    if (sched->num == 0) return NULL;

    // pop constraint
    return sched->array[--sched->num];
}


// ---------------------------------------------------------------------
//  dc_sched_stack_destroy
// ---------------------------------------------------------------------
void dc_sched_stack_destroy(void* scheduler) {
    _sched_t* sched = (_sched_t*)scheduler;
    free(sched->array);
    free(sched);
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
