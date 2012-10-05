# =====================================================================
#  dc/examples/Makefile
# =====================================================================

#  Author:         (C) 2011 Camil Demetrescu
#  License:        See the end of this file for license information
#  Created:        January 12, 2011
#  Module:         dc

#  Last changed:   $Date: 2011/01/14 11:57:12 $
#  Changed by:     $Author: demetres $
#  Revision:       $Revision: 1.3 $


CC 		 = gcc
C++ 	 = g++
RM 		 = ../../rm
OUT_DIR  = ./
LIB      = -L../lib -L$(RM)/lib/ -ldisasm -lm -ldc \
		   -lrm -lelf -lglib-2.0
LIBD     = -L../lib -L$(RM)/lib/ -ldisasm -lm -ldc-debug \
           -lrm -lelf -lglib-2.0
INC 	 = -I../include -I$(RM)/include/ \
		   -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
FLAGS_O0 = -m32 -g -O0 -Wall
FLAGS_O3 = -m32 -O3


all: exptrees vartracker watcher

exptrees:
	$(C++) $(FLAGS_O0) -o $(OUT_DIR)exptrees-O0 exptrees.cpp \
		$(INC) $(LIBD)
	$(C++) $(FLAGS_O3) -o $(OUT_DIR)exptrees-O3 exptrees.cpp \
		$(INC) $(LIB)

vartracker:
	$(C++) $(FLAGS_O0) -o $(OUT_DIR)vartracker-O0 vartracker.cpp \
		$(INC) $(LIBD)
	$(C++) $(FLAGS_O3) -o $(OUT_DIR)vartracker-O3 vartracker.cpp \
		$(INC) $(LIB)

watcher:
	$(C++) $(FLAGS_O0) -o $(OUT_DIR)watcher-O0 watcher.cpp \
		$(INC) $(LIBD)
	$(C++) $(FLAGS_O3) -o $(OUT_DIR)watcher-O3 watcher.cpp \
		$(INC) $(LIB)


# Copyright (C) 2011 Camil Demetrescu

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
