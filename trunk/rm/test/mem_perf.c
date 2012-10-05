/* ============================================================================
 *  mem_perf.c
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
#define N 1000000

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

int main() {
	size_t shadow_rec_size = 32;
	size_t shadow_wordsize = 4;
	
	int* prot_array;
	int* unprot_array;
	
	double prot_write_times;
	double prot_read_times;
	double unprot_write_times;
	double unprot_read_times;
	
	struct timeval tvBeginProtWrite, tvEndProtWrite;
	struct timeval tvBeginProtRead, tvEndProtRead;
	struct timeval tvBeginUnprotWrite, tvEndUnprotWrite;
	struct timeval tvBeginUnprotRead, tvEndUnprotRead;
	
	int i, value;
	
	printf("MAIN - start\n");
	
	/*initializes rm*/
	if (rm_init(read_handler, write_handler, NULL, NULL, shadow_rec_size, shadow_wordsize) == -1) {
		printf("main: rm_init error!!\n");
		return -1;
	}
	
	printf("N is %d\n", N);
	
	/*allocates protected mem*/
	prot_array = (int*)rm_malloc(N * sizeof(int));
	if(prot_array == NULL)
		printf("[main] ERROR: Could not allocate protected mem!\n");
	
	/*allocates unprotected mem*/
	unprot_array = (int*)malloc(N * sizeof(int));
	if(unprot_array == NULL)
		printf("[main] ERROR: Could not allocate unprotected mem!\n");
	
	
	/* PROTECTED WRITE LOOP */
	printf("Protected write loop\n");
	/*fetches initial time*/
	gettimeofday(&tvBeginProtWrite, NULL);
	/*performs N writes to prot mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		*(prot_array + i) = i;
	}
	/*fetches final time*/
	gettimeofday(&tvEndProtWrite, NULL);
	prot_write_times = tvEndProtWrite.tv_sec + (tvEndProtWrite.tv_usec*0.000001)
			-tvBeginProtWrite.tv_sec - (tvBeginProtWrite.tv_usec*0.000001);
	
	/* UNPROTECTED WRITE LOOP */
	printf("Unprotected write loop\n");
	/*fetches initial time*/
	gettimeofday(&tvBeginUnprotWrite, NULL);
	/*performs N writes to unprot mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		*(unprot_array + i) = i;
	}
	/*fetches final time*/
	gettimeofday(&tvEndUnprotWrite, NULL);
	unprot_write_times = tvEndUnprotWrite.tv_sec + (tvEndUnprotWrite.tv_usec*0.000001)
			-tvBeginUnprotWrite.tv_sec - (tvBeginUnprotWrite.tv_usec*0.000001);
	
	/* PROTECTED READ LOOP */
	printf("Protected read loop\n");
	/*fetches initial time*/
	gettimeofday(&tvBeginProtRead, NULL);
	/*performs N reads from prot mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		value = *(prot_array + i);
	}
	/*fetches final time*/
	gettimeofday(&tvEndProtRead, NULL);
	prot_read_times = tvEndProtRead.tv_sec + (tvEndProtRead.tv_usec*0.000001)
			-tvBeginProtRead.tv_sec - (tvBeginProtRead.tv_usec*0.000001);
	
	/* UNPROTECTED READ LOOP */
	printf("Unprotected read loop\n");
	/*fetches initial time*/
	gettimeofday(&tvBeginUnprotRead, NULL);
	/*performs N reads from unprot mem*/
	for(i=0; i<N; i++) {
		/*performs access*/
		value = *(unprot_array + i);
	}
	/*fetches final time*/
	gettimeofday(&tvEndUnprotRead, NULL);
	unprot_read_times = tvEndUnprotRead.tv_sec + (tvEndUnprotRead.tv_usec*0.000001)
			-tvBeginUnprotRead.tv_sec - (tvBeginUnprotRead.tv_usec*0.000001);
	
	
	/*prints debug controls*/
	printf("\n*** DEBUG ***\n");
	printf("write_num = %d\n", write_num);
	printf("read_num = %d\n", read_num);
	printf("*(prot_array) = %d\n", *(prot_array));
	printf("*(prot_array + 5) = %d\n", *(prot_array + 5));
	printf("*(prot_array + N - 1) = %d\n", *(prot_array + N - 1));
	
	/*prints results*/
	printf("\n*** RESULTS ***\n");
	printf("Seconds taken for %d protected writes: %f\n", N, prot_write_times);
	printf("Seconds taken for %d unprotected writes: %f\n", N, unprot_write_times);
	printf("Seconds taken for %d protected reads: %f\n", N, prot_read_times);
	printf("Seconds taken for %d unprotected reads: %f\n", N, unprot_read_times);
	printf("Ratio between prot and unprot writes: %f\n", prot_write_times/unprot_write_times);
	printf("Ratio between prot and unprot reads: %f\n", prot_read_times/unprot_read_times);
	
	/*clean up*/
	rm_free(prot_array);
	free(unprot_array);
	
	printf("MAIN - end\n");
	
	return 0;
}
