/* =====================================================================
 *  rlist_updates.h
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 8, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/08 14:26:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1 $
*/


#ifndef __rlist_updates__
#define __rlist_updates__

#include "rlist.h"
#include <stddef.h>


// ---------------------------------------------------------------------
// rlist_updates
// ---------------------------------------------------------------------
/** Perform update sequence on input list.
 *  \param list input list 
 *  \param family string with the format "family-name:arg1:arg2:..."
 *         describing the update family, where 
 *         family-name can be either of the following:
 *         "rem-ins"     = equivalent to rlist_rem_ins_updates();
 *         "incr-decr"   = equivalent to rlist_rem_ins_updates(),
 *                         where arg1 specifies the value of delta
 *         "rnd-batch"   = equiv. to rlist_rnd_batch_ins_rem_updates(),
 *                         where arg1 specifies the value of k
 *  \param callback pointer to function to be called after each update
 *  \param param user parameter to be passed to callback()
*/
size_t rlist_updates(rlist* list, char* family,
                     void (*callback)(void*), void* param);


// ---------------------------------------------------------------------
// rlist_rem_ins_updates
// ---------------------------------------------------------------------
/** Perform removal/insertion update sequence on input list. Each node 
 *  of the input list is removed and immediately re-added.
 *  \param list input list 
 *  \param callback pointer to function to be called after each update
 *  \param param user parameter to be passed to callback()
*/
size_t rlist_rem_ins_updates(rlist* list, 
                             void (*callback)(void*), void* param);


// ---------------------------------------------------------------------
// rlist_incr_decr_updates
// ---------------------------------------------------------------------
/** Perform increase/decrease update sequence on input list. The value
 *  of each node in the input list is first increased by delta and tne 
 *  immediately restored to its previous value.
 *  \param list input list 
 *  \param callback pointer to function to be called after each update
 *  \param param user parameter to be passed to callback()
*/
size_t rlist_incr_decr_updates(rlist* list, int delta,
                                 void (*callback)(void*), void* param);


// ---------------------------------------------------------------------
// rlist_rnd_batch_ins_rem_updates
// ---------------------------------------------------------------------
/** Perform batch update sequence on input list. The list is scanned
 *  three times: for each node, with probability 1/2 either we insert a 
 *  new successor, or we remove the successor. k updates in a row
 *  form an atomic sequence, so that the DC solver is only activated 
 *  once every k operations.
 *  \param list input list 
 *  \param k size of the batch as number of list update operations
 *  \param callback pointer to function to be called after each update
 *  \param param user parameter to be passed to callback()
*/
size_t rlist_rnd_batch_ins_rem_updates(rlist* list, size_t k,
                                 void (*callback)(void*), void* param);

#endif


/* Copyright (C) 2011 Camil Demetrescu

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
