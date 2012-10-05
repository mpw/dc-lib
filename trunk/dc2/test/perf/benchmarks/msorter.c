/* =====================================================================
 *  msorter.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 6, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/07 13:01:09 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.6 $
*/

#include <stdio.h>
#include <stdlib.h>

#include "dc.h"
#include "dc_inspect.h"
#include "msorter.h"
#include "halver.h"
#include "merger.h"


// macros
#ifndef DEBUG
#define DEBUG 2
#endif


// intermediate lists
typedef struct {
    rlist*    l_us;   // unsorted left half
    rlist*    r_us;   // unsorted right half
    rlist*    l_s;    // sorted left half
    rlist*    r_s;    // sorted right half
    halver*   h;      // halver
    merger*   m;      // merger
    msorter*  l;      // left half sorter
    msorter*  r;      // right half sorter
} inter;


// object structure
struct msorter_s {
    dc_cons*  cons;
    rlist*    in;     // input unsorted list
    rlist*    out;    // output sorted list
    inter*    i;      // intermediate lists
};


// ---------------------------------------------------------------------
//  dump_sorter_rec
// ---------------------------------------------------------------------
static void dump_sorter_rec(msorter* s, int indent) {

    if (s->i != NULL) {
        dc_printf("%*s[msorter %p] :\n", indent, "", s);
        dc_printf("%*s    <in> : ", indent, "");
        rlist_print(s->in);
        dc_printf("%*s    [halver %p]\n", indent, "", s->i->h);
        dump_sorter_rec(s->i->l, indent + 4);
        dump_sorter_rec(s->i->r, indent + 4);
        dc_printf("%*s    [merger %p]\n", indent, "", s->i->m);
        dc_printf("%*s    <out> : ", indent, "");
        rlist_print(s->out);
    }
    else {
        dc_printf("%*s[msorter %p] <in/out> : ", indent, "", s);
        rlist_print(s->out);
    }
}


// ---------------------------------------------------------------------
//  msorter_dump
// ---------------------------------------------------------------------
void msorter_dump(msorter* s) {
    dump_sorter_rec(s, 0);
}


// ---------------------------------------------------------------------
//  tree_dump_rec
// ---------------------------------------------------------------------
static void tree_dump_rec(msorter* s, int indent) {
    int j;
    for (j=0; j<indent; j++)  printf("%c",(j%4)?' ':'.');
    for (j=0; j<rlist_length(s->in); j++) printf("*");
    printf(" (%d",j);

    if (s->i != NULL) {
        printf(" = %u + ", 
            (s->i->l==NULL) ? 0 : rlist_length(s->i->l->in));
        printf("%u)\n", 
            (s->i->r==NULL) ? 0 : rlist_length(s->i->r->in));
        tree_dump_rec(s->i->l, indent + 4);
        tree_dump_rec(s->i->r, indent + 4);
    }
    else printf(")\n");

}


// ---------------------------------------------------------------------
//  msorter_tree_dump
// ---------------------------------------------------------------------
void msorter_tree_dump(msorter* s) {
    tree_dump_rec(s, 0);
}


// ---------------------------------------------------------------------
//  sorter_depth_rec
// ---------------------------------------------------------------------
static int sorter_depth_rec(msorter* s, int depth, int *num_leaves,
                            int *leaves_depth) {
    if (s->i != NULL) {
        int left_depth = sorter_depth_rec(s->i->l, depth + 1, 
                                            num_leaves, leaves_depth);
        int right_depth = sorter_depth_rec(s->i->r, depth + 1, 
                                            num_leaves, leaves_depth);
        if (left_depth > right_depth) return left_depth+1;
        else return right_depth+1;
    }
    else {
        (*num_leaves)++;
        (*leaves_depth) += depth;
        return depth;
    }
}


// ---------------------------------------------------------------------
//  msorter_depth
// ---------------------------------------------------------------------
int msorter_depth(msorter* s, int *num_leaves, int *leaves_depth) {
    *num_leaves = 0;
    *leaves_depth = 0;
    return sorter_depth_rec(s, 0, num_leaves, leaves_depth);
}


// ---------------------------------------------------------------------
//  msorter_cons
// ---------------------------------------------------------------------
static void msorter_cons(msorter* s) {

    #if DEBUG > 0
    dc_printf("[msorter %p] exec cons - input list: ", s);
    dc_begin_no_log();
    rlist_print(s->in);
    dc_end_no_log();
    #endif

    // base step: input list is empty, or it contains only one item
    if (rlist_first(s->in) == NULL || 
        rlist_next(rlist_first(s->in)) == NULL) {

        dc_begin_no_log();

        // update the output list so that it is equal to the input list
        #if 0
        if (rlist_first(s->in) != NULL) {
            if (rlist_first(s->out) == NULL)
                rlist_insert_first(s->out, 
                                   rlist_val(rlist_first(s->in)));
            else {
                rlist_set_val(rlist_first(s->out), 
                              rlist_val(rlist_first(s->in)));
                while (rlist_next(rlist_first(s->out)) != NULL)
                    rlist_remove_next(s->out, rlist_first(s->out));
            }
        }
        else while (rlist_first(s->out) != NULL) 
                rlist_remove_first(s->out);
        #else
        rlist_remove_all(s->out);
        if (rlist_first(s->in) != NULL)
            rlist_insert_first(s->out, rlist_val(rlist_first(s->in)));
        #endif

        // dispose of intermediate lists, if any
        if (s->i != NULL) {

            #if DEBUG > 0
            dc_printf("[msorter %p] deleting sub-sorters\n", s);
            #endif

            merger_delete(s->i->m);
            msorter_delete(s->i->l);
            msorter_delete(s->i->r);
            halver_delete(s->i->h);
            rlist_delete(s->i->l_us);
            rlist_delete(s->i->r_us);
            rlist_delete(s->i->l_s);
            rlist_delete(s->i->r_s);
            free(s->i);
            s->i = NULL;
        }

        dc_end_no_log();
    }

    // list contains more than one item: maintain intermediate lists
    else if (s->i == NULL) {

        #if DEBUG > 0
        dc_printf("[msorter %p] creating sub-sorters\n", s);
        #endif

        dc_begin_no_log();

        s->i = (inter*)malloc(sizeof(inter));

        s->i->l_us = rlist_new();
        s->i->r_us = rlist_new();
        s->i->l_s  = rlist_new();
        s->i->r_s  = rlist_new();
        s->i->h    = halver_new(s->in, s->i->l_us, s->i->r_us);
        s->i->l    = msorter_new(s->i->l_us, s->i->l_s);
        s->i->r    = msorter_new(s->i->r_us, s->i->r_s);
        s->i->m    = merger_new(s->i->l_s, s->i->r_s, s->out);

        dc_end_no_log();
    }

    #if DEBUG > 2
    dc_printf("[msorter %p] -- end cons\n", s);
    #endif
}


// ---------------------------------------------------------------------
// msorter_new
// ---------------------------------------------------------------------
msorter* msorter_new(rlist* in, rlist* out) {

    // create sorter object
    msorter* s = (msorter*)malloc(sizeof(msorter));
    if (s == NULL) return NULL;

    #if DEBUG > 0
    dc_printf("[msorter %p] creating for input list: ", s);
    rlist_print(in);
    #endif

    // setup in and out lists
    s->in  = in;
    s->out = out;

    // no intermediate lists
    s->i = NULL;

    // create constraint
    s->cons = dc_new_cons((dc_cons_f)msorter_cons, s, NULL);

    return s;
}


// ---------------------------------------------------------------------
// msorter_delete
// ---------------------------------------------------------------------
void msorter_delete(msorter* s) {

    #if DEBUG > 0
    dc_printf("[msorter %p] deleting\n", s);
    #endif

    // delete constraint
    dc_del_cons(s->cons);

    // dispose of intermediate lists, if any
    if (s->i) {
        dc_begin_at();
        merger_delete(s->i->m);
        msorter_delete(s->i->l);
        msorter_delete(s->i->r);
        halver_delete(s->i->h);
        rlist_delete(s->i->l_us);
        rlist_delete(s->i->r_us);
        rlist_delete(s->i->l_s);
        rlist_delete(s->i->r_s);
        free(s->i);
        dc_end_at();
    }

    free(s);
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
