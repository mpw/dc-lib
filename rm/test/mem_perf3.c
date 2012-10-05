/* ============================================================================
 *  mem_perf3.c
 * ============================================================================
 *
 *  Note: 	memory performance test
 *		(requires rm library compilated with: "-D rm_STAT=0 -D rm_DEBUG=0 -D PATCH_DEBUG=0")
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "rm.h"

/*defines*/
#define N 10000000

/*global variables*/
int write_num, read_num;

void read_handler(void* rm_addr, size_t size) {
//	printf("*** READ_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
	read_num += 1;
}

void write_handler(void* rm_addr, size_t size) {
//	printf("*** WRITE_HANDLER *** rm_addr = %p, size = %d\n", rm_addr, size);
	write_num += 1;
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (tv.tv_usec*0.000001);
}

double read_loop(int* array) {
	int i, value;
    
	/*fetches initial time*/
	double start_time = get_time();
	
    /*performs N reads from mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		value = *(array + i);
	}
	
    /* return elapsed time */
	return get_time()-start_time;
}

double write_loop(int* array) {
	int i;

	/*fetches initial time*/
	double start_time = get_time();

	/*performs N writes to mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		*(array + i) = i;
	}

	/* return elapsed time */
	return get_time()-start_time;
}

int main() {
	size_t shadow_rec_size = 32;
	size_t shadow_wordsize = 4;
	
	int* prot_array;
	int* unprot_array;
	int* unprot_array2;
	
	double prot_write_times;
	double prot_read_times;
	double unprot_write_times;
	double unprot_read_times;
	double unprot_write_times2;
	double unprot_read_times2;
	
	printf("MAIN - start\n");
	
	/*initializes rm*/
	if (rm_init(read_handler, write_handler, NULL, NULL, shadow_rec_size, shadow_wordsize) == -1) {
		printf("[main] ERROR: rm_init error\n");
		return -1;
	}
	
	printf("N is %d\n", N);
	
	/*allocates protected mem*/
	prot_array = (int*)rm_malloc(N * sizeof(int));
	if(prot_array == NULL)
		printf("[main] ERROR: Could not allocate protected mem\n");
	
	/*allocates unprotected mem*/
	unprot_array = (int*)malloc(N * sizeof(int));
	if(unprot_array == NULL)
		printf("[main] ERROR: Could not allocate unprotected mem\n");
	
	/*allocates unprotected mem2*/
	unprot_array2 = (int*)malloc(N * sizeof(int));
	if(unprot_array2 == NULL)
		printf("[main] ERROR: Could not allocate unprotected mem2\n");
	
	
	/* UNPROTECTED WRITE LOOP */
	printf("Unprotected write loop\n");
	unprot_write_times = write_loop(unprot_array);
	
	/* PROTECTED WRITE LOOP */
	printf("Protected write loop\n");
	prot_write_times = write_loop(prot_array);
	
	/* UNPROTECTED WRITE LOOP 2 */
	printf("Unprotected write loop 2\n");
	unprot_write_times2 = write_loop(unprot_array2);
	
	/* UNPROTECTED READ LOOP */
	printf("Unprotected read loop\n");
	unprot_read_times = read_loop(unprot_array);
	
	/* PROTECTED READ LOOP */
	printf("Protected read loop\n");
	prot_read_times = read_loop(prot_array);
	
	/* UNPROTECTED READ LOOP 2 */
	printf("Unprotected read loop 2\n");
	unprot_read_times2 = read_loop(unprot_array2);
	
	
	/*prints debug controls*/
	printf("\n*** DEBUG ***\n");
	printf("write_num = %d\n", write_num);
	printf("read_num = %d\n", read_num);
	printf("*(prot_array) = %d\n", *(prot_array));
	printf("*(prot_array + 5) = %d\n", *(prot_array + 5));
	printf("*(prot_array + N - 1) = %d\n", *(prot_array + N - 1));
	
	/*prints results*/
	printf("\n*** RESULTS ***\n");
	printf("Seconds taken for %d unprotected writes: %f\n", N, unprot_write_times);
	printf("Seconds taken for %d protected writes: %f\n", N, prot_write_times);
	printf("Seconds taken for %d unprotected writes (2): %f\n", N, unprot_write_times2);
	printf("Seconds taken for %d unprotected reads: %f\n", N, unprot_read_times);
	printf("Seconds taken for %d protected reads: %f\n", N, prot_read_times);
	printf("Seconds taken for %d unprotected reads (2): %f\n", N, unprot_read_times2);
	printf("Ratio between prot and unprot writes: %f\n", prot_write_times/unprot_write_times);
	printf("Ratio between prot and unprot reads: %f\n", prot_read_times/unprot_read_times);
	printf("Ratio between unprot(2) and unprot writes: %f\n", unprot_write_times2/unprot_write_times);
	printf("Ratio between unprot(2) and unprot reads: %f\n", unprot_read_times2/unprot_read_times);
	
	/*clean up*/
	rm_free(prot_array);
	free(unprot_array);
	
	printf("MAIN - end\n");
	
	return 0;
}
