// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
#define NEW 0
#define READY 1
#define RUNNING 2
#define WAITING 3
#define TERMINATED 4

#define QUANTUM 2500 //25 milliseconds

static int mypthread_count = 0;
static mypthread_t * sch_thread; //this is the scheduler thread; when a thread yields, this thread takes over and swaps
static mypthread_t * current_thread; //a pointer to whatever the current thread is 
static mypthread_t * front;
//static mypthread_t * back;
static int list_length=0;

static struct sigaction sa;
static struct itimerval timer;

static void do_nothing(){
	if(current_thread !=NULL){
		printf("terminated: thread id  %d\n", current_thread->id);
		current_thread->state=TERMINATED;
	}
	while(1);
}

int init_thread(mypthread_t * thread) {
	//init_timer();
	ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(context);
	thread->next=NULL;
	thread->prev=NULL;
	thread->context = context;
	thread->state=READY;
	thread->id=mypthread_count;
	char * stack =(void *) malloc(STACK_SIZE);
	thread->stack=stack;
	//before calling makecontext, we must intitilaize all these 
	context->uc_link=sch_thread->context;
	context->uc_stack.ss_sp=stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	printf(" this thread has id%d\n", mypthread_count);
	mypthread_count++;
	return 0; //returns 0, since there could be errnos in malloc, etc it doesn't return null
}

int init_timer() {
	//struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &schedule;
	sigaction (SIGPROF, &sa, NULL);
	//signal(SIGPROF, schedule);
	// Create timer struct
	//struct itimerval timer;

	// Set up what the timer should reset to after the timer goes off
	timer.it_interval.tv_usec = QUANTUM; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = QUANTUM;
	timer.it_value.tv_sec = 0;

	// Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &timer, NULL);
}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
	       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	if (mypthread_count == 0) { //if there are no threads, then this has only been invoked once and we need to make a shceuler thread
		init_timer();
		sch_thread=(mypthread_t *) malloc(sizeof(mypthread_t));
		init_thread(sch_thread);
		makecontext(sch_thread->context,&do_nothing,0);
	}
	init_thread(thread);
	if(arg==NULL){
		makecontext(thread->context,(void (*)()) function,0);
	} else { //parallel_cal,external_cal and vector_mutiply all take 1 arg, we're going to assume that we dont have to worry about more
		//printf("x is %d\n", arg);
		makecontext(thread->context,(void (*)()) function,1,arg);
	}

	//puts this new thread at the head of the thread linked list
	if(front==NULL){
		front=thread;
	}else {
		thread->next=front;
		front=thread;
	}
	//schedule();
    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context
	current_thread->state=READY;
	swapcontext(current_thread->context, sch_thread->context); //swapcontext(ucontext_t *oucp, ucontext_t *ucp)-Transfers control to ucp and saves the current execution state into oucp

	// YOUR CODE HERE
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread
	//current_thread->state=TERMINATED;
	free(&(current_thread->context->uc_stack));
	free(current_thread->context);
	//
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
	if (front ==NULL) {
		exit(0);
	}
	if(current_thread!=NULL){
		printf("thread->id is %d elapsed %d quanta \n", current_thread->id,current_thread->elapsed);
		current_thread->elapsed++;
		if(current_thread->elapsed>20) {
			exit(0);
		}
	}
	if(front->next!=NULL){
		sched_stcf();
	}else{
		current_thread=front;
	}
	if(current_thread->state==TERMINATED) {
		exit(0);
	}else{
		setcontext(current_thread->context);
	}

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

	//sch_thread->next is the head of the linked list;
	while(front->state==TERMINATED) {
		front=front->next;
	}
	mypthread_t * ptr=front;
	if(ptr==NULL) {
		return;
	}
	mypthread_t * lowest=ptr;
	while(ptr!=NULL) {
		if(ptr->elapsed<lowest->elapsed){
			lowest=ptr;
		}
		ptr=ptr->next;
	}
	current_thread=lowest;

}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	//aparently only Grad OS needs to do this nice
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need
