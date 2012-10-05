#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "rm.h"

#define NOP_PATCH 1

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

int g_var;

// Handler per le patch di default
void read_handler(void* rm_addr, size_t size) {
    //if (g_var==0)
      //  printf("*** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

void write_handler(void* rm_addr, size_t size) {
	printf("*** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

// Handler per le patch inline
void read_handler_inline(void* rm_addr, size_t size) {
	//printf("*** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

void write_handler_inline(void* rm_addr, size_t size) {
	printf("(inline)(inline)(inline) *** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
}

//runs the test loop...
double foo (void *inPtr, unsigned int n)
{
    struct timeval tvBeginLoop, tvEndLoop;
    double loopTime;
    
    int *a=(int *)inPtr;
    int b, i;
    
    /*fetches initial time*/
    gettimeofday(&tvBeginLoop, NULL);
    
    for (i=0; i<n; i++)
    {
        /*g_var++;
        if (g_var>=M)
            g_var=0;*/
        
        b=*a;   
    }
    
    /*fetches final time*/
    gettimeofday(&tvEndLoop, NULL);
    
    loopTime = tvEndLoop.tv_sec + (tvEndLoop.tv_usec*0.000001)
            -tvBeginLoop.tv_sec - (tvBeginLoop.tv_usec*0.000001);
            
    return loopTime;
}


#if NOP_PATCH==1
// Macro per il codice delle patch inline
#define INLINE_PATCH_CODE(case, reg, reg2) \
    __asm__ __volatile__ ( \
    "handler_call_start_"case":" \
            "nop;" \
    "handler_call_end_"case":" \
    );
#endif

#if NOP_PATCH==0
// Macro per il codice delle patch inline
#define INLINE_PATCH_CODE(case, reg, reg2) \
	__asm__ __volatile__ ( \
			"jmp handler_call_end_"case";" \
	); \
	__asm__ __volatile__ ( \
		"handler_call_start_"case":" \
	/***		"call	HANDLER;" */ \
    /*        "cmpl   $0x00000000, 0x08000000;" /*addr of g_var to be inserted at runtime...*/ \
    /*        "jnz    handler_call_end_"case";" /* if g_var != 0 skips inline handler...*/ \
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
#endif

// Funzione per l'inserimento degli indirizzi degli handler nel codice delle patch inline
// N.B. questa funzione contiene anche il codice delle patch inline (non viene mai eseguito, serve solo per essere copiato)
void inline_patch() {
#if NOP_PATCH==0
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
    
    //copies address of g_var...
    //*(unsigned*)(&handler_call_start_RA + 2) = &g_var;
    //*(unsigned*)(&handler_call_start_RC + 2) = &g_var;
	
	// Reimposta protezione pagina del codice delle patch inline
	if(-1 == mprotect((void*)page_start, page_end-page_start, PROT_READ|PROT_EXEC)) {
		printf("[inline_patch]: mprotect() failed!\n");
		exit(1);
	}
#endif

	// Codice delle patch inline (non viene mai eseguito, serve solo per essere copiato)
	INLINE_PATCH_CODE("RA", "eax", "ecx")
	INLINE_PATCH_CODE("RC", "ecx", "eax")
	INLINE_PATCH_CODE("WA", "eax", "ecx")
	INLINE_PATCH_CODE("WC", "ecx", "eax")
	
	return;
}

#define N 100000000
#define M 1000

int main() {
	size_t shadow_rec_size = 32;
	size_t shadow_wordsize = 4;
    
    void *rPtr, *nPtr;
        
    double X_Time, Patch_Time, Xprime_Time, XAD_Time;
	
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
	
#if rm_DEBUG == 1
    if (rm_make_dump_file("dump1.txt") == -1) {
        printf("main: rm_make_dump_file error!!\n");
        return -1;
    }
#endif
	
	rPtr=rm_malloc (sizeof (int));
    nPtr=malloc (sizeof (int));
    
    //X test loop...
    X_Time=foo (nPtr, N);
    
    //causes foo() to be patched...
    Patch_Time=foo (rPtr, 1);
    
    //X' test loop...
    Xprime_Time=foo (nPtr, N);
    
    //X+A(+D) test loop...
    XAD_Time=foo (rPtr, N);
    
    printf ("N = %u\n", N);
    printf("X = %f\n", X_Time);
    printf("Patch_Time = %f\n", Patch_Time);
    printf("X' = %f\n", Xprime_Time);
    
    #if NOP_PATCH==1
    printf("X+A = %f\n", XAD_Time);
    #endif
    #if NOP_PATCH==0
    printf("X+A+D = %f\n", XAD_Time);
    #endif
	
	rm_free (rPtr);
    free (nPtr);
    
#if rm_DEBUG == 1
    if (rm_make_dump_file("dump2.txt") == -1) {
        printf("main: rm_make_dump_file error!!\n");
        return -1;
    }
#endif
	
	printf("MAIN - end\n");
	
	return 0;
}
