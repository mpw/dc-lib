/* ============================================================================
 *  LType.c
 * ============================================================================

 *  Author:         (c) 2001-2003 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 9, 2003
 *  Module:         LL

 *  Last changed:   $Date: 2005/11/17 18:10:42 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.1.1.1 $
*/

#include "LType.h"

/* DEFAULT COMPARATORS */
i4 LType_UI1Compar  (const void* inA, const void* inB) { 
    return (i4)_ui1_(inA) - (i4)_ui1_(inB); 
}

i4 LType_UI2Compar  (const void* inA, const void* inB) { 
    return (i4)_ui2_(inA) - (i4)_ui2_(inB); 
}

i4 LType_UI4Compar  (const void* inA, const void* inB) { 
    return (_ui4_(inA) < _ui4_(inB)) ? -1 : (_ui4_(inA) > _ui4_(inB)) ? 1 : 0; 
}

i4 LType_I1Compar   (const void* inA, const void* inB) { 
    return (i4) _i1_(inA) - (i4) _i1_(inB); 
}

i4 LType_I2Compar   (const void* inA, const void* inB) { 
    return (i4) _i2_(inA) - (i4) _i2_(inB); 
}

i4 LType_I4Compar   (const void* inA, const void* inB) { 
    return _i4_(inA) - _i4_(inB); 
}

i4 LType_F4Compar   (const void* inA, const void* inB) { 
    return (_f4_(inA) < _f4_(inB)) ? -1 : (_f4_(inA) > _f4_(inB)) ? 1 : 0; 
}

i4 LType_F8Compar   (const void* inA, const void* inB) { 
    return (_f8_(inA) < _f8_(inB)) ? -1 : (_f8_(inA) > _f8_(inB)) ? 1 : 0; 
}

i4 LType_BoolCompar (const void* inA, const void* inB) { 
    return (i4)(_i1_(inA)? 1:0) - (i4)(_i1_(inB)? 1:0); 
}

i4 LType_PtrCompar  (const void* inA, const void* inB) { 
    return (i4)((i1*)inA - (i1*)inB); 
}

/* INTRINSIC TYPES RECORDS */
#ifndef __LVM__
const LType_TType LType_UI1  = { LType_UI1_ID,  1, LType_UI1Compar  };
const LType_TType LType_UI2  = { LType_UI2_ID,  2, LType_UI2Compar  }; 
const LType_TType LType_UI4  = { LType_UI4_ID,  4, LType_UI4Compar  };
const LType_TType LType_I1   = { LType_I1_ID,   1, LType_I1Compar   }; 
const LType_TType LType_I2   = { LType_I2_ID,   2, LType_I2Compar   }; 
const LType_TType LType_I4   = { LType_I4_ID,   4, LType_I4Compar   }; 
const LType_TType LType_F4   = { LType_F4_ID,   4, LType_F4Compar   };
const LType_TType LType_F8   = { LType_F8_ID,   8, LType_F8Compar   };
const LType_TType LType_Bool = { LType_Bool_ID, 1, LType_BoolCompar };
const LType_TType LType_Ptr  = { LType_Ptr_ID,  sizeof(void*), LType_PtrCompar  };
#endif

/* FOR PLATFORM CHECKING... */
typedef struct {
    ui4 mField1;
    ui4 mField2;
    ui4 mField3;
} _CheckStructT;

/* ---------------------------------------------------------------------------------
 *  CheckConfig
 * ---------------------------------------------------------------------------------
 * function that checks if the compiler meets the LL preconditions  */
Bool LType_CheckConfig(){
    return sizeof(i1 )==1 &&
           sizeof(ui1)==1 &&
           sizeof(i2 )==2 &&
           sizeof(ui2)==2 &&
           sizeof(i4 )==4 &&
           sizeof(ui4)==4 &&
           sizeof(f4 )==4 &&
           sizeof(f8 )==8 &&
           sizeof(void*)==4 &&
           LType_FieldOffset(_CheckStructT,mField1)==0 &&
           LType_FieldOffset(_CheckStructT,mField2)==4 &&
           LType_FieldOffset(_CheckStructT,mField3)==8;
}


/* Copyright (C) 2001-2003 Camil Demetrescu

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
