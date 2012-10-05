// =====================================================================
//  dc/include/dcpp.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        October 20, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/01/12 16:27:51 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.2 $


#ifndef __DCPP__
#define __DCPP__

#include "dc.h"

#define DEBUG 0

#if DEBUG == 1
#include <iostream>
#endif


// ---------------------------------------------------------------------
// robject
// ---------------------------------------------------------------------
struct robject {
    void* operator new(size_t size) {
        #if DEBUG == 1
        std::cout << "[robject::new] allocating " << size << " bytes\n";
        #endif
        return dc_malloc(size);
    }

    void operator delete(void* ptr) {
        dc_free(ptr);
        #if DEBUG == 1
        std::cout << "[robject::delete] robject deleted\n";
        #endif
    }
};


// ---------------------------------------------------------------------
// rcons
// ---------------------------------------------------------------------

static void con_h(void*), fin_h(void*);

class rcons {
    dc_cons* c;

  public:
  
    void* vptr;

    rcons() { 
        c = NULL;
    }

    ~rcons() { 
        #if DEBUG == 1
        std::cout << "[rcons::~rcons] deleting cons\n"; 
        #endif
        disable(); 
    }

    virtual void cons() = 0;

    virtual void final() { }

    void enable() {
        #if DEBUG == 1
        std::cout << "[rcons::enable] enabling cons\n";
        #endif
        if (c != NULL) return;
        vptr = *(void**)this;
        c = dc_new_cons(con_h, this, NULL);
    }

    void disable() {
        #if DEBUG == 1
        std::cout << "[rcons::disable] disabling cons\n";
        #endif
        if (c == NULL) return;
        dc_del_cons(c);
        c = NULL;
    }
    
    void arm_final() {
        if (c != NULL) dc_schedule_final(c, fin_h);
    }

    void unarm_final() {
        if (c != NULL) dc_schedule_final(c, NULL);
    }
};

void con_h (void* p) { 
    
    #if DEBUG == 1
    std::cout << "[cons_h] calling cons - orig vptr=" 
              << ((rcons*)p)->vptr  
              << " - curr vptr=" 
              << *(void**)p << std::endl;  
    #endif

    if (*(void**)p == ((rcons*)p)->vptr) 
       ((rcons*)p)->cons();  
}

void fin_h (void* p) { 
    
    #if DEBUG == 1
    std::cout << "[cons_h] calling cons - orig vptr=" 
              << ((rcons*)p)->vptr  
              << " - curr vptr=" 
              << *(void**)p << std::endl;  
    #endif

    if (*(void**)p == ((rcons*)p)->vptr) 
       ((rcons*)p)->final();  
}

#endif


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
