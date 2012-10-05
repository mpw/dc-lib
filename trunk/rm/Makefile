# ======================================================================
#  rm/Makefile
# ======================================================================

#  Author:         (C) 2010 the rm team
#  License:        See the end of this file for license information
#  Created:        July 21, 2010
#  Module:         rm

#  Last changed:   $Date: 2012/09/25 14:16:16 $
#  Changed by:     $Author: ribbi $
#  Revision:       $Revision: 1.11 $

all: librm

librm:
	gcc -O0 -g -D rm_STAT=1 -D rm_DEBUG=1 -D PATCH_DEBUG=0 -D rm_DISASSEMBLER=rm_LIBDISASM -c ./src/rm_alloc.c ./src/rm_asm.c ./src/rm_av.c ./src/rm_emu.c ./src/rm_exec_heap.c ./src/rm_heap.c ./src/rm_init.c ./src/rm_patch.c -I./include/ -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
	ar rcs ./lib/librm.a ./*.o
	rm -f ./*.o


# Copyright (c) 2010 the rm team

 # This library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Lesser General Public
 # License as published by the Free Software Foundation; either
 # version 2.1 of the License, or (at your option) any later version.

 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.

 # You should have received a copy of the GNU Lesser General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
