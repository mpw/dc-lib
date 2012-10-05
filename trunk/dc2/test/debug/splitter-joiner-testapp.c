/* ============================================================================
 *  splitter-joiner-testapp.c
 * ============================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 21, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/01/31 08:42:55 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.8 $
*/


#include <stdio.h>
#include <stdlib.h>
#include "dc.h"
#include "dc_inspect.h"
#include "rlist.h"
#include "splitter.h"
#include "joiner.h"

#define MAX_VAL         100
#define MAKE_DUMP_FILE  0
#define NUM_ITEMS       20


int check_splitter(rnode_t* in, rnode_t* tmp1, rnode_t* tmp2) {

    volatile rnode_t* p;
    int pivot = in == NULL ? 0 : in->val;

    // check if list lengths are correct
    printf("checking list lengths...\n");
    if (rlist_length(in) != rlist_length(tmp1)+rlist_length(tmp2)) {
        printf("error: incorrect list lengths\n");
        return 0;
    }

    // check if all nodes in tmp1 are correct
    printf("checking small list...\n");
    for (p = tmp1; p != NULL; p=p->next) 
        if (p->val > pivot) {
            printf("error: one item in small list is larger than pivot\n");
            return 0;
        }

    // check if all nodes in tmp2 are correct
    printf("checking large list...\n");
    for (p = tmp2; p != NULL; p = p->next) {
        int v = p->val;
        if (v < pivot) {
            printf("error: one item in small list is larger than pivot\n");
            return 0;
        }
    }

    printf("all checks ok\n");

    return 1;
}


// ---------------------------------------------------------------------
//  check_joiner
// ---------------------------------------------------------------------
int check_joiner(rnode_t* in1, rnode_t* in2, rnode_t* out) {

    while (out != NULL && in1 != NULL) {
        if (out->val != in1->val) return 0;
        out = out->next;
        in1 = in1->next;
    }

    while (out != NULL && in2 != NULL) {
        if (out->val != in2->val) return 0;
        out = out->next;
        in2 = in2->next;
    }

    return out == NULL && in1 == NULL && in2 == NULL;
}



void dump(rnode_t** in, rnode_t** tmp1, rnode_t** tmp2, rnode_t** out, 
          int* count) {

    unsigned new_count = dc_get_num_exec_cons();

    printf("in:    "); rlist_print(*in);
    printf("small: "); rlist_print(*tmp1);
    if (*in) printf("pivot: %d\n", (*in)->val);
    printf("large: "); rlist_print(*tmp2);
    printf("out:   "); rlist_print(*out);

    if (count == NULL) return;

    if (check_splitter(*in, *tmp1, *tmp2))
         puts("split operation ok");
    else puts("*** split operation failed");

    if (check_joiner(*tmp1, *tmp2, *out))
         puts("join operation ok");
    else puts("*** join operation failed");

    printf("exec cons: %u\n", new_count - *count);

    *count = new_count;
}


int main() {

    int count = 0;  // number of executed constraints

    // list heads
    rnode_t **in   = (rnode_t**)dc_malloc(sizeof(rnode_t));
    rnode_t **tmp1 = (rnode_t**)dc_malloc(sizeof(rnode_t));
    rnode_t **tmp2 = (rnode_t**)dc_malloc(sizeof(rnode_t));
    rnode_t **out  = (rnode_t**)dc_malloc(sizeof(rnode_t));

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/splitter-start.dump");
    #endif

    srand(3);

    *in = rlist_new_rand(NUM_ITEMS, MAX_VAL);
    *tmp1 = *tmp2 = *out = NULL;

    puts("--initial list in");
    dump(in, tmp1, tmp2, out, NULL);

    puts("--making splitter");
    splitter_t* h = splitter_new(in, tmp1, tmp2);
    if (h == NULL) {
        printf("error: cannot initialize splitter\n");
        exit(0);
    }
    dump(in, tmp1, tmp2, out, &count);

    puts("--making joiner");
    joiner_t* j = joiner_new(tmp1, tmp2, out);
    if (j == NULL) {
        printf("error: cannot initialize joiner\n");
        exit(0);
    }
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--changing to 1 first item of in (pivot)");
    (*in)->val = 1;
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--removing first item from in (pivot)");
    rlist_remove_first(in);
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--removing second item from in");
    rlist_remove_first(&(*in)->next);
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--removing three items from in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_remove_first(in);
    rlist_remove_first(in);
    dc_end_at();
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--adding four items {-1,-2,-3-,4} to in past the second item (batch)");
    dc_begin_at();
    rlist_add_first(&(*in)->next->next, -4);
    rlist_add_first(&(*in)->next->next, -3);
    rlist_add_first(&(*in)->next->next, -2);
    rlist_add_first(&(*in)->next->next, -1);
    dc_end_at();
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--removing 1st item + adding {1,2} to in (batch)");
    dc_begin_at();
    rlist_remove_first(in);
    rlist_add_first(in, 2);
    rlist_add_first(in, 1);
    dc_end_at();
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--replacing entire list in");
    rnode_t* b = *in;
    *in = rlist_new_rand(5, MAX_VAL);
    dump(in, tmp1, tmp2, out, &count);

    #if 0
    puts("\n--adding 33 to in");
    rlist_add_first(in, 33);
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--deleting entire list in");
    rnode_t* c = *in;
    *in = NULL;
    rlist_remove_all(&c);
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--restoring previous list in");
    *in = b;
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--individually removing three items past the fourth node of in");
    rlist_remove_first(&(*in)->next->next->next->next);
    dump(in, tmp1, tmp2, out, &count);
    rlist_remove_first(&(*in)->next->next->next->next);
    dump(in, tmp1, tmp2, out, &count);
    rlist_remove_first(&(*in)->next->next->next->next);
    dump(in, tmp1, tmp2, out, &count);

    puts("\n--disposing of joiner");
    joiner_delete(j);

    puts("\n--disposing of splitter");
    splitter_delete(h);

    puts("\n--clearing input list");
    rlist_remove_all(in);

    #if MAKE_DUMP_FILE
    rm_make_dump_file("logs/splitter-joiner-end.dump");
    #endif

    dc_free(in);
    dc_free(tmp1);
    dc_free(tmp2);
    dc_free(out);

    #endif

    return 0;
}


/* Copyright (C) 2010-2011 Camil Demetrescu

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  
 * USA
*/
