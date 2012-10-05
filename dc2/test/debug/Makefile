# =====================================================================
#  dc/test/debug/Makefile
# =====================================================================

#  Author:         (C) 2010-2011 Camil Demetrescu
#  License:        See the end of this file for license information
#  Created:        December 31, 2010
#  Module:         dc

#  Last changed:   $Date: 2011/03/03 16:36:23 $
#  Changed by:     $Author: demetres $
#  Revision:       $Revision: 1.31 $

CC 		 = gcc
C++		 = g++
RM 		 = ../../../rm
OUT_DIR  = ./bin/
UTIL     = ../perf/
BENCH    = ../perf/benchmarks/
LIB      = -L../../lib -L$(RM)/lib/ -ldisasm \
           -lm -ldc-profile -lrm -lelf -lglib-2.0
LIBD     = -L../../lib -L$(RM)/lib/ \
		   -ldisasm -lm -ldc-debug -lrm -lelf -lglib-2.0
LIBP     = -L../../lib -L$(RM)/lib/ \
		   -ldisasm -lm -ldc-profile -lrm -lelf -lglib-2.0
INC      = -I../../include -I$(RM)/include/ -I/usr/include/glib-2.0 \
           -I/usr/lib/glib-2.0/include -I$(BENCH) -I. -I$(UTIL)
FLAGS_O0 = -m32 -g -O0 -Wall -DOPT=\"O0\" 
FLAGS_O3 = -m32 -O3 -Wall -DOPT=\"O3\"


all: test00 test01 test02 test03 test04 test05 test06 test07 \
	test_sched_heap mapper-test splitter-test

test00:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test00-O0 test00.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test00-O3 test00.c $(INC) $(LIB)

test01:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test01-O0 test01.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test01-O3 test01.c $(INC) $(LIB)

test02:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test02-O0 test02.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test02-O3 test02.c $(INC) $(LIB)

test03:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test03-O0 test03.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test03-O3 test03.c $(INC) $(LIB)

test04:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test04-O0 test04.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test04-O3 test04.c $(INC) $(LIB)

test05:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test05-O0 test05.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test05-O3 test05.c $(INC) $(LIB)

test06:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test06-O0 test06.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test06-O3 test06.c $(INC) $(LIB)

test07:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)test07-O0 test07.c \
		$(BENCH)list.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)test07-O3 test07.c \
		$(BENCH)list.c $(INC) $(LIB)

mapper:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)mapper-testapp-O0 mapper-testapp.c \
		$(BENCH)mapper.c $(BENCH)rlist.c $(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)mapper-testapp-O3 mapper-testapp.c \
		$(BENCH)mapper.c $(BENCH)rlist.c $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)mapper-testapp-p mapper-testapp.c \
		$(BENCH)mapper.c $(BENCH)rlist.c $(INC) $(LIBP)

halver:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)halver-testapp-O0 			\
		halver-testapp.c $(BENCH)halver.c $(BENCH)rlist.c 	\
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)halver-testapp-O3 			\
		halver-testapp.c $(BENCH)halver.c $(BENCH)rlist.c 	\
		$(INC) $(LIB)

splitter:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)splitter-testapp-O0 			\
		splitter-testapp.c $(BENCH)splitter.c $(BENCH)rlist.c 	\
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)splitter-testapp-O3 			\
		splitter-testapp.c $(BENCH)splitter.c $(BENCH)rlist.c 	\
		$(INC) $(LIB)

splitter-large:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)splitter-large-testapp-O0    \
		splitter-testapp.c $(BENCH)splitter.c $(BENCH)rlist.c 	\
		$(INC) $(LIBD) -DLARGE_TEST -DDEBUG=0
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)splitter-large-testapp-O3    \
		splitter-testapp.c $(BENCH)splitter.c $(BENCH)rlist.c 	\
		$(INC) $(LIB) -DLARGE_TEST -DDEBUG=0

merger:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)merger-testapp-O0 			\
		merger-testapp.c $(BENCH)merger.c $(BENCH)rlist.c 		\
		$(INC) $(LIBD) -DDEBUG=2
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)merger-testapp-O3 			\
		merger-testapp.c $(BENCH)merger.c $(BENCH)rlist.c 		\
		$(INC) $(LIB)

joiner:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)joiner-testapp-O0 			\
		joiner-testapp.c $(BENCH)joiner.c $(BENCH)rlist.c 		\
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)joiner-testapp-O3 			\
		joiner-testapp.c $(BENCH)joiner.c $(BENCH)rlist.c 		\
		$(INC) $(LIB)

splitter-joiner:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)splitter-joiner-testapp-O0 	\
		splitter-joiner-testapp.c							 	\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c		\
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)splitter-joiner-testapp-O3 	\
		splitter-joiner-testapp.c							 	\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c 	\
		$(INC) $(LIB)

qsorter:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)qsorter-testapp-O0 			\
		sorter-testapp.c $(BENCH)qsorter.c 						\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c		\
		$(INC) $(LIBD) -DQSORTER -DDEBUG=2
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)qsorter-testapp-O3 			\
		sorter-testapp.c $(BENCH)qsorter.c 		 				\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c 	\
		$(INC) $(LIB) -DQSORTER

qsorter-large:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)qsorter-large-testapp-O0 	\
		sorter-testapp.c $(BENCH)qsorter.c						\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c		\
		$(INC) $(LIBD) -DLARGE_TEST -DDEBUG=0 -DQSORTER
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)qsorter-large-testapp-O3 	\
		sorter-testapp.c $(BENCH)qsorter.c 						\
		$(BENCH)rlist.c $(BENCH)splitter.c $(BENCH)joiner.c 	\
		$(INC) $(LIB) -DLARGE_TEST -DDEBUG=0 -DQSORTER

msorter:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)msorter-testapp-O0 			\
		sorter-testapp.c $(BENCH)msorter.c $(BENCH)merger.c		\
		$(BENCH)rlist.c $(BENCH)halver.c						\
		$(INC) $(LIBD) -DMSORTER -DDEBUG=2
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)msorter-testapp-O3 			\
		sorter-testapp.c $(BENCH)msorter.c $(BENCH)merger.c		\
		$(BENCH)rlist.c $(BENCH)halver.c 						\
		$(INC) $(LIB) -DMSORTER

msorter-large:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)msorter-large-testapp-O0 	\
		sorter-testapp.c $(BENCH)msorter.c	$(BENCH)merger.c	\
		$(BENCH)rlist.c $(BENCH)halver.c 						\
		$(INC) $(LIBD) -DLARGE_TEST -DDEBUG=0 -DMSORTER
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)msorter-large-testapp-O3 	\
		sorter-testapp.c $(BENCH)msorter.c 	$(BENCH)merger.c	\
		$(BENCH)rlist.c $(BENCH)halver.c 						\
		$(INC) $(LIB) -DLARGE_TEST -DDEBUG=0 -DMSORTER

sched-heap:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)sched-heap-testapp-O0 		\
		sched-heap-testapp.c ../../src/dc_sched_heap.c $(INC) 
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)sched-heap-testapp-O3 		\
		sched-heap-testapp.c ../../src/dc_sched_heap.c $(INC)

fibonacci:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)fib-O0 fibonacci-testapp.c 	\
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)fib-O3 fibonacci-testapp.c 	\
		$(INC) $(LIB)

edit-distance:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)edit-dist-O0 editdist-testapp.c \
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)edit-dist-O3 editdist-testapp.c \
		$(INC) $(LIB)

edit-dist-sapienza:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)edit-sapienza-O0 editdist-sapienza.c \
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)edit-sapienza-O3 editdist-sapienza.c \
		$(INC) $(LIB)

vector-matrix-prod:
	$(CC) $(FLAGS_O0) -o $(OUT_DIR)vector-matrix-prod-O0 vector-matrix-prod.c \
		$(INC) $(LIBD)
	$(CC) $(FLAGS_O3) -o $(OUT_DIR)vector-matrix-prod-O3 vector-matrix-prod.c \
		$(INC) $(LIB)

clean:
	cd bin; rm -f *-*


# Copyright (C) 2010-2011 Camil Demetrescu

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
