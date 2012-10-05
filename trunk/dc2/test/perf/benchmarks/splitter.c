/* =====================================================================
 *  splitter.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/05 18:37:34 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.18 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "splitter.h"
#include "dc.h"
#include "dc_inspect.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// constraint object (24 bytes on 32-bit platform)
typedef struct snode_s snode;
struct snode_s {
    dc_cons*  cons;
    splitter* s;

    rnode*    out;
    snode*    next;
    snode*    prev;
    int       is_small;
};

// splitter object (20 bytes on 32-bit platform)
struct splitter_s {
    rlist* in;
    rlist* small;
    rlist* large;
    snode* small_s;
    snode* large_s;
};


// private functions
static void        panic(char* msg);
static void        ins(splitter* s, rnode* prev);
static void        rem(splitter* s, rnode* prev);
static void        cons(rnode* node);
static inline void hook(snode* c, int val);
static inline void unhook(snode* c);


// ---------------------------------------------------------------------
// splitter_new
// ---------------------------------------------------------------------
splitter* splitter_new(rlist* in, rlist* small, rlist* large) {

    // create splitter
    splitter* s = malloc(sizeof(splitter));
    if (s == NULL) panic("out of memory");

    #if DEBUG > 0
    dc_printf("[splitter %p] creating for input list: ", s);
    rlist_print(in);
    #endif

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

        snode* c = calloc(sizeof(snode), 1);
        if (c == NULL) panic("out of memory");
        c->s = s;
        rlist_set_param(node, c);

        if (node != rlist_first(in)) {
            c->cons = dc_new_cons((dc_cons_f)cons, node, NULL);
            hook(c, rlist_val(node));
        }
    }
    dc_end_at();

    return s;
}


// ---------------------------------------------------------------------
// splitter_delete
// ---------------------------------------------------------------------
void splitter_delete(splitter* s) {
    rnode* node;

    #if DEBUG > 0
    dc_printf("[splitter %p] deleting\n", s);
    #endif

    for (node = rlist_first(s->in); 
         node != NULL; 
         node = rlist_next(node)) {

        snode* c = (snode*)rlist_param(node);
        if (c->cons != NULL) dc_del_cons(c->cons);
        free(c);
    }
    rlist_unsubscribe(s->in);

    free(s);
}


// ---------------------------------------------------------------------
//  ins
// ---------------------------------------------------------------------
static void ins(splitter* s, rnode* prev) {

    rnode *in;

    #if DEBUG > 0
    in = prev == NULL ? rlist_first(s->in) : prev->next;
    dc_printf("[splitter %p] ins node %p as succ of %p with val %d\n", 
        s, in, prev, rlist_val(in));
    #endif

    snode* c = calloc(sizeof(snode), 1);
    if (c == NULL) panic("out of memory");
    c->s = s;

    if (prev == NULL) {
        in = rlist_first(s->in);
        if (rlist_next(in)) {
            snode* next = (snode*)rlist_param(rlist_next(in));
            next->cons = 
                dc_new_cons((dc_cons_f)cons, rlist_next(in), NULL);
            hook(next, rlist_val(rlist_next(in)));
        }
    }
    else {
        in = prev->next;
        c->cons = dc_new_cons((dc_cons_f)cons, in, NULL);
        hook(c, rlist_val(in));
    }

    rlist_set_param(in, c);
}


// ---------------------------------------------------------------------
//  rem
// ---------------------------------------------------------------------
static void rem(splitter* s, rnode* prev) {
    snode* c;

    #if DEBUG > 0
    dc_printf("[splitter %p] removing successor of node %p\n", s, prev);
    #endif

    if (prev == NULL) {
        c = (snode*)rlist_param(rlist_first(s->in));
        rnode* dead = rlist_first(s->in);
        if (rlist_next(dead)) {
            snode* next = (snode*)rlist_param(rlist_next(dead));
            dc_del_cons(next->cons);
            next->cons = NULL;
            unhook(next);
        }
    }
    else {
        c = (snode*)rlist_param(prev->next);
        dc_del_cons(c->cons);
        unhook(c);
    }

    free(c);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
static void cons(rnode* node) {

    snode* c = (snode*)rlist_param(node);

    // read input
    int new_val = rlist_val(node);
    int pivot   = rlist_val(rlist_first(c->s->in));

    // update output
    dc_begin_no_log();

    #if DEBUG > 0
    dc_printf("[splitter %p] exec cons - updating node %p to %d "
           "(old val = %d, pivot = %d)\n", 
        c->s, node, new_val, rlist_val(c->out), pivot);
    #endif

    if ((new_val >= pivot &&  c->is_small) || 
        (new_val <  pivot && !c->is_small)) {

        // re-hook
        unhook(c);
        hook(c, new_val);
    }
    else rlist_set_val(c->out, new_val);

    dc_end_no_log();
}


// ---------------------------------------------------------------------
//  hook
// ---------------------------------------------------------------------
static inline void hook(snode* c, int val) {

    int     pivot = rlist_val(rlist_first(c->s->in));
    rlist*  out;
    snode** head;

    // check if item is small or large
    if (val < pivot) {
        out  =  c->s->small;
        head = &c->s->small_s;
        c->is_small = 1;
    }
    else {
        out  =  c->s->large;
        head = &c->s->large_s;
        c->is_small = 0;
    }

    #if DEBUG > 0
    dc_printf("[splitter %p]     hook %d (snode %p) to %s "
              "output list\n", 
        c->s, val, c, val < pivot ? "small" : "large");
    #endif

    // hook to doubly-linked snode list
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
static inline void unhook(snode* c) {

    rlist*  out;
    snode** head;

    // check if item is small or large
    if (c->is_small) {
        out  =  c->s->small;
        head = &c->s->small_s;
    }
    else {
        out  =  c->s->large;
        head = &c->s->large_s;
    }

    #if DEBUG > 0
    dc_printf("[splitter %p]     unhook %d (snode %p) from %s "
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
    dc_printf("[splitter] %s\n", msg);
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
