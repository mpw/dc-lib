// =====================================================================
//  dc/src/dc_rmem.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 30, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/02/05 16:46:57 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.7 $


#include "dc_globals.h"


// ---------------------------------------------------------------------
//  dc_malloc
// ---------------------------------------------------------------------
void* dc_malloc(size_t size) {

    if (!dc_is_initialized_g && dc_init() != 0) return NULL;

    #ifdef DC_PROFILE
    dc_curr_profile.num_alloc++;
    if (dc_curr_profile.peak_num_reactive_blocks < 
        dc_curr_profile.num_alloc - dc_curr_profile.num_free) 
        dc_curr_profile.peak_num_reactive_blocks = 
            dc_curr_profile.num_alloc - dc_curr_profile.num_free;
    #endif

    #if DC_DEBUG == 1
    void* temp = rm_malloc(size);
    if (temp == NULL) _dc_panic("dc_malloc failed");
    return temp;
    #else
    return rm_malloc(size);
    #endif
}


// ---------------------------------------------------------------------
//  dc_calloc
// ---------------------------------------------------------------------
void* dc_calloc(size_t num, size_t size) {
    if (!dc_is_initialized_g && dc_init() != 0) return NULL;

    #ifdef DC_PROFILE
    dc_curr_profile.num_alloc++;
    #endif

    #if DC_DEBUG == 1
    void* temp = rm_calloc(num, size);
    if (temp == NULL) _dc_panic("dc_calloc failed");
    return temp;
    #else
    return rm_calloc(num, size);
    #endif
}


// ---------------------------------------------------------------------
//  dc_realloc
// ---------------------------------------------------------------------
void* dc_realloc(void* ptr, size_t size) {
    if (!dc_is_initialized_g) return NULL;

    #if DC_DEBUG == 1
    void* temp = rm_realloc(ptr, size);
    if (temp == NULL) _dc_panic("dc_realloc failed");
    return temp;
    #else
    return rm_realloc(ptr, size);
    #endif
}


// ---------------------------------------------------------------------
//  dc_free
// ---------------------------------------------------------------------
void dc_free(void* ptr) {
    if (dc_is_initialized_g) {

        #ifdef DC_PROFILE
        dc_curr_profile.num_free++;
        #endif

        rm_free(ptr);
    }
}


// Copyright (C) 2010-2011 Camil Demetrescu

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
