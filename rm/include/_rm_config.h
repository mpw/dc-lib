/* ============================================================================
 *  _rm_config.h
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           rm configuration header

 *  Last changed:   $Date: 2011/01/06 17:24:20 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.8 $
*/


#ifndef ___rm_config__
#define ___rm_config__

#ifdef __cplusplus
extern "C" {
#endif


#define rm_DUMP_FILE_NAME           "dump.txt"
#define rm_ILLEGAL_INSTRUCTION      0xCC
//#define rm_CODE_START             0x08048000
#define rm_heap_START_NUM_OF_PAGES  1
#define rm_SINGLE_INSN_MAX_SIZE     256

// The number of the array of instructions the decoder function will use to return the disassembled instructions.
// Play with this value for performance...
#define rm_MAX_DIS_INSTRUCTIONS     1000



#ifdef __cplusplus
}
#endif

#endif


/* Copyright (c) 2010 the rm team

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
