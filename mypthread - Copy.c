// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define NEW 0 //might do the same thing as ready
#define READY 1
#define RUNNING 2
#define WAITING 3 
#define TERMINATED 4

#define QUANTUM 25 //25 milliseconds

static int init = 0; //if 0, then the timer and stuff has not been initialized
static int mypthread_count=0;
static int active=0;
static tcb * sch_thread_cb; //this is the scheduler thread; when a thread yields, this thread takes over and swaps
static tcb * current_thread_cb; //a pointer to whatever the current thread is 
static tcb * front;
static tcb * main_thread_cb;
//static tcb * back;
static int list_length=0;

struct itimerval it_quantum;
struct itimerval it_zero;

static ucontext_t * before;

void timer_handler(int sig){
	printf("%s\n", "thyme");
	if(current_thread_cb!=NULL){
    	//getcontext(current_thread_cb->context);
    	//puts("garlic");
    }
    //setcontext(sch_thread_cb->context);
    swapcontext(current_thread_cb->context,sch_thread_cb->context);
}

static void terminate_and_schedule(){
	if(current_thread_cb !=NULL && current_thread_cb->state==RUNNING){ //if the thread is READY then it either just started or yielded
		printf("terminated: thread id  %d\n", current_thread_cb->id);
		current_thread_cb->state=TERMINATED;
		active-=1;
	}
	schedule();
}

int init_thread_cb(tcb * thread) {
	//init_timer();
	ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(context);
	thread->next=NULL;
	thread->prev=NULL;
	thread->context = context;
	thread->state=READY;
	thread->id=mypthread_count;
	char * stack =(void *) malloc(STACK_SIZE);
	//before calling makecontext, we must intitilaize all these 
	//context->uc_link=sch_thread_cb->context; //what this does is when this thread terminates, it will go back to the main scheduling thread
	context->uc_link=sch_thread_cb->context;
	context->uc_stack.ss_sp=stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	//printf(" this thread has id%d\n", mypthread_count);
	mypthread_count++;
	return 0; //returns 0, since there could be errnos in malloc, etc it doesn't return null
}

void shutdown() { //when we've run out of threads, this turns off the timer and cleans up everything
	puts("hags");
	tcb * ptr= front;
	while(ptr!=NULL) { //free all of the nodes except front in the ll; it is asumed that they are all terminated
		if(ptr->next!=NULL) {
			tcb * zombie =ptr->next;
			ptr->next=zombie->next;
			free_thread_cb_resources(zombie);
		}
	}
	//setcontext(before);
	//exit(0);

}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
	       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	active+=1;
	if (init == 0) { //if init is 0, we have'nt started any threads or anything

			sch_thread_cb=(tcb *) malloc(sizeof(tcb));
			main_thread_cb=(tcb *) malloc(sizeof(tcb));
			init_thread_cb(sch_thread_cb);
			makecontext(sch_thread_cb->context,&schedule,0);
		it_zero.it_value.tv_sec = 0; //set up the sero timer
    	it_zero.it_value.tv_usec = 0;
    	ucontext_t main;
    	//getcontext(&main);
    	//main=(ucontext_t*) malloc(sizeof(ucontext_t));
    	main_thread_cb->context=(ucontext_t*) malloc(sizeof(ucontext_t));
    	main_thread_cb->state=RUNNING;
    	getcontext(main_thread_cb->context);
    	init=1;
    	main_thread_cb->next=front;
		front=main_thread_cb;
    	//swapcontext(main_thread_cb->context,sch_thread_cb->context);
    	//setcontext(sch_thread_cb->context);
    	//setitimer(ITIMER_PROF,&it_quantum,NULL);
	}
	tcb * thread_cb=(tcb*) malloc(sizeof(tcb));
	ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(context);
	thread_cb->next=NULL;
	thread_cb->prev=NULL;
	thread_cb->context = context;
	thread_cb->state=READY;
	thread_cb->id=++mypthread_count;
	*thread=thread_cb->id;
	char * stack =(void *) malloc(STACK_SIZE);
	//before calling makecontext, we must intitilaize all these 
	//context->uc_link=sch_thread_cb->context; //what this does is when this thread terminates, it will go back to the main scheduling thread
	context->uc_link=sch_thread_cb->context;
	context->uc_stack.ss_sp=stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	//printf(" this thread has id%d\n", mypthread_count);
	thread_cb->context->uc_link=sch_thread_cb->context;
	if(arg==NULL){
		makecontext(thread_cb->context,(void (*)()) function,0);
	} else { //parallel_cal,external_cal and vector_mutiply all take 1 arg, we're going to assume that we dont have to worry about more
		//printf("x is %d\n", arg);
		makecontext(thread_cb->context,(void (*)()) function,1,arg);
	}

	//puts this new thread at the head of the thread linked list
	if(front==NULL){
		front=thread_cb;
		current_thread_cb=thread_cb;
	}else {
		thread_cb->next=front;
		front=thread_cb;
	}
	//swapcontext(main_thread_cb->context,sch_thread_cb->context);

	setcontext(sch_thread_cb->context);
    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context
	if(current_thread_cb==NULL) {
		return -1;
	}
	current_thread_cb->state=READY; //sets to READY, so the schedular won't terminate it
	//printf("%s\n", "yileding");
	getcontext(current_thread_cb->context);
	//printf("%s\n", "line 167");
	setitimer(ITIMER_PROF,&it_quantum,NULL);
	//swapcontext(current_thread_cb->context, sch_thread_cb->context); //swapcontext(ucontext_t *oucp, ucontext_t *ucp)-Transfersto ucp and saves curent state into oucp
	//getcontext(current_thread_cb->context);
	setcontext(sch_thread_cb->context);
	//printf("%s\n", "171");
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) { //going to assume this value ptr is always NULL
	current_thread_cb->state=TERMINATED;
	if(value_ptr){
        current_thread_cb->retval = value_ptr;
    } 
        // turn the timer off
    setitimer(ITIMER_PROF,&it_zero,NULL);
	setcontext(sch_thread_cb->context); //swaps to sch_thread_cb context, which sets it to be terminated
	//
}

//frees thread resources
// separate from mypthread_exit so we can reuse this for cleaning in sched_stcf
void free_thread_cb_resources(tcb * thread_cb) {
	free(thread_cb->context->uc_stack.ss_sp);
	free(thread_cb->context);
}


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule(int signum) {
	struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGPROF, &sa, NULL);    

    it_quantum.it_value.tv_sec = QUANTUM/1000;
    it_quantum.it_value.tv_usec = (QUANTUM*1000) % 1000000;
	while (1) { //if we try to schedule and there are no threads we're done :)
		//printf("%s\n", "loop");
		setitimer(ITIMER_PROF,&it_quantum,NULL);
		//printf("%s\n", "smash");
		//getcontext(sch_thread_cb->context);
		//printf("%s\n", "dash");
		//swapcontext(sch_thread_cb->context, main_thread_cb->context);
		//setcontext(main_thread_cb->context);
		//printf("%s\n","hulk smash" );
		sched_stcf();
		printf("%s\n", "always smash");
		if(current_thread_cb==NULL || current_thread_cb->state==TERMINATED) {
			shutdown();
		}else{
			//printf("%s\n", "love to smash");
			swapcontext(sch_thread_cb->context, current_thread_cb->context);
			if(current_thread_cb!=NULL){ //just to keep track of things
				printf("id %d elapsed %d\n", current_thread_cb->id,current_thread_cb->elapsed);
				current_thread_cb->elapsed++;
			}
		}
	}
	//setcontext(main_thread_cb->context);

}
/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	/*
	For the first scheduling algorithm, you are required to implement a pre-emptive SJF, which is also known
as STCF. Unfortunately, you may have noticed that our scheduler DOES NOT know how long a thread will
run for completion of job. Hence, in our scheduler, we could book-keep the time quantum each thread
has to run; this is on the assumption that the more time quantum a thread has run, the longer
this job will run to finish. Therefore, you might need a generic "QUANTUM" value defined possibly in
mypthread.h, which denotes the minimum window of time after which a thread can be context switched
out of the CPU. Let’s assume each quantum is 5 milliseconds; depending on your scheduler logic, one could
context switch out a thread after one quantum or more than one quantum. To implement a mechanism
like this, you might also need to keep track of how many time quantums each thread has ran for*/
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	//we check for front==null in schedule()
	//printf("%s\n", "sched");
	if(current_thread_cb!=NULL){
		current_thread_cb->state=READY;//now this old thread is no longer running- its just ready
	}
	while(front!=NULL && front->state==TERMINATED) {
		tcb* zombie=front;
		front=front->next;
		free_thread_cb_resources(zombie);
	}
	//printf("%s\n", "not null");
	tcb * ptr=front;
	if(ptr==NULL) {
		return;
	}
	tcb * lowest=ptr;
	while(ptr!=NULL) {
		if(ptr->elapsed<lowest->elapsed && ptr->state==READY){
			lowest=ptr;
		}
		ptr=ptr->next;
	}
	current_thread_cb=lowest; //changes current thread to point t the next thread to be executed
	if(current_thread_cb!=NULL && current_thread_cb->state==READY){
		current_thread_cb->state=RUNNING;
	}
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	//aparently only Grad OS needs to do this nice
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need
