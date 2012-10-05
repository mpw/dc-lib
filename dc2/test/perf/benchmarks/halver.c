/* =====================================================================
 *  halver.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 5, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/06 11:50:20 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.3 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "halver.h"
#include "dc.h"
#include "dc_inspect.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// constraint object (24 bytes on 32-bit platform)
typedef struct hnode_s hnode;
struct hnode_s {
    dc_cons*  cons;
    halver* s;

    rnode*    out;
    hnode*    next;
    hnode*    prev;
    int       is_small;
};

// halver object (24 bytes on 32-bit platform)
struct halver_s {
    rlist* in;
    rlist* small;
    rlist* large;
    hnode* small_s;
    hnode* large_s;
    int    balancing;
};


// private functions
static void        panic(char* msg);
static void        ins(halver* s, rnode* prev);
static void        rem(halver* s, rnode* prev);
static void        cons(rnode* node);
static inline void hook(hnode* c, int val);
static inline void unhook(hnode* c);


// ---------------------------------------------------------------------
// halver_new
// ---------------------------------------------------------------------
halver* halver_new(rlist* in, rlist* small, rlist* large) {

    // create halver
    halver* s = malloc(sizeof(halver));
    if (s == NULL) panic("out of memory");

    #if DEBUG > 0
    dc_printf("[halver %p] creating for input list: ", s);
    rlist_print(in);
    #endif

    s->balancing = 0;
    s->in  = in;
    s->small = small;
    s->large = large;
    s->small_s = s->large_s = NULL;

    // clear output lists
    rlist_remove_all(small);
    rlist_remove_all(large);

    // subscribe to input list
    rlist_subscribe(in, s, (rlist_ins_t)ins, (rlist_rem_t)rem);

    // initialize output
    dc_begin_at();
    rnode *node;
    for (node = rlist_first(in); 
         node != NULL; 
         node = rlist_next(node)) {

        hnode* c = calloc(sizeof(hnode), 1);
        if (c == NULL) panic("out of memory");
        c->s = s;
        rlist_set_param(node, c);
        c->cons = dc_new_cons((dc_cons_f)cons, node, NULL);
        hook(c, rlist_val(node));
    }
    dc_end_at();

    return s;
}


// ---------------------------------------------------------------------
// halver_delete
// ---------------------------------------------------------------------
void halver_delete(halver* s) {
    rnode* node;

    #if DEBUG > 0
    dc_printf("[halver %p] deleting\n", s);
    #endif

    for (node = rlist_first(s->in); 
         node != NULL; 
         node = rlist_next(node)) {

        hnode* c = (hnode*)rlist_param(node);
        dc_del_cons(c->cons);
        free(c);
    }
    rlist_unsubscribe(s->in);

    free(s);
}


// ---------------------------------------------------------------------
//  ins
// ---------------------------------------------------------------------
static void ins(halver* s, rnode* prev) {

    rnode* in = prev == NULL ? rlist_first(s->in) : prev->next;

    #if DEBUG > 0
    dc_printf("[halver %p] ins node %p as succ of %p with val %d\n", 
        s, in, prev, rlist_val(in));
    #endif

    hnode* c = calloc(sizeof(hnode), 1);
    if (c == NULL) panic("out of memory");
    c->s = s;
    rlist_set_param(in, c);
    c->cons = dc_new_cons((dc_cons_f)cons, in, NULL);
    hook(c, rlist_val(in));
}


// ---------------------------------------------------------------------
//  rem
// ---------------------------------------------------------------------
static void rem(halver* s, rnode* prev) {

    #if DEBUG > 0
    dc_printf("[halver %p] removing successor of node %p\n", s, prev);
    #endif

    hnode* c = prev == NULL ? (hnode*)rlist_param(rlist_first(s->in)) :
                              (hnode*)rlist_param(prev->next);

    dc_del_cons(c->cons);
    unhook(c);

    free(c);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
static void cons(rnode* node) {

    hnode* c = (hnode*)rlist_param(node);

    #if DEBUG > 0
    dc_printf("[halver %p] exec cons - updating node %p to %d "
           "(old val = %d)\n", 
        c->s, node, rlist_val(node), rlist_val(c->out));
    #endif

    rlist_set_val(c->out, rlist_val(node));
}


// ---------------------------------------------------------------------
//  hook
// ---------------------------------------------------------------------
static inline void hook(hnode* c, int val) {

    rlist*  out;
    hnode** head;

    // check current balancing
    if (c->s->balancing >= 0) {
        out  =  c->s->small;
        head = &c->s->small_s;
        c->is_small = 1;
        c->s->balancing--;
    }
    else {
        out  =  c->s->large;
        head = &c->s->large_s;
        c->is_small = 0;
        c->s->balancing++;
    }

    #if DEBUG > 0
    dc_printf("[halver %p]     hook %d (hnode %p) to %s "
              "output list\n", 
        c->s, val, c, c->is_small ? "small" : "large");
    #endif

    // hook to doubly-linked hnode list
    c->next = NULL;
    c->prev = *head;
    *head = c;

    // add node to output list
    if (c->prev == NULL)
        c->out = rlist_insert_first(out, val);
    else {
        c->prev->next = c;
        c->out = rlist_insert_next(out, c->prev->out, val);
    }
}


// ---------------------------------------------------------------------
//  unhook
// ---------------------------------------------------------------------
static inline void unhook(hnode* c) {

    rlist*  out;
    hnode** head;

    // check if item is small or large
    if (c->is_small) {
        out  =  c->s->small;
        head = &c->s->small_s;
        c->s->balancing++;
    }
    else {
        out  =  c->s->large;
        head = &c->s->large_s;
        c->s->balancing--;
    }

    #if DEBUG > 0
    dc_printf("[halver %p]     unhook %d (hnode %p) from %s "
        "output list\n", 
        c->s, rlist_val(c->out), c, c->is_small ? "small" : "large");
    #endif

    // unhook from doubly-linked list and from output list
    if (c->prev != NULL) {
        c->prev->next = c->next;
        if (c->next != NULL) c->next->prev = c->prev;
        else *head = c->prev;
        rlist_remove_next(out, c->prev->out);
    }
    else {
        if (c->next == NULL) *head = NULL;
        else c->next->prev = NULL;
        rlist_remove_first(out);
    }
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
static void panic(char* msg) {
    dc_printf("[halver] %s\n", msg);
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
