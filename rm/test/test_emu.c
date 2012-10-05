#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rm.h"

#define PRINT_FLAGS 1

void read_handler(void* rm_addr, size_t size) {
	printf("*** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

void write_handler(void* rm_addr, size_t size) {
	printf("*** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

int main() {
	size_t shadow_rec_size = 32;
	size_t shadow_wordsize = 4;
	
	printf("MAIN - start\n");
	
	if (rm_init(read_handler, write_handler, NULL, NULL, shadow_rec_size, shadow_wordsize) == -1) {
		printf("main: rm_init error!!\n");
		return -1;
	}
	
#if rm_DEBUG == 1
	if (rm_make_dump_file("dump1.txt") == -1) {
		printf("main: rm_make_dump_file error!!\n");
		return -1;
	}
#endif
	
#if rm_STAT == 1
	printf("\n\n\n*** STATS - start ***\n");
	printf("g_stats.patch_instr_number = %u\n", g_stats.patch_instr_number);
	printf("g_stats.cache_instr_number = %u\n", g_stats.cache_instr_number);
	printf("g_stats.BB_number = %u\n", g_stats.BB_number);
	printf("g_stats.BB_min_size = %u\n", g_stats.BB_min_size);
	printf("g_stats.BB_med_size = %u\n", g_stats.BB_med_size);
	printf("g_stats.BB_max_size = %u\n", g_stats.BB_max_size);
	printf("g_stats.BB_min_instr_number = %u\n", g_stats.BB_min_instr_number);
	printf("g_stats.BB_med_instr_number = %u\n", g_stats.BB_med_instr_number);
	printf("g_stats.BB_max_instr_number = %u\n", g_stats.BB_max_instr_number);
	printf("*** STATS - end ***\n");
#endif
	
	int* a;
	int b;
	a = (int*)rm_malloc(sizeof(int));
	printf("\n\n\n");
	printf("Puntatore a = %p\n", a);
	printf("Puntatore rm_get_inactive_ptr(a) = %p\n", rm_get_inactive_ptr(a));
	printf("rm_is_reactive(a) = %d\n", rm_is_reactive(a));
	printf("rm_is_reactive(rm_get_inactive_ptr(a)) = %d\n", rm_is_reactive(rm_get_inactive_ptr(a)));
	printf("rm_get_shadow_rec(a) = %p\n", rm_get_shadow_rec(a));
	
	printf("*** Scrittura ***\n");
	*a = 5;
	printf("*** Lettura ***\n");
	b = *a;
	printf("b = %d\n", b);
	
	
	//***** Prova Emulazione: ***********************************************************
	//***** - uso asm per forzare un'istruzione in cache ********************************
	//***** - in questo modo posso testare la relativa funzione di emulazione di rm *****
	printf("\n\n*** Prova Emulazione ***\n");
	int dest = 10;
//	int dest = 0x7fffffff;	// test Overflow Flag
	int* source = a;
	printf("dest = %d, *source = %d\n", dest, *source);
	printf("*** Emulazione (ADD dest, *source) ***\n");
#if PRINT_FLAGS == 1
	unsigned int the_flags_pre, the_flags_post;
	asm volatile ( 
			"pushf;"
			"pop	%%eax;"
			: "=a" (the_flags_pre)
	);
#endif
	asm volatile ( 
			"jmp	ADD_label;"		// serve per dividere il BB e fare in modo che la ADD vada in cache
			"jmp	ADD_END_label;"		// serve per dividere il BB e fare in modo che la ADD vada in cache
		"ADD_label:"
			"addl 	(%%ebx), %%eax;"
		"ADD_END_label:"
			: "=a" (dest)
			: "a" (dest), "b" (source)
	);
#if PRINT_FLAGS == 1
	asm volatile ( 
			"pushf;"
			"pop	%%eax;"
			: "=a" (the_flags_post)
	);
	printf("the_flags_pre = %08x\n", the_flags_pre);
	printf("the_flags_post = %08x\n", the_flags_post);
#endif
	printf("dest = %d\n", dest);
	
	
#if rm_DEBUG == 1
	if (rm_make_dump_file("dump2.txt") == -1) {
		printf("main: rm_make_dump_file error!!\n");
		return -1;
	}
#endif
	
	
	
	printf("MAIN - end\n");
	
	return 0;
}
