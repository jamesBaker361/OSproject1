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

#define QUANTUM 1 //25 milliseconds

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
	//printf("%s\n", "thyme");
	if(current_thread_cb!=NULL){
    	//getcontext(current_thread_cb->context);
    	//puts("garlic");
    }
    //setcontext(sch_thread_cb->context);
    swapcontext(current_thread_cb->context,sch_thread_cb->context);
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
	mypthread_count++;
	return 0; //returns 0, since there could be errnos in malloc, etc it doesn't return null
}

void shutdown() { //when we've run out of threads, this turns off the timer and cleans up everything
	tcb * ptr= front;
	while(ptr!=NULL) { //free all of the nodes except front in the ll; it is asumed that they are all terminated
		if(ptr->next!=NULL) {
			tcb * zombie =ptr->next;
			ptr->next=zombie->next;
			free_thread_cb_resources(zombie);
		}
	}
	printf("shutdown\n");
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
		setitimer(ITIMER_PROF,&it_quantum,NULL);
		sch_thread_cb=(tcb *) malloc(sizeof(tcb));
		main_thread_cb=(tcb *) malloc(sizeof(tcb));
		init_thread_cb(sch_thread_cb);
		makecontext(sch_thread_cb->context,&schedule,0);

		//we make a main context and add it to the rest of the threads
    	ucontext_t main;
    	main_thread_cb->context=(ucontext_t*) malloc(sizeof(ucontext_t));
    	main_thread_cb->state=RUNNING;
    	main_thread_cb->elapsed=0;
    	getcontext(main_thread_cb->context);
    	init=1;
    	main_thread_cb->next=front;
		if(main_thread_cb==front){
			main_thread_cb->next=NULL;
		}
		front=main_thread_cb;

    	it_quantum.it_value.tv_usec = QUANTUM; //set up the timer
    	it_quantum.it_interval.tv_usec = QUANTUM;
    	setitimer(ITIMER_PROF,&it_quantum,NULL);
	}
	tcb * thread_cb=(tcb*) malloc(sizeof(tcb));
	ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(context);
	thread_cb->next=NULL;
	thread_cb->prev=NULL;
	thread_cb->context = context;
	thread_cb->elapsed=0;
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
	thread_cb->context->uc_link=NULL; //sch_thread_cb->context;
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
		if(thread_cb==front){
			thread_cb->next=NULL;
		}
		front=thread_cb;
	}
	swapcontext(main_thread_cb->context,sch_thread_cb->context);
    return(thread_cb->id);
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
	//getcontext(current_thread_cb->context);
	//printf("%s\n", "line 167");
	//setitimer(ITIMER_PROF,&it_quantum,NULL);
	swapcontext(current_thread_cb->context, sch_thread_cb->context); //swapcontext(ucontext_t *oucp, ucontext_t *ucp)-Transfersto ucp and saves curent state into oucp
	//getcontext(current_thread_cb->context);
	//setcontext(sch_thread_cb->context);
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
    //setitimer(ITIMER_PROF,&it_zero,NULL);
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
	//printf("thread id %d\n", thread);
	tcb* temp;
	tcb * ptr=front;
	while(ptr!=NULL){
		printf("joineth");
		if(ptr->id== thread){
			temp=ptr;
			break;
		}
		ptr=ptr->next;
	}
    // infinitely loop to block thread that called this until thread passed into parameters is terminated
    while(temp!=NULL && temp->state != TERMINATED );


    // check if user wants a retval
    if(value_ptr!=NULL){
        *value_ptr = temp->retval;
    }
	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex
	*mutex = *((mypthread_mutex_t*)malloc(sizeof(mypthread_mutex_t)));
	mutex->locked = 0;
	mutex->head = (tcb*)malloc(sizeof(tcb));
	mutex->tail = (tcb*)malloc(sizeof(tcb));
	// form link
	mutex->head->next = mutex->tail;
	mutex->tail->prev = mutex->head;
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
	// use the built-in test-and-set atomic function to test the mutex
	// if the mutex is acquired successfully, enter the critical section
	// if acquiring mutex fails, push current thread into block list and //
	// context switch to the scheduler thread
	while(__sync_lock_test_and_set(&mutex->locked, 1) == 1) {
		current_thread_cb->next = mutex->head->next;
		current_thread_cb->prev = mutex->head;
		mutex->head->next = current_thread_cb;
		current_thread_cb->next->prev = current_thread_cb;
		current_thread_cb->state = WAITING;
    	swapcontext(current_thread_cb->context, sch_thread_cb->context); // is this how we do it?
	}
	if (mutex->locked == 1) {
		mutex->holder = current_thread_cb; 
	}

	return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.
	if (mutex->holder == current_thread_cb && mutex->head->next != mutex->tail) {
		tcb* nextThread = mutex->tail->prev;
		nextThread->prev->next = mutex->tail;
		mutex->tail->prev = nextThread->prev;
		nextThread->prev->next = nextThread->next;
		if (nextThread->next != NULL) {
			nextThread->next->prev = nextThread->prev;
		}
		nextThread->state = READY;

		if (front == NULL) {
			front = (tcb*)malloc(sizeof(tcb));
			front->id = 1;
		}
		nextThread->prev = front;
		nextThread->next = front->next;
		nextThread->next->prev = nextThread;
		front->next = nextThread;	

	}
	mutex->locked = 0;
	mutex->holder = NULL;

	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init
	if (current_thread_cb != mutex->holder) {
		return -1;
	}
	else {
		free(mutex);
	}
	return 0;
};

/* scheduler */
static void schedule(int signum) {
	struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGPROF, &sa, NULL);    
	while (1) { //if we try to schedule and there are no threads we're done :)
		printf("%s\n", "bad scheule");
		main_thread_cb->elapsed++;
		current_thread_cb=main_thread_cb;
		swapcontext(sch_thread_cb->context, main_thread_cb->context);
		sched_stcf();
		if(current_thread_cb==NULL || current_thread_cb->state==TERMINATED) {
			shutdown();
		}else{
			swapcontext(sch_thread_cb->context, current_thread_cb->context);
			if(current_thread_cb!=NULL){ 
				current_thread_cb->elapsed++;
			}
		}
	}
	//setcontext(main_thread_cb->context);

}
/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	if(current_thread_cb!=NULL){
		current_thread_cb->state=READY;//now this old thread is no longer running- its just ready
	}
	while(front!=NULL && front->state==TERMINATED) {
		printf("front!=Null");
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
		printf("ptr!=Null");
		if(ptr->elapsed<lowest->elapsed && ptr->state==READY){
			lowest=ptr;
		}
		if(ptr->next==ptr){
			break;
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