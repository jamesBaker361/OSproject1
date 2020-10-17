#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
int i_hate_it_here(int x) {
	int i=0;
	printf("\n");
	while(i++<100000){
		printf("%d", x);
	}
	mypthread_yield();
	return 0;
}

int test(mypthread_t* thread) {
	while(1) {
		thread->id;
	}
}

int main(int argc, char **argv) {

	/* Implement HERE 
	mypthread_t* thread = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread,NULL,&i_hate_it_here,50);
	mypthread_t* thread1 = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread1,NULL,&i_hate_it_here,20);
	*/
	mypthread_t* thread2 = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread2,NULL,&i_hate_it_here,3);
	mypthread_t* thread3 = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread3,NULL,&i_hate_it_here,7);
	// Kill some time
	int z=0;
	while(z++<100000000){
		printf("%s", "z");
	}
	printf("%s\n","all done" );
	return 0;
}
