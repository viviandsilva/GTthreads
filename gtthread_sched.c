/**********************************************************************
gtthread_sched.c.  

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
  Include as needed
*/

#include "gtthread.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
/* 
   Students should define global variables and helper functions as
   they see fit.
 */


// Defining a thread as a structure with its various attributes
//possible attributes - 
// 1) thread ID, 
// 2) return status of the thread
// 3) indication if thread has been completed
// 4) context
// 5) cancellation of a thread


//in addition to init, create, join functions: what other things do we need?
//-> obtain thread by its ID 
//-> schedule threads (round robin scheduler) 
//-> need to create the signal handler function

#define bool int
#define true 1
#define false 0


//variable to give thread ID (by keeping count)
static long int thread_count = 1;

//Alarms and timers
struct itimerval *T;
struct sigaction act;




/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.  If period is zero, then thread switching should
  occur only on calls to gtthread_yield().

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */


static void scheduler(gtthread_int_t *thread) {
  gtthread_int_t *front;
  int i;
  //printf("Here\n");
  do { 

    if (!steque_isempty(&sched_queue)){
      front = (gtthread_int_t *) steque_front(&sched_queue);
      //if a thread is cancelled, it is complete
      if (front->cancelled) {
        front->completed = 1;  
        //Remove completed thread from the queue 
        steque_pop(&sched_queue); 

    } 
    //handling a case when main is cancelled
    if (front->id == 1 && front->cancelled){
      //printf("Entered this!\n");
      exit(0);
    }

  } else{
      //if there is nothing in the schedule queue
      exit(0);
    }
  } while (front->completed); 

  swapcontext(&thread->context, &front->context);  

}

//function to find the thread by its ID
static gtthread_int_t* find_thread_by_id(gtthread_t thread) {

  gtthread_int_t *required_thread;
  int thread_queue_len;  
  thread_queue_len = steque_size(&thread_pool);
  //printf("Size of thread_pool: %d\n",thread_queue_len);
  int i;
  for (i=0; i<thread_queue_len; i++) {
    required_thread = (gtthread_int_t*) steque_front(&thread_pool);
    steque_cycle(&thread_pool);
    if (required_thread->id == thread){
      return required_thread; //return the thread which matches the required thread
    }
  }
  return (NULL); //if thread is not found
}

//wrapper function to obtain return value from start routine
static void wrapper(void *(*start_routine)(void *), void *arg) {

  sigprocmask(SIG_UNBLOCK,&vtalrm, NULL); 
  void *retval;
  retval = start_routine(arg); //piazza post, need the return value 
  gtthread_exit(retval); //using the return value to gtthread_exit

  }

//this function handles the schedule queue when a signal comes in
void alrm_handler() { 
  
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread_int_t *front;
  front=(gtthread_int_t*)steque_pop(&sched_queue); 
  steque_enqueue(&sched_queue,front); //putting the thread from front to the back 
  scheduler(front);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return;

}

void gtthread_init(long period){

  //need initialization code for a thread
  gtthread_int_t *main_thread;  
  main_thread = (gtthread_int_t*)malloc(sizeof(gtthread_int_t));
  
  //initialize thread pool
  steque_init(&thread_pool);
  steque_init(&sched_queue);
   
  main_thread->id = thread_count++; //giving an ID to the main thread 
  main_thread->completed = 0; //not completed
  main_thread->cancelled = 0; //don't cancel
  steque_enqueue(&thread_pool,main_thread);  //put main thread in thread pool
  steque_enqueue(&sched_queue,main_thread);  //put main thread in schedule queue
  
  
  //printf("Main thread id is %lu\n", main_thread->id);
 
  getcontext(&main_thread->context);
  
  //initializing signal stack
  //do we neeed this?
  //the main thread would use the program stack, so I don't think this is needed
  //(main_thread->context).uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  //(main_thread->context).uc_stack.ss_size = SIGSTKSZ;
  
  //setting up timers and alarms
  sigemptyset(&vtalrm);
  sigaddset(&vtalrm, SIGVTALRM);
  
  T = (struct itimerval*) malloc(sizeof(struct itimerval));
  T->it_value.tv_sec = T->it_interval.tv_sec = 0;
  T->it_value.tv_usec = T->it_interval.tv_usec = period;

  //Setting up the handler
  memset (&act, '\0', sizeof(act));
  act.sa_handler = &alrm_handler; //stuff to be done when a signal comes
  if (sigaction(SIGVTALRM, &act, NULL) < 0) {
    perror ("sigaction");
    return;
  }

  setitimer(ITIMER_VIRTUAL, T, NULL);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  

}

/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
		    void *(*start_routine)(void *),
		    void *arg){

  gtthread_int_t *created_thread;  
  gtthread_int_t *parent_thread; //this is for the thread that calls the gtthread_create
  sigprocmask(SIG_BLOCK,&vtalrm,NULL);  

  created_thread = (gtthread_int_t*) malloc(sizeof(gtthread_int_t)) ;
  created_thread->id = thread_count++;
  *thread = created_thread->id;
  created_thread->completed = 0;
  created_thread->cancelled = 0; 
  steque_enqueue(&thread_pool,created_thread); //putting the created thread in thread pool
  steque_enqueue(&sched_queue,created_thread); //putting the created thread in schedule pool
  
  //saving context for the thread
  getcontext(&created_thread->context);
  
  //signal handlers for the thread  
  (created_thread->context).uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  (created_thread->context).uc_stack.ss_size = SIGSTKSZ;
  parent_thread = (gtthread_int_t*) steque_front(&sched_queue);  
  (created_thread->context).uc_link = &parent_thread->context;
  makecontext(&created_thread->context,(void (*) (void)) wrapper, 2,start_routine, arg);
  sigprocmask(SIG_UNBLOCK,&vtalrm,NULL); 
  return 0;
}


int  gtthread_equal(gtthread_t t1, gtthread_t t2){

  if (t1==t2){
    return 1;
  } else {
    return 0;
  }

}

void gtthread_yield(void){

  sigprocmask(SIG_BLOCK,&vtalrm,NULL);
  gtthread_int_t *front;
  front = (gtthread_int_t*)steque_pop(&sched_queue);
  steque_enqueue(&sched_queue,front);
  scheduler(front);
  sigprocmask(SIG_UNBLOCK,&vtalrm,NULL);


}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */


int gtthread_cancel(gtthread_t thread){

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread_int_t *cancel_thread;
  // need to know which thread has to be cancelled
  cancel_thread = find_thread_by_id(thread);
  sigprocmask(SIG_UNBLOCK, &vtalrm,NULL);
  if(cancel_thread != NULL) { 
      cancel_thread->cancelled =1;
      return 0;
  }
   else { 
      return 1 ; // thread not found 
  }  
   
}


/*
  Returns calling thread. */
gtthread_t gtthread_self(void){

  gtthread_int_t *self;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  self = (gtthread_int_t *) steque_front(&sched_queue);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return self->id;

}


void gtthread_exit(void* retval){

  gtthread_int_t *thread;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  thread = (gtthread_int_t*) steque_pop(&sched_queue);
  thread->retval=retval;
  thread->completed = 1;
  //since this thread has completed, let some other thread run
  scheduler(thread);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return;
}


int gtthread_join(gtthread_t thread, void **status){

  gtthread_int_t* target; //the one that needs to be joined
  gtthread_int_t* parent; //the thread that calls gtthread_join
  
  sigprocmask(SIG_BLOCK,&vtalrm,NULL);
  target = find_thread_by_id(thread); //need to find the thread that needs to be joined 
  parent = (gtthread_int_t*) steque_front(&sched_queue);
  
  if(target!=NULL){

    target->join_id = parent->id;

    //when a join b and b join a 
    if (parent->join_id == target->id){
      //printf("This is an error\n");
      return 1;
    }

    //situation when thread to be joined is cancelled
    if (target->cancelled){
      //printf("Cannot join cancelled thread\n");
      return 1;
    }

    //situation when a thread tries to join itself 
    if (parent->id == target->id  ){
      //printf ("This is an error, a Thread is trying to join itself\n");
      return 1;
    }
   
  sigprocmask(SIG_UNBLOCK,&vtalrm,NULL);
  while (!target->completed ){
       
    gtthread_yield();

  } //status
    if (status!=NULL){
       *status= target->retval;
    }
   
     return 0;

  } else {
     sigprocmask (SIG_UNBLOCK,&vtalrm,NULL);
     return 1 ; //target not found

  } 
}

/*void print_sched_queue(void) {
  int queue_len;
  unsigned long int i;
  gtthread_int_t *item;

  queue_len = steque_size(&sched_queue); //obtaining size of the run/schedule queue

  for (i = 0; i < queue_len; i++) {
    item = (gtthread_int_t *) steque_pop(&sched_queue); //pop elements from the top
    steque_enqueue(&sched_queue, item);  //push it at the back to maintain ordering
    printf("%d->", (int) item->id); //printing ID of the threads in the run/schedule queue
    fflush(stdout);  
  }

  printf("\n");
  fflush(stdout); 
}*/