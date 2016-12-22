#include <stdio.h>
#include <stdlib.h>
#include "gtthread.h"

/* Tests creation.
   Should print "Hello World!" */
gtthread_t main_thread, t1, t2,t3,t4,t5,t6,t7,t8;
gtthread_mutex_t count;

long increment=1000;

void *thr1(void *in) {

unsigned int i, j;
int x;

gtthread_mutex_lock (&count);
  for(i =0; i<10;i++){
      for(j=0; j<1000000;j++);{
      printf("Hello world from 1\n");
}
gtthread_mutex_unlock(&count);

    }

 x = gtthread_join(t2,NULL);
 //printf("Status1 %d\n", x);
  
  fflush(stdout);


  return (void*) 42;
}

void *thr2(void *in) {
int x;
unsigned int i, j;

//x = gtthread_join(t1,NULL);
//printf("Status2 %d\n", x);
  fflush(stdout);


  return NULL;
}

int main() {
  //gtthread_t t1, t2;
  unsigned long long int i,j;
  
  gtthread_init(1000);

  gtthread_mutex_init(&count);
 gtthread_create( &t1, thr1,NULL );
gtthread_create( &t2, thr2,NULL );
   gtthread_mutex_lock(&count);
     for(i =0; i<10;i++){
      for(j=0; j<1000000;j++);{
      printf("Hello world from main\n");
}
    }
gtthread_mutex_unlock(&count);
   //gtthread_mutex_unlock(&count);
     
   //gtthread_mutex_unlock(&count);
  void *status;
  //main_thread = gtthread_self();

   //gtthread_join(t1,&status);
   printf("%d\n",(int)status);   

 
  //for(i=0;i<100000000;i++);
 
  
 
printf("I am out\n");
  gtthread_exit(NULL);

//gtthread_create(&t2,thr2,NULL);
 

 //return EXIT_SUCCESS;
}
