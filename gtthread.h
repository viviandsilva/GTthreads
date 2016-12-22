#ifndef GTTHREAD_H
#define GTTHREAD_H

#include "steque.h"
#include <ucontext.h>


//Defining structures
typedef unsigned long int gtthread_t;

typedef struct gtthread_int_t {
  gtthread_t id; // the thread name (id)
  gtthread_t join_id; //the id of the thread that has called join on this thread
  int cancelled; 
  int completed;  
  void *retval;
  ucontext_t context;
} gtthread_int_t;

typedef struct gtthread_mutex_t {
  gtthread_t owner;
  int locked; //0 is unlocked, 1 locked
  int init;
  steque_t mutex_queue;
} gtthread_mutex_t;

//pool for all the threads
steque_t thread_pool;

//pool of scheduled threads
steque_t sched_queue;

sigset_t vtalrm;

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);
//void print_sched_queue(void);


void alrm_handler();

int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);


#endif
