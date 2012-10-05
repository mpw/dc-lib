/* =====================================================================
 *  rlist_updates.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 8, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/08 14:26:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dc.h"
#include "rlist_updates.h"
#include "rand_pm.h"


// ---------------------------------------------------------------------
//  rlist_updates
// ---------------------------------------------------------------------
size_t rlist_updates(rlist* list, 
                     char* family,
                     void (*callback)(void*), 
                     void* param) {

    char* name = strtok(family, ":");

    if (strcmp(name, "rem-ins") == 0) 
        return rlist_rem_ins_updates(list, callback, param);

    if (strcmp(name, "incr-decr") == 0) 
        return rlist_incr_decr_updates(
                    list, atoi(strtok(NULL, ":")),
                    callback, param);

    if (strcmp(name, "rnd-batch") == 0) 
        return rlist_rnd_batch_ins_rem_updates(list, 
                    atoi(strtok(NULL, ":")), 
                    callback, param);

    exit(printf("[rlist_updates] unknown update family"));
}


// ---------------------------------------------------------------------
//  rlist_rem_ins_updates
// ---------------------------------------------------------------------
size_t rlist_rem_ins_updates(rlist* list, 
                             void (*callback)(void*), void* param) {

    rnode* node;
    size_t num_updates = 2;
    if (rlist_first(list) == NULL) return 0;
    int val = rlist_val(rlist_first(list));
    rlist_remove_first(list);
    if (callback) callback(param);
    rlist_insert_first(list, val);
    if (callback) callback(param);
    for (node = rlist_first(list); 
         node != NULL && rlist_next(node) != NULL; 
         node = rlist_next(node)) {

        val = rlist_val(rlist_next(node));
        rlist_remove_next(list, node);
        if (callback) callback(param);
        rlist_insert_next(list, node, val);
        if (callback) callback(param);
        num_updates += 2;
    }
    return num_updates;
}


// ---------------------------------------------------------------------
//  rlist_rnd_change_updates
// ---------------------------------------------------------------------
size_t rlist_rnd_change_updates(rlist* list, 
                                 void (*callback)(void*), void* param) {

    rnode* node;
    size_t num_updates = 0;
    for (node = rlist_first(list); 
         node != NULL; 
         node = rlist_next(node)) {
        int val = rlist_val(node);
        dc_begin_at();
        rlist_set_val(node, rand_pm());
        dc_end_at();
        if (callback) callback(param);
        dc_begin_at();
        rlist_set_val(node, val);
        dc_end_at();
        if (callback) callback(param);
        num_updates += 2;
    }
    return num_updates;
}


// ---------------------------------------------------------------------
//  rlist_incr_decr_updates
// ---------------------------------------------------------------------
size_t rlist_incr_decr_updates(rlist* list, int delta,
                                 void (*callback)(void*), void* param) {

    rnode* node;
    size_t num_updates = 0;
    for (node = rlist_first(list); 
         node != NULL; 
         node = rlist_next(node)) {
        int val = rlist_val(node);
        dc_begin_at();
        rlist_set_val(node, val+delta);
        dc_end_at();
        if (callback) callback(param);
        dc_begin_at();
        rlist_set_val(node, val);
        dc_end_at();
        if (callback) callback(param);
        num_updates += 2;
    }
    return num_updates;
}


// ---------------------------------------------------------------------
// rlist_rnd_batch_ins_rem_updates
// ---------------------------------------------------------------------
size_t rlist_rnd_batch_ins_rem_updates(rlist* list, size_t k,
                                void (*callback)(void*), void* param) {

    int j;
    rnode* node;
    size_t num_changes = 0, num_updates = 0;

    dc_begin_at();

    // cycles through list 3 times...
    for (j = 0; j < 3; j++) {

        // process list head
        if ((rand_pm() % 2)==0) 
             rlist_insert_first(list, rand_pm());
        else rlist_remove_first(list);

        num_changes++;

        // process list tail...
        for (node = rlist_first(list); 
             node != NULL; 
             node = rlist_next(node)) {   

            if ((rand_pm() % 2)==0 || rlist_next(node) == 0)
                 rlist_insert_next(list, node, rand_pm());
            else rlist_remove_next(list, node);

            num_changes++;

            if (num_changes == k) {
                num_changes = 0;
                dc_end_at();
                if (callback) callback(param);
                num_updates++;
                dc_begin_at();
            }
        }
    }

    dc_end_at();
    if (num_changes != 0) {
        if (callback) callback(param);
        num_updates++;
    }

    return num_updates;
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
