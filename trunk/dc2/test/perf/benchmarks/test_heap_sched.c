#include "heap_sched.h"
#include <stdio.h>
#include <stdlib.h>

#define N 20

int comp(dc_cons* x, dc_cons* y) {
    return (unsigned)x - (unsigned)y;
}

int main() {

    int i;
    void* sched = heap_sched_g->create();

    // create heap
    heap_set_comp(sched, comp);

    // schedule items
    for (i = 0; i < N; i++) {
        int x = 1 + rand() % 20;
        printf("schedule: %d\n", x);
        heap_sched_g->schedule(sched, (dc_cons*)x);
    }

    // pick items
    for (;;) {
        dc_cons* c = heap_sched_g->pick(sched);
        if (c == NULL) break;
        printf("pick: %d\n", (unsigned)c);
    }

    // delete heap
    heap_sched_g->destroy(sched);

    return 0;
}

