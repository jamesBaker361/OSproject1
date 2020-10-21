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

void* testFunc(void* params){

    puts("main(): in test func");

    //while(1){
        //puts("testFunc1");
    //}
    mypthread_exit(NULL);
    puts("haram");
    return 0;
}

void* testFunc2(void* params){

    puts("main(): in test func2");

    //while(1){
        //puts("testFunc2");
    //}
    mypthread_yield();
    puts("nice yield");
    //mypthread_exit(NULL);
    return 0;
}

void* testFunc3(void* params){
    puts("main(): in test func3");

    while(1){
    	//printf("%s\n", "threepeat");
    }
    return 0;
}

void * pregame_func(void * params) {
	int y=0;
	while(y++<300000);
	puts("inshallah");
	return 0;
}

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
		//thread->id;
	}
}

int main(int argc, char **argv) {

	/* Implement HERE 
	mypthread_t* thread = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread,NULL,&i_hate_it_here,50);
	mypthread_t* thread1 = (mypthread_t*) malloc(sizeof(mypthread_t));
	mypthread_create(thread1,NULL,&i_hate_it_here,20);
	
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
	*/
	    mypthread_t* test; 
	    mypthread_t *test2;
	    mypthread_t *test3;
	    mypthread_t *pregame;

    puts("main(): beore 1st create");
    test=mypthread_create(&test,NULL, testFunc,NULL);
    puts("main(): after 1st create");

    puts("main(): before 2nd create");
    test2=mypthread_create(&test2,NULL,testFunc2,NULL);
    puts("main(): after 2nd create");

    puts("mains() before 3rdcreate");
    test3=mypthread_create(&test3, NULL, testFunc3,NULL);
    puts("main() after 3rd create");

    puts("mains() before 4rdcreate");
    pregame=mypthread_create(&pregame, NULL, testFunc3,NULL);
    puts("main() after 4rd create");
    
    //mypthread_join(test,NULL);

    puts("main(): we are after 1st join");
    mypthread_join(test,NULL);
    puts("bean");
    //rpthread_join(test2,NULL);
    while(1){
    	//printf("%s\n", "gucci main");
    }
}
