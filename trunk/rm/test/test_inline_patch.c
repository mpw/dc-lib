#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "rm.h"

// Etichette per il codice delle patch inline
extern char handler_call_start_RA;
extern char handler_call_end_RA;
extern char handler_call_RA;

extern char handler_call_start_RC;
extern char handler_call_end_RC;
extern char handler_call_RC;

extern char handler_call_start_WA;
extern char handler_call_end_WA;
extern char handler_call_WA;

extern char handler_call_start_WC;
extern char handler_call_end_WC;
extern char handler_call_WC;


// Handler per le patch di default
void read_handler(void* rm_addr, size_t size) {
	printf("*** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

void write_handler(void* rm_addr, size_t size) {
	printf("*** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

// Handler per le patch inline
void read_handler_inline(void* rm_addr, size_t size) {
	printf("(inline)(inline)(inline) *** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

void write_handler_inline(void* rm_addr, size_t size) {
	printf("(inline)(inline)(inline) *** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}


#if 1
// Macro per il codice delle patch inline
#define INLINE_PATCH_CODE(case, reg, reg2) \
	__asm__ __volatile__ ( \
			"jmp handler_call_end_"case";" \
	); \
	__asm__ __volatile__ ( \
		"handler_call_start_"case":" \
	/***		"call	HANDLER;" */ \
			"push	%"reg2";" \
			"push	%edx;" \
			"push	$0x4;"			/* parameter #2 */ \
			"push	%"reg";"		/* parameter #1 */ \
		"handler_call_"case":" \
			"movl	$0x08000000, %"reg";"	/* l'indirizzo di memoria sarà modificato */ \
			"call	*%"reg";" \
			"addl	$0x8, %esp;"		/* remove the parameters from stack */ \
			"pop	%edx;" \
			"pop	%"reg2";" \
	/***		"call	HANDLER;" */ \
		"handler_call_end_"case":" \
	);

// Funzione per l'inserimento degli indirizzi degli handler nel codice delle patch inline
// N.B. questa funzione contiene anche il codice delle patch inline (non viene mai eseguito, serve solo per essere copiato)
void inline_patch() {
	// Sprotegge pagina del codice delle patch inline
	unsigned int page_start = ((unsigned int)&handler_call_start_RA & ~(getpagesize()-1));
	unsigned int page_end = ((unsigned int)(&handler_call_end_WC-1) & ~(getpagesize()-1)) + getpagesize();
	if(-1 == mprotect((void*)page_start, page_end-page_start, PROT_READ|PROT_WRITE|PROT_EXEC)) {
		printf("[inline_patch]: mprotect() failed!\n");
		exit(1);
	}
	
	// Patch del codice delle patch inline (inserimento degli indirizzi degli handler)
	unsigned int read_handler_addr = (unsigned int)read_handler_inline;
	unsigned int write_handler_addr = (unsigned int)write_handler_inline;
	memcpy(&handler_call_RA + 1, &read_handler_addr, 4);
	memcpy(&handler_call_RC + 1, &read_handler_addr, 4);
	memcpy(&handler_call_WA + 1, &write_handler_addr, 4);
	memcpy(&handler_call_WC + 1, &write_handler_addr, 4);
	
	// Reimposta protezione pagina del codice delle patch inline
	if(-1 == mprotect((void*)page_start, page_end-page_start, PROT_READ|PROT_EXEC)) {
		printf("[inline_patch]: mprotect() failed!\n");
		exit(1);
	}
	
	// Codice delle patch inline (non viene mai eseguito, serve solo per essere copiato)
	INLINE_PATCH_CODE("RA", "eax", "ecx")
	INLINE_PATCH_CODE("RC", "ecx", "eax")
	INLINE_PATCH_CODE("WA", "eax", "ecx")
	INLINE_PATCH_CODE("WC", "ecx", "eax")
	
	return;
}
#endif


#if 0
#define INLINE_PATCH_CODE(case, reg, reg2, input_reg, mode) \
	__asm__ __volatile__ ( \
			"jmp handler_call_end_"case";" \
	); \
	__asm__ __volatile__ ( \
		"handler_call_start_"case":" \
	/***		"call	HANDLER;" */ \
			"push	%"reg2";" \
			"push	%edx;" \
			"push	$0x4;"			/* parameter #2 */ \
			"push	%"reg";"		/* parameter #1 */ \
	); \
	__asm__ __volatile__ ( \
		"handler_call_"case":" \
		/*	"movl	$0x08000000, %"reg";"	/* l'indirizzo di memoria sarà modificato */ \
			"call	*%%"reg";" \
			: \
			: input_reg (mode##_handler_inline) \
	); \
	__asm__ __volatile__ ( \
			"addl	$0x8, %esp;"		/* remove the parameters from stack */ \
			"pop	%edx;" \
			"pop	%"reg2";" \
	/***		"call	HANDLER;" */ \
		"handler_call_end_"case":" \
	);

void inline_patch() {
	INLINE_PATCH_CODE("RA", "eax", "ecx", "a", read)
	INLINE_PATCH_CODE("RC", "ecx", "eax", "c", read)
	INLINE_PATCH_CODE("WA", "eax", "ecx", "a", write)
	INLINE_PATCH_CODE("WC", "ecx", "eax", "c", write)
	return;
}
#endif



int main() {
	size_t shadow_rec_size = 32;
	size_t shadow_wordsize = 4;
	
	void* patch_table[rm_patch_num][rm_size_num];
	size_t size_table[rm_patch_num][rm_size_num];
	
	printf("MAIN - start\n");
	
	// Inserimento degli indirizzi degli handler nel codice delle patch inline
	inline_patch();
	
	// Creazione tabelle per patch inline
	patch_table[rm_read_eax_patch][rm_size_4] = &handler_call_start_RA;
	size_table[rm_read_eax_patch][rm_size_4] = &handler_call_end_RA - &handler_call_start_RA;
	patch_table[rm_read_ecx_patch][rm_size_4] = &handler_call_start_RC;
	size_table[rm_read_ecx_patch][rm_size_4] = &handler_call_end_RC - &handler_call_start_RC;
	patch_table[rm_write_eax_patch][rm_size_4] = &handler_call_start_WA;
	size_table[rm_write_eax_patch][rm_size_4] = &handler_call_end_WA - &handler_call_start_WA;
	patch_table[rm_write_ecx_patch][rm_size_4] = &handler_call_start_WC;
	size_table[rm_write_ecx_patch][rm_size_4] = &handler_call_end_WC - &handler_call_start_WC;
	
	// RM init
//	if (rm_init(NULL, NULL, NULL, NULL, shadow_rec_size, shadow_wordsize) == -1) { // no handler
	if (rm_init(NULL, NULL, patch_table, size_table, shadow_rec_size, shadow_wordsize) == -1) { // inline patch
//	if (rm_init(read_handler, write_handler, NULL, NULL, shadow_rec_size, shadow_wordsize) == -1) { // default patch
		printf("main: rm_init error!!\n");
		return -1;
	}
	
	// DEBUG patch inline
	printf("\n\n\nDEBUG patch inline\n");
	printf("read_handler_inline = %08x\n", (unsigned int)read_handler_inline);
	printf("write_handler_inline = %08x\n", (unsigned int)write_handler_inline);
	printf("size_table[rm_read_eax_patch][rm_size_4] = %d\n", size_table[rm_read_eax_patch][rm_size_4]);
	printf("size_table[rm_read_ecx_patch][rm_size_4] = %d\n", size_table[rm_read_ecx_patch][rm_size_4]);
	printf("size_table[rm_write_eax_patch][rm_size_4] = %d\n", size_table[rm_write_eax_patch][rm_size_4]);
	printf("size_table[rm_write_ecx_patch][rm_size_4] = %d\n", size_table[rm_write_ecx_patch][rm_size_4]);
	
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
	
#if rm_DEBUG == 1
	if (rm_make_dump_file("dump2.txt") == -1) {
		printf("main: rm_make_dump_file error!!\n");
		return -1;
	}
#endif
	
	
	
	printf("MAIN - end\n");
	
	return 0;
}
