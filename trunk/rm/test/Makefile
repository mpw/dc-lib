
LIB = -L../lib -lrm -ldisasm -lm -lelf -lglib-2.0
INC = -I../include


all: test1

test1:
	gcc -O0 -D rm_STAT=1 -D rm_DEBUG=1 -D PATCH_DEBUG=0 -o ../bin/test1 ./test1.c $(INC) $(LIB)
	
mem_perf:
	gcc -O0 -o ../bin/mem_perf ./mem_perf.c $(INC) $(LIB)
	
mem_perf3:
	gcc -O0 -o ../bin/mem_perf3 ./mem_perf3.c $(INC) $(LIB)

exptrees:
	gcc -O3 -o ../bin/exptrees ./exptrees.c $(INC) $(LIB)

testmalloc:
	gcc -O0 -o ../bin/testmalloc ./testmalloc.c $(INC) $(LIB)

bug: ./bug.c
	gcc -O3 -o ./bug ./bug.c $(INC) $(LIB)

test_emu:
	gcc -O0 -D rm_STAT=1 -D rm_DEBUG=1 -D PATCH_DEBUG=0 -o ../bin/test_emu ./test_emu.c $(INC) $(LIB)

test_inline_patch:
	gcc -O0 -D rm_STAT=1 -D rm_DEBUG=1 -D PATCH_DEBUG=0 -o ../bin/test_inline_patch ./test_inline_patch.c $(INC) $(LIB)
	
test_inline_patch_xad:
	gcc -O0 -D rm_STAT=1 -D rm_DEBUG=1 -D PATCH_DEBUG=0 -o ../bin/test_inline_patch_xad ./test_inline_patch_xad.c $(INC) $(LIB)

