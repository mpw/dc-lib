// =====================================================================
//  dc/examples/vartracker.cpp
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        October 20, 2010
//  Module:         dc/examples

//  Last changed:   $Date: 2011/01/14 11:57:12 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.3 $


#include "vartracker.h"

int main() {

    volatile long* var = (long*)dc_malloc(sizeof(long));

    vartracker<long> mytracker(var);

    *var = 10;
    *var = 20;

    mytracker.disable();

    (*var)++;

    return 0;
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
