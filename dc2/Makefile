# ======================================================================
#  dc/Makefile
# ======================================================================

#  Author:         (C) 2010-2011 Camil Demetrescu
#  License:        See the end of this file for license information
#  Created:        December 30, 2010
#  Module:         dc

#  Last changed:   $Date: 2011/02/05 10:09:53 $
#  Changed by:     $Author: demetres $
#  Revision:       $Revision: 1.23 $


# macros

RM     = ../rm
INLINE = -finline-limit=2000
FLAGSD = -m32 -O0 -g -Wall \
		 -DDC_DEBUG=1 -DDC_TRACE=1 -DDC_PROFILE
FLAGSP = -m32 -O3 -Wall -Winline $(INLINE) \
         -Wstrict-aliasing=1 -DDC_DEBUG=0 -DDC_TRACE=0 -DDC_PROFILE
FLAGS  = -m32 -O3 -Wall -Winline $(INLINE) \
		 -Wstrict-aliasing=1 -DDC_DEBUG=0 -DDC_TRACE=0


# rules

all: libdc-debug libdc-profile libdc

libdc-debug:
	gcc $(FLAGSD) -c src/*.c -Iinclude -I$(RM)/include -L$(RM)/lib
	ar rcs lib/libdc-debug.a *.o
	rm -f *.o

libdc-profile:
	gcc $(FLAGSP) -c src/*.c -Iinclude -I$(RM)/include -L$(RM)/lib
	ar rcs lib/libdc-profile.a *.o
	rm -f *.o

libdc:
	gcc $(FLAGS) -c src/*.c -Iinclude -I$(RM)/include -L$(RM)/lib
	ar rcs lib/libdc.a *.o
	rm -f *.o


# temp rules...

asm:
	gcc $(FLAGS) -S src/dc_asm.c -Iinclude \
		-I$(RM)/include -L$(RM)/lib

solver:
	gcc $(FLAGS) -S src/dc_solver.c \
		-Iinclude -I$(RM)/include -L$(RM)/lib
	gcc $(FLAGS) -c src/dc_solver.c -g \
		-Iinclude -I$(RM)/include -L$(RM)/lib
	objdump -S dc_solver.o > dc_solver_dump.s

sched_heap:
	gcc $(FLAGSD) -S src/dc_sched_heap.c \
		-Iinclude -I$(RM)/include -L$(RM)/lib
	gcc $(FLAGSD) -c src/dc_sched_heap.c -g \
		-Iinclude -I$(RM)/include -L$(RM)/lib
	objdump -S dc_sched_heap.o > dc_sched_heap_dump.s


# documentation

manual: html latex

html:
	cd docs; doxygen html_config

latex:
	cd docs; doxygen latex_config; cd latex; $(MAKE)

website: html
	scp docs/html/* demetres@www.dis.uniroma1.it:public_html/dc/


# Copyright (c) 2010-2011 Camil Demetrescu

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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
# USA
