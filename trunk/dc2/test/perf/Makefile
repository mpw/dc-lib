# ======================================================================
#  dc/test/perf/Makefile
# ======================================================================

#  Author:         (C) 2010-2011 Camil Demetrescu, Irene Finocchi, 
#                                Andrea Ribichini
#  License:        See the end of this file for license information
#  Created:        October 23, 2010
#  Module:         dc/test/perf

#  Last changed:   $Date: 2011/04/05 21:19:24 $
#  Changed by:     $Author: ribbi $
#  Revision:       $Revision: 1.54 $


CC       = gcc
C++      = g++
RM       = ../../../rm
OUT_DIR  = bin/
SRC      = benchmarks/
LIB 	 = -L../../lib -L$(RM)/lib/ -ldc -ldisasm -lm              	\
			-lrm -lelf -lglib-2.0
LIBP	 = -L../../lib -L$(RM)/lib/ -ldc-profile -ldisasm -lm       \
			-lrm -lelf -lglib-2.0
INC 	 = -I. -I../../include -I$(RM)/include/ -Ibenchmarks/      	\
           -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
LOGS     = logs/
TEST_SRC = test-driver.c logfile.c
FLAGS_O0 = -m32 -g -O0 -D OPT=\"O0\"                               	\
		   -D DEBUG=0 -D LOGS_DIR=\"logs/\"
FLAGS_O3 = -m32 -O3 -Wall -D OPT=\"03\"                            	\
           -D DEBUG=0 -D LOGS_DIR=\"logs/\"


all: rbfm dsp exptrees mapper msorter splitter vecmat matmat matadd

test_heap_sched:
	$(CC) -o $(OUT_DIR)test_heap_sched 								\
		$(SRC)heap_sched.c $(SRC)test_heap_sched.c $(INC) 

RShortestPaths:
	$(CC) $(FLAGS_O3)                                               \
		-o $(OUT_DIR)RShortestPaths-O3 $(SRC)RShortestPaths.c 		\
		$(INC) $(LIB)												\
		../../../ll-core/src/*.c -I../../../ll-core/include


# ----------------------------------------------------------------------
# rbfm
# ----------------------------------------------------------------------

rbfm:
	$(CC) $(FLAGS_O3) -DDC=1 -DDEFAULT_SCHED=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0	\
		-o $(OUT_DIR)rbfm-test-driver-ds-O3 $(SRC)rbfm-test.c 			\
		$(SRC)rbfm.c $(SRC)dijkstra.c $(SRC)heap_sched.c 			\
		$(TEST_SRC) $(INC) $(LIB) ../../../ll-core/src/*.c 			\
		-I../../../ll-core/include
	$(CC) $(FLAGS_O3) -DDC=1 -DDEFAULT_SCHED=0 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0	\
		-o $(OUT_DIR)rbfm-test-driver-O3 $(SRC)rbfm-test.c 			\
		$(SRC)rbfm.c $(SRC)dijkstra.c $(SRC)heap_sched.c 			\
		$(TEST_SRC) $(INC) $(LIB) ../../../ll-core/src/*.c 			\
		-I../../../ll-core/include
	$(CC) $(FLAGS_O3) -DDC=1 -DDEFAULT_SCHED=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)rbfm-test-driver-ds-p-O3 $(SRC)rbfm-test.c 		\
		$(SRC)rbfm.c $(SRC)dijkstra.c $(SRC)heap_sched.c 			\
		$(TEST_SRC) $(INC) $(LIBP) ../../../ll-core/src/*.c 		\
		-I../../../ll-core/include
	$(CC) $(FLAGS_O3) -DDC=1 -DDEFAULT_SCHED=0 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)rbfm-test-driver-p-O3 $(SRC)rbfm-test.c 		\
		$(SRC)rbfm.c $(SRC)dijkstra.c $(SRC)heap_sched.c 			\
		$(TEST_SRC) $(INC) $(LIBP) ../../../ll-core/src/*.c 		\
		-I../../../ll-core/include

rbfm-test:
	rm -f rbfm-test-*.log
	$(OUT_DIR)rbfm-test-driver-O3 			\
		264347 benchmarks/data/NY.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		264347 benchmarks/data/NY.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		264347 benchmarks/data/NY.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		264347 benchmarks/data/NY.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1	
	$(OUT_DIR)rbfm-test-driver-O3 			\
		321271 benchmarks/data/BAY.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		321271 benchmarks/data/BAY.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		321271 benchmarks/data/BAY.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		321271 benchmarks/data/BAY.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1		
	$(OUT_DIR)rbfm-test-driver-O3 			\
		435667 benchmarks/data/COL.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		435667 benchmarks/data/COL.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		435667 benchmarks/data/COL.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		435667 benchmarks/data/COL.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1		
	$(OUT_DIR)rbfm-test-driver-O3 			\
		1070377 benchmarks/data/FLA.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		1070377 benchmarks/data/FLA.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		1070377 benchmarks/data/FLA.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		1070377 benchmarks/data/FLA.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1		
	$(OUT_DIR)rbfm-test-driver-O3 			\
		1207946 benchmarks/data/NW.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		1207946 benchmarks/data/NW.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		1207946 benchmarks/data/NW.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		1207946 benchmarks/data/NW.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-O3 			\
		1524454 benchmarks/data/NE.gr		\
		"decr_hs" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-O3 		\
		1524454 benchmarks/data/NE.gr		\
		"decr_ds" rbfm-test-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-p-O3 		\
		1524454 benchmarks/data/NE.gr		\
		"decr_hs" rbfm-test-p-O3.log 971 1
	$(OUT_DIR)rbfm-test-driver-ds-p-O3 		\
		1524454 benchmarks/data/NE.gr		\
		"decr_ds" rbfm-test-p-O3.log 971 1

# ----------------------------------------------------------------------
# dsp
# ----------------------------------------------------------------------

dsp:
	$(CC) $(FLAGS_O3) -DDC=0 -DCONV=0 -DCHECK=1 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)dsp-test-driver-O3 $(SRC)dsp-test.c 			\
		$(SRC)dijkstra.c $(SRC)dsp.c $(TEST_SRC) $(INC) $(LIB)		\
		../../../ll-core/src/*.c -I../../../ll-core/include

dsp-test:
	rm -f dsp-test-*.log
	$(OUT_DIR)dsp-test-driver-O3 			\
		264347 benchmarks/data/NY.gr		\
		"decr" dsp-test-O3.log 971 1
	$(OUT_DIR)dsp-test-driver-O3 			\
		321271 benchmarks/data/BAY.gr		\
		"decr" dsp-test-O3.log 971 1
	$(OUT_DIR)dsp-test-driver-O3 			\
		435667 benchmarks/data/COL.gr		\
		"decr" dsp-test-O3.log 971 1
	$(OUT_DIR)dsp-test-driver-O3 			\
		1070377 benchmarks/data/FLA.gr		\
		"decr" dsp-test-O3.log 971 1
	$(OUT_DIR)dsp-test-driver-O3 			\
		1207946 benchmarks/data/NW.gr		\
		"decr" dsp-test-O3.log 971 1
	$(OUT_DIR)dsp-test-driver-O3 			\
		1524454 benchmarks/data/NE.gr		\
		"decr" dsp-test-O3.log 971 1


# ----------------------------------------------------------------------
# ddsp
# ----------------------------------------------------------------------

ddsp:
	$(CC) $(FLAGS_O3) -DDC=0 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)ddsp-test-driver-O3 $(SRC)ddsp-test.c 			\
		$(SRC)dijkstra.c $(SRC)ddsp.c $(TEST_SRC) $(INC) $(LIB)		\
		../../../ll-core/src/*.c -I../../../ll-core/include \
		-D DDSP_STAT=1

ddsp-test:
	rm -f ddsp-test-*.log
	$(OUT_DIR)ddsp-test-driver-O3 			\
		264347 benchmarks/data/NY.gr		\
		"decr" ddsp-test-O3.log 971 1
	$(OUT_DIR)ddsp-test-driver-O3 			\
		321271 benchmarks/data/BAY.gr		\
		"decr" ddsp-test-O3.log 971 1
	$(OUT_DIR)ddsp-test-driver-O3 			\
		435667 benchmarks/data/COL.gr		\
		"decr" ddsp-test-O3.log 971 1
	$(OUT_DIR)ddsp-test-driver-O3 			\
		1070377 benchmarks/data/FLA.gr		\
		"decr" ddsp-test-O3.log 971 1
	$(OUT_DIR)ddsp-test-driver-O3 			\
		1207946 benchmarks/data/NW.gr		\
		"decr" ddsp-test-O3.log 971 1
	$(OUT_DIR)ddsp-test-driver-O3 			\
		1524454 benchmarks/data/NE.gr		\
		"decr" ddsp-test-O3.log 971 1


# ----------------------------------------------------------------------
# exptrees
# ----------------------------------------------------------------------

exptrees:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)exptrees-test-driver-O3 $(SRC)exptrees-test.c 	\
		$(SRC)exptrees.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)exptrees-test-driver-p-O3 $(SRC)exptrees-test.c 	\
		$(SRC)exptrees.c $(TEST_SRC) $(INC) $(LIBP)

exptrees-test:
	rm -f $(LOGS)exptr*
	for input_size in 100000 200000 300000 400000 500000 600000    	\
				      700000 800000 900000 1000000 ; do            	\
    $(OUT_DIR)exptrees-test-driver-O3 $$input_size "n/a" "100andBack"\
              exptrees-test-O3.log 971 1;                   		\
    $(OUT_DIR)exptrees-test-driver-p-O3 $$input_size "n/a" "100andBack"\
              exptrees-test-p-O3.log 971 1;                   		\
    done

# ----------------------------------------------------------------------
# merger
# ----------------------------------------------------------------------

merger:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)merger-test-driver-O3 						   	\
		$(SRC)merger-test.c $(SRC)merger.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=1      	\
		-o $(OUT_DIR)merger-test-driver-p-O3 						\
		$(SRC)merger-test.c $(SRC)merger.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIBP)

merger-test:
	rm -f merger-test-*.log
	for input_size in 100000 200000 300000 400000 500000 600000 	\
	                  700000 800000 900000 1000000 ; do         	\
    $(OUT_DIR)merger-test-driver-O3 $$input_size "1-1" "rem-ins" 	\
		merger-test-O3.log 971 1; 									\
    $(OUT_DIR)merger-test-driver-p-O3 $$input_size "1-1" "rem-ins" 	\
		merger-test-p-O3.log 971 1; 								\
    done


# ----------------------------------------------------------------------
# adder
# ----------------------------------------------------------------------

adder:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)adder-test-driver-O3 						   	\
		$(SRC)adder-test.c $(SRC)adder.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=1      	\
		-o $(OUT_DIR)adder-test-driver-p-O3 						\
		$(SRC)adder-test.c $(SRC)adder.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIBP)

adder-test:
	rm -f adder-test-*.log
	for input_size in 100000 200000 300000 400000 500000 600000 	\
	                  700000 800000 900000 1000000 ; do         	\
    $(OUT_DIR)adder-test-driver-O3 $$input_size "n/a" "rem-ins" 	\
		adder-test-O3.log 971 1; 									\
    $(OUT_DIR)adder-test-driver-p-O3 $$input_size "n/a" "rem-ins" 	\
		adder-test-p-O3.log 971 1; 								\
    done

# ----------------------------------------------------------------------
# halver
# ----------------------------------------------------------------------

halver:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)halver-test-driver-O3 						   	\
		$(SRC)halver-test.c $(SRC)halver.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=1      	\
		-o $(OUT_DIR)halver-test-driver-p-O3 						\
		$(SRC)halver-test.c $(SRC)halver.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIBP)

halver-test:
	rm -f halver-test-*.log
	for input_size in 100000 200000 300000 400000 500000 600000 	\
	                  700000 800000 900000 1000000 ; do         	\
    $(OUT_DIR)halver-test-driver-O3 $$input_size "n/a" "rem-ins" 	\
		halver-test-O3.log 971 1; 									\
    $(OUT_DIR)halver-test-driver-p-O3 $$input_size "n/a" "rem-ins" 	\
		halver-test-p-O3.log 971 1; 								\
    done


# ----------------------------------------------------------------------
# mapper
# ----------------------------------------------------------------------

mapper:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)mapper-test-driver-O3 						   	\
		$(SRC)mapper-test.c $(SRC)mapper.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=1      	\
		-o $(OUT_DIR)mapper-test-driver-p-O3 						\
		$(SRC)mapper-test.c $(SRC)mapper.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIBP)

mapper-test:
	rm -f mapper-test-*.log
	for input_size in 100000 200000 300000 400000 500000 600000 	\
	                  700000 800000 900000 1000000 ; do         	\
    $(OUT_DIR)mapper-test-driver-O3 $$input_size "n/a" "rem-ins" 	\
		mapper-test-O3.log 971 1; 									\
    $(OUT_DIR)mapper-test-driver-p-O3 $$input_size "n/a" "rem-ins" 	\
		mapper-test-p-O3.log 971 1; 								\
    done


# ----------------------------------------------------------------------
# msorter
# ----------------------------------------------------------------------

msorter:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)msorter-test-driver-O3 -D MSORTER				\
		$(SRC)sorter-test.c $(SRC)msorter.c $(SRC)halver.c 			\
		$(SRC)merger.c $(SRC)rlist.c $(SRC)rlist_updates.c 			\
		$(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)msorter-test-driver-p-O3 -D MSORTER			\
		$(SRC)sorter-test.c $(SRC)msorter.c $(SRC)halver.c 			\
		$(SRC)merger.c $(SRC)rlist.c  $(SRC)rlist_updates.c			\
		$(TEST_SRC) $(INC) $(LIBP)

msorter-test:
	rm -f msorter-test-*.log
	for input_size in 10000 20000 30000 40000 50000 60000 			\
	                  70000 80000 90000 100000; do         		\
    $(OUT_DIR)msorter-test-driver-O3 $$input_size "n/a" "rem-ins"	\
		msorter-test-O3.log 971 1; 									\
    $(OUT_DIR)msorter-test-driver-p-O3 $$input_size "n/a" "rem-ins"	\
		msorter-test-p-O3.log 971 1; 								\
    done


# ----------------------------------------------------------------------
# splitter
# ----------------------------------------------------------------------

splitter:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)splitter-test-driver-O3 						   	\
		$(SRC)splitter-test.c $(SRC)splitter.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=1      	\
		-o $(OUT_DIR)splitter-test-driver-p-O3 						\
		$(SRC)splitter-test.c $(SRC)splitter.c $(SRC)rlist_updates.c   	\
		$(SRC)rlist.c $(SRC)list.c $(TEST_SRC) $(INC) $(LIBP)

splitter-test:
	rm -f splitter-test-*.log
	for input_size in 100000 200000 300000 400000 500000 600000 	\
	                  700000 800000 900000 1000000 ; do         	\
    $(OUT_DIR)splitter-test-driver-O3 $$input_size "n/a" "rem-ins" 	\
		splitter-test-O3.log 971 1; 									\
    $(OUT_DIR)splitter-test-driver-p-O3 $$input_size "n/a" "rem-ins" 	\
		splitter-test-p-O3.log 971 1; 								\
    done


# ----------------------------------------------------------------------
# vecmat
# ----------------------------------------------------------------------

vecmat:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=1 -DCHECK=0 -DMAKE_ASM_DUMP=0   \
		-o $(OUT_DIR)vecmat-test-driver-O3 						   	\
		$(SRC)vecmat-test.c $(SRC)vecmat.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)vecmat-test-driver-p-O3 						\
		$(SRC)vecmat-test.c $(SRC)vecmat.c $(TEST_SRC) $(INC) $(LIBP)

vecmat-test:
	rm -f vecmat-test-*.log
	for input_size in 1 2 4 8 16 32 64 128 256 512 1024 2000; do 	\
	$(OUT_DIR)vecmat-test-driver-O3 $$input_size "square_2000" 		\
		"mrnd_50000" vecmat-test-O3.log 971 1; 						\
	$(OUT_DIR)vecmat-test-driver-p-O3 $$input_size "square_2000" 	\
		"mrnd_50000" vecmat-test-p-O3.log 971 1; 					\
	done; 															\
	for input_size in 1 2 4 8 16 32 64 128 256 512 1012 2000; do 	\
	$(OUT_DIR)vecmat-test-driver-O3 $$input_size "square_2000" 		\
		"crnd_500" vecmat-test-O3.log 971 1; 						\
	$(OUT_DIR)vecmat-test-driver-p-O3 $$input_size "square_2000" 	\
		"crnd_500" vecmat-test-p-O3.log 971 1; 						\
	done


# ----------------------------------------------------------------------
# matmat
# ----------------------------------------------------------------------

matmat:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=1 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)matmat-test-driver-O3 						   	\
		$(SRC)matmat-test.c $(SRC)matmat.c 							\
		$(SRC)vecmat.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)matmat-test-driver-p-O3 						\
		$(SRC)matmat-test.c $(SRC)matmat.c 							\
		$(SRC)vecmat.c $(TEST_SRC) $(INC) $(LIBP)

matmat-test-a:
	rm -f matmat-test-a*.log
	for input_size in 500 1000 1500 2000 2500 3000; do 				\
	$(OUT_DIR)matmat-test-driver-O3 $$input_size "n/a" 				\
		"arow_100" matmat-test-a-O3.log 971 1; 						\
	$(OUT_DIR)matmat-test-driver-p-O3 $$input_size "n/a" 			\
		"arow_100" matmat-test-a-p-O3.log 971 1; 					\
	done

matmat-test-ab:
	rm -f matmat-test-ab*.log
	for input_size in 500 1000 1500 2000 2500 3000; do 				\
	$(OUT_DIR)matmat-test-driver-O3 $$input_size "n/a" 				\
		"arow/bcol_100" matmat-test-ab-O3.log 971 1; 				\
	$(OUT_DIR)matmat-test-driver-p-O3 $$input_size "n/a" 			\
		"arow/bcol_100" matmat-test-ab-p-O3.log 971 1; 				\
	done


# ----------------------------------------------------------------------
# matadd
# ----------------------------------------------------------------------

matadd:
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0   \
		-o $(OUT_DIR)matadd-test-driver-O3 						   	\
		$(SRC)matadd-test.c $(SRC)matadd.c $(TEST_SRC) $(INC) $(LIB)
	$(CC) $(FLAGS_O3) -DDC=1 -DCONV=0 -DCHECK=0 -DMAKE_ASM_DUMP=0  	\
		-o $(OUT_DIR)matadd-test-driver-p-O3 						\
		$(SRC)matadd-test.c $(SRC)matadd.c $(TEST_SRC) $(INC) $(LIBP)

matadd-test:
	rm -f matadd-test-*.log
	for input_size in 32 64 128 256 512 1024 2048; do 			    \
	$(OUT_DIR)matadd-test-driver-O3 $$input_size "square" 			\
		"ABrnd_1000" matadd-test-O3.log 971 1; 						\
	$(OUT_DIR)matadd-test-driver-p-O3 $$input_size "square" 		\
		"ABrnd_1000" matadd-test-p-O3.log 971 1; 					\
	done; 															\
	for input_size in 1 2 4 8 16 32 64 128 256; do 					\
	$(OUT_DIR)matadd-test-driver-O3 $$input_size "square" 			\
		"Arow_50" matadd-test-O3.log 971 1; 						\
	$(OUT_DIR)matadd-test-driver-p-O3 $$input_size "square" 		\
		"Arow_50" matadd-test-p-O3.log 971 1; 						\
	done


# Copyright (C) 2010-2011 C. Demetrescu, I. Finocchi, A. Ribichini

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA 
