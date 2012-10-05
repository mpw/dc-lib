/* =====================================================================
 *  rlist.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/08 14:26:24 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.15 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "dc.h"
#include "rlist.h"
#include "rand_pm.h"


// private functions
static void panic(char* msg);
static inline rnode* rlist_insert(rlist* list, rnode** head, int val);
static inline void rlist_remove(rlist* list, rnode** head);


// ---------------------------------------------------------------------
//  rlist_new
// ---------------------------------------------------------------------
rlist* rlist_new() {
    rlist* list = malloc(sizeof(rlist));
    if (list == NULL) panic("out of memory");
    list->first = dc_malloc(sizeof(rnode*));
    if (list->first == NULL) panic("out of memory");
    *list->first = NULL;
    list->observer = NULL;
    list->ins = NULL;
    list->rem = NULL;
    return list;
}


// ---------------------------------------------------------------------
//  rlist_delete
// ---------------------------------------------------------------------
void rlist_delete(rlist* list) {
    list->observer = NULL;
    rlist_remove_all(list);
    dc_free(list->first);
    free(list);
}


// ---------------------------------------------------------------------
//  rlist_new_rand
// ---------------------------------------------------------------------
rlist* rlist_new_rand(int n, int max_val) {
    dc_begin_at();
    rlist* list = rlist_new();
    for (; n > 0; --n) rlist_insert_first(list, rand_pm() % max_val);
    dc_end_at();
    return list;
}


// ---------------------------------------------------------------------
//  rlist_new_sorted_rand
// ---------------------------------------------------------------------
rlist* rlist_new_sorted_rand(int n, int max_delta) {
    int val = 1 + rand_pm() % max_delta;
    dc_begin_at();
    rlist* list = rlist_new();
    rnode* prev = rlist_insert_first(list, val);
    for (; n > 1; --n) {
        val += 1 + rand_pm() % max_delta;
        prev = rlist_insert_next(list, prev, val);
    }
    dc_end_at();
    return list;
}


// ---------------------------------------------------------------------
//  rlist_subscribe
// ---------------------------------------------------------------------
void rlist_subscribe(rlist* list,
                void* observer, rlist_ins_t ins, rlist_rem_t rem) {

    if (list->observer != NULL) panic("can't hook more than once");
    list->observer = observer;
    list->ins = ins;
    list->rem = rem;
}


// ---------------------------------------------------------------------
//  rlist_unsubscribe
// ---------------------------------------------------------------------
void rlist_unsubscribe(rlist* list) {
    list->observer = NULL;
}


// ---------------------------------------------------------------------
//  rlist_insert_first
// ---------------------------------------------------------------------
rnode* rlist_insert_first(rlist* list, int val) {
    dc_begin_at();
    rnode* node = rlist_insert(list, list->first, val);
    if (list->observer) list->ins(list->observer, NULL);
    dc_end_at();
    return node;
}


// ---------------------------------------------------------------------
//  rlist_insert_next
// ---------------------------------------------------------------------
rnode* rlist_insert_next(rlist* list, rnode* prev, int val) {
    dc_begin_at();
    rnode* node = rlist_insert(list, &prev->next, val);
    if (list->observer) list->ins(list->observer, prev);
    dc_end_at();
    return node;
}


// ---------------------------------------------------------------------
//  rlist_remove_first
// ---------------------------------------------------------------------
void rlist_remove_first(rlist* list) {
    dc_begin_at();
    if (list->observer) list->rem(list->observer, NULL);  
    rlist_remove(list, list->first);
    dc_end_at();
}


// ---------------------------------------------------------------------
//  rlist_remove_next
// ---------------------------------------------------------------------
void rlist_remove_next(rlist* list, rnode* prev) {
    dc_begin_at();
    if (list->observer) list->rem(list->observer, prev);  
    rlist_remove(list, &prev->next);
    dc_end_at();
}


// ---------------------------------------------------------------------
//  rlist_remove_all
// ---------------------------------------------------------------------
void rlist_remove_all(rlist* list) {
    dc_begin_at();
    while (*list->first) rlist_remove_first(list);
    dc_end_at();
}


// ---------------------------------------------------------------------
//  rlist_last
// ---------------------------------------------------------------------
rnode* rlist_last(rlist* list) {
    rnode *node = *list->first, *prev = NULL;
    for (; node != NULL; node = node->next) prev = node;
    return prev;
}


// ---------------------------------------------------------------------
//  rlist_length
// ---------------------------------------------------------------------
size_t rlist_length(rlist* list) {
    size_t len = 0;
    rnode* node = *list->first;
    for (; node != NULL; node = node->next) len++;
    return len;
}


// ---------------------------------------------------------------------
//  rlist_print
// ---------------------------------------------------------------------
void rlist_print(rlist* list) {
    rnode* node = *list->first;
    for (; node != NULL; node = node->next)
        printf("%d ", node->val);
    printf("\n");
}


// ---------------------------------------------------------------------
//  rlist_insert
// ---------------------------------------------------------------------
static inline rnode* rlist_insert(rlist* list, rnode** head, int val) {
    rnode* node = dc_malloc(sizeof(rnode));
    if (node == NULL) panic("out of memory");
    node->val = val;
    node->next = dc_inactive(*head);
    *head = node;
    return node;
}


// ---------------------------------------------------------------------
//  rlist_remove
// ---------------------------------------------------------------------
static inline void rlist_remove(rlist* list, rnode** head) {
    rnode* dead = dc_inactive(*head);
    *head = dead->next;
    dc_free(dead);
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
static void panic(char* msg) {
    fprintf(stderr, "[rlist] %s\n", msg);
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
