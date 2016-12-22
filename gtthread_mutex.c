/**********************************************************************
gtthread_mutex.c.  

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*Include as needed*/

#include "gtthread.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */

//need to have a queue for the mutex according to the description

int gtthread_mutex_init(gtthread_mutex_t* mutex){
  // initializing the queue for this mutex

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  if (mutex->init == 1){

    //printf("This mutex is already initialized\n");
   return 1;

  } else {

      mutex->init = 1; //initializing the mutex  
      mutex->locked = 0; //unlock
      mutex->owner = 0; //no owner initially
      steque_init(&mutex->mutex_queue);
  }
   sigprocmask(SIG_UNBLOCK,&vtalrm,NULL);
   return 0;
 
}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */

int gtthread_mutex_lock(gtthread_mutex_t* mutex){
  sigprocmask(SIG_BLOCK,&vtalrm,NULL);
  gtthread_int_t* thread;
  
  //when a particular thread comes along , it yields if the mutex is already locked, else it acquires the lock
  thread = (gtthread_int_t*) steque_front(&sched_queue);

  //printf("The thread that is entering : %lu\n",thread->id );

  //checking if the lock is initialized
  if(mutex->init != 1){

    //printf("acquiring a lock that has not been initialized\n");
    return 1;
  }

  //checking if the thread that is calling the lock function already has the lock
  if (mutex->owner == gtthread_self() && mutex->locked ==1){

    //printf("you have the lock\n");
    sigprocmask(SIG_UNBLOCK,&vtalrm, NULL);
    return 1;

  }

  steque_enqueue(&mutex->mutex_queue,thread);
  
  //spinning
  while(mutex->locked == 1 || ((gtthread_int_t*)(steque_front(&mutex->mutex_queue)))->id != gtthread_self()) {
    gtthread_yield();
    sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  }
  
  //you can acquire the lock now!
   mutex->locked = 1;
   mutex->owner = gtthread_self();
   //printf("Owner of lock is %lu\n", mutex->owner);
   
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  
  return 0;

}
/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex){

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  
  //printf("In unlock\n");
  
  // Check if lock is initialized
  if(mutex->init != 1) {
    //printf("You are trying to acquire lock that is not yet initialized\n");
    return 1;
  }
  
  // Check if lock is currently held by the calling thread
  if(mutex->owner != gtthread_self()) {
    //printf("Calling thread is not the lock owner. This is an error!\n");
    return 1;
  } 
  else if(mutex->locked == 1) {
    //unlock it
    mutex->locked = 0;
    mutex->owner = 0;
    steque_pop(&mutex->mutex_queue);
  }
  
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  
  return 0; 
}
/*
  The gtthread_mutex_destroy() function is analogous to
  pthread_mutex_destroy and frees any resourcs associated with the mutex.
*/
/*
int gtthread_mutex_destroy(gtthread_mutex_t *mutex){

}
*/


