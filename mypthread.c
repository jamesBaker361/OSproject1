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
#define WAITING 3 //might do the same thing as ready
#define TERMINATED 4

#define QUANTUM 1 //25 milliseconds

static int init = 0; //if 0, then the timer and stuff has not been initialized
static int mypthread_count=0;
static mypthread_t * sch_thread; //this is the scheduler thread; when a thread yields, this thread takes over and swaps
static mypthread_t * current_thread; //a pointer to whatever the current thread is 
static mypthread_t * front;
//static mypthread_t * back;
static int list_length=0;

static struct sigaction sa;
static struct itimerval timer;

static ucontext_t * before;

static void do_nothing(){
	if(current_thread !=NULL && current_thread->state==RUNNING){ //if the thread is READY then it either just started or yielded
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
	//before calling makecontext, we must intitilaize all these 
	context->uc_link=sch_thread->context; //what this does is when this thread terminates, it will go back to the main scheduling thread
	context->uc_stack.ss_sp=stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	//printf(" this thread has id%d\n", mypthread_count);
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

void shutdown() { //when we've run out of threads, this turns off the timer and cleans up everything
	printf("%s\n", "shutdown");
	
	init=0;
	mypthread_t * ptr= front;
	while(ptr!=NULL) { //free all of the nodes except front in the ll; it is asumed that they are all terminated
		if(ptr->next!=NULL) {
			mypthread_t * zombie =ptr->next;
			ptr->next=zombie->next;
			free_thread_resources(zombie);
		}
	}
	init=0;
	timer.it_interval.tv_usec=0;
	timer.it_value.tv_usec =0;
	setitimer(ITIMER_PROF, &timer, NULL); //turns off the timer
	//setcontext(before);
	exit(0);

}

static void finish() {
}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
	       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	if (init == 0) { //if init is 0, we have'nt started any threads or anything
		before=(ucontext_t*) malloc(sizeof(ucontext_t));
		init=1;
		//getcontext(before);
		char * stack =(void *) malloc(STACK_SIZE);
		//before->uc_link=sch_thread->context; //what this does is when this thread terminates, it will go back to the main scheduling thread
		before->uc_stack.ss_sp=stack;
		before->uc_stack.ss_size=STACK_SIZE;
		before->uc_stack.ss_flags=0;
		makecontext(before,NULL,0);
		init=1;
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
    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context
	if(current_thread==NULL) {
		return -1;
	}
	current_thread->state=READY; //sets to READY, so the schedular won't terminate it
	getcontext(current_thread->context);
	//swapcontext(current_thread->context, sch_thread->context); //swapcontext(ucontext_t *oucp, ucontext_t *ucp)-Transfersto ucp and saves curent state into oucp
	//getcontext(current_thread->context);
	setcontext(sch_thread->context);
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) { //going to assume this value ptr is always NULL
	current_thread->state=RUNNING;
	setcontext(sch_thread->context); //swaps to sch_thread context, which sets it to be terminated
	//
}

//frees thread resources
// separate from mypthread_exit so we can reuse this for cleaning in sched_stcf
void free_thread_resources(mypthread_t * thread) {
	free(thread->context->uc_stack.ss_sp);
	free(thread->context);
	free(thread);
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
	printf("%s\n", "time 2 schedule");
	if (front ==NULL) { //if we try to schedule and there are no threads we're done :)
		shutdown();
	}
	if(current_thread!=NULL){ //just to keep track of things
		printf("thread->id is %d elapsed %d quanta \n", current_thread->id,current_thread->elapsed);
		current_thread->elapsed++;
		if(current_thread->elapsed>20) { //only for testing purposes
			//exit(0);
		}
		//getcontext(current_thread->context); //saves the current context to current_thread-> context before we swap to a new curent_thread
	}
	sched_stcf();
	if(current_thread==NULL || current_thread->state==TERMINATED) {
		shutdown();
	}else{
		printf("%s\n","not null" );
		if(current_thread->state==TERMINATED){
			printf("%s\n", "done");
		}
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

	//we check for front==null in schedule()
	//printf("%s\n", "not null");
	while(front!=NULL && front->state==TERMINATED) {
		mypthread_t* zombie=front;
		front=front->next;
		free_thread_resources(zombie);
	}
	//printf("%s\n", "not null");
	mypthread_t * ptr=front;
	if(ptr==NULL) {
		return;
	}
	mypthread_t * lowest=ptr;
	while(ptr!=NULL) {
		if(ptr->elapsed<lowest->elapsed && ptr->state!=TERMINATED){
			lowest=ptr;
		}
		ptr=ptr->next;
	}
	if(current_thread!=NULL){
		current_thread->state=READY;
		//getcontext(current_thread->context);
	}
	current_thread=lowest; //changes current thread to point t the next thread to be executed
	if(current_thread!=NULL && current_thread->state==READY){
		current_thread->state=RUNNING;
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
