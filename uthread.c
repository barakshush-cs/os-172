#include "types.h"
#include "user.h"
#include "uthread.h"
#include "x86.h"


/*GLOBAL*/
int id_for_next_thread;                   //index of next tid
int curr_thread_index;                    //current running thread
struct uthread thread_table[MAX_THREADS]; //user level thread table


int
uthread_init()
{
  printf(1, "** INIT USER LEVEL THREAD **\n");
  
  int i;
  for(i = 0; i < MAX_THREADS; ++i) {
    thread_table[i].state = FREE;
    }

  id_for_next_thread = 1;

  curr_thread_index = 0;
  thread_table[curr_thread_index].tid = 0;
  thread_table[curr_thread_index].stack = 0; 
  thread_table[curr_thread_index].state = RUNNING;

  /*set signal SIGALRM handler to be the scheduler*/
  if(signal(SIGALRM, (sighandler_t)uthread_schedule) != 0) {
    return -1;
  }

  /*set quanta*/
  alarm(UTHREAD_QUANTA);

  return 0;
}

int 
uthread_create(void (*func)(void *), void* arg)
{
  int i;

  printf(1, "-------------------uthread_create------------------- \n\n");
  /*disable context switch*/
  alarm(0);

  /*look for empty entery*/
  for(i = 0; i < MAX_THREADS; ++i) {
    if (thread_table[i].state == FREE) {
      goto creat_thread;
    }
  }

  alarm(UTHREAD_QUANTA);
  return -1;

creat_thread:
  thread_table[i].tid = id_for_next_thread;
  id_for_next_thread++;
  /*allocation of uesr level thread stack*/ 
  thread_table[i].stack = malloc(STACK_SIZE);

  /*set the general porpose registers to 0*/
  memset(&thread_table[i].tf, 0, sizeof(struct my_trapframe));

  thread_table[i].esp = (uint)(thread_table[i].stack+STACK_SIZE);
  
  /*push args*/
  thread_table[i].esp -= 4;
  *((uint*)thread_table[i].esp) = (uint)arg;
  /*push call to exit*/
  thread_table[i].esp -= 4;
  *((uint*)thread_table[i].esp) = (uint)uthread_exit;
  thread_table[i].ebp = thread_table[i].esp;
  thread_table[i].eip = (uint)func;
  thread_table[i].firstRun = 1;

  /*set state*/
  thread_table[i].state = RUNNABLE;
  

  /*context switch*/
  printf(1, "NEW thread  allocaTED\n");
  alarm(UTHREAD_QUANTA);
  return thread_table[i].tid;
}

void 
uthread_exit(void)
{

  int i;
  printf(1, "-------------====EXIT====------------- \n");

  /*disable context switch*/
  alarm(0);
  /*stack dealloction*/
  if (thread_table[curr_thread_index].stack != 0) {
    free(thread_table[curr_thread_index].stack);
    thread_table[curr_thread_index].stack = 0;
    thread_table[curr_thread_index].tid = -1;
  }

  /*clear entery of outgoing thread*/
  thread_table[curr_thread_index].state = FREE;

  /*Take care of waiting threds*/
  for(i = 0; i < MAX_THREADS; ++i) {
    if (thread_table[i].state == SLEEP) {
      thread_table[i].state = RUNNABLE;
    }
  }
  /*if there is a non empty entry - trig context switch
    else freform exit */
  for(i = 1; i < MAX_THREADS; ++i) {
    if (thread_table[i].state != FREE) {
      sigsend(getpid(), SIGALRM);
    }
  }
  printf(1, "-------------====EXIT after loop------------- \n");
  exit();
}

void 
uthread_schedule(void)
{
  uint esp = 0;
  int prev_thread=curr_thread_index;
  asm("movl %%esp, %0;" : "=r" (esp));
  int f=1;
  /*disable contetxt switch*/ 
  alarm(0);
  /*Last thread can be free*/
  if ((thread_table[curr_thread_index].state == RUNNING) & f) {
    thread_table[curr_thread_index].state = RUNNABLE;
  }
  /*save trap frame*/
  memmove(&thread_table[curr_thread_index].tf,(void*)(esp+24),sizeof(struct my_trapframe));

  /*find next thred in a round eobin manner*/    
  int iterator=curr_thread_index;
  iterator = (iterator+1)%MAX_THREADS;

  for(;; iterator = (iterator+1)%MAX_THREADS)
    if(thread_table[iterator].state == RUNNABLE){
    	//printf(1, "RUNNABLE******************,%d\n",iterator);
      break;
    }
  curr_thread_index = iterator;
  /*new thread in running*/
  thread_table[curr_thread_index].state = RUNNING;

  if(thread_table[curr_thread_index].firstRun) 
  {
  	thread_table[curr_thread_index].firstRun = 0;
  	thread_table[curr_thread_index].tf = thread_table[prev_thread].tf;
  	thread_table[curr_thread_index].tf.esp = thread_table[curr_thread_index].esp;
  	thread_table[curr_thread_index].tf.eip = thread_table[curr_thread_index].eip;
  	thread_table[curr_thread_index].tf.ebp = thread_table[curr_thread_index].ebp;

  	memmove((void*)(esp+24),&thread_table[curr_thread_index].tf,sizeof(struct my_trapframe));
  	printf(1,"prev_thread %d\n",prev_thread);
  	alarm(UTHREAD_QUANTA);
  } 
  else  {
    /*restore trap frame*/
    memmove((void*)(esp+24),&thread_table[curr_thread_index].tf,sizeof(struct my_trapframe));
    /*set quanta again- eneble context switch*/
    alarm(UTHREAD_QUANTA);
  }
}

int 
uthred_self(void)
{
  return thread_table[curr_thread_index].tid;
}

int  
uthread_join(int tid)
{
  int i;
  /*cheks tid validity
    If that thread has already terminated/not exists, 
    then uthread_join returns immediately.*/
  if(tid >= id_for_next_thread || tid < 0) {
    return -1;
  }

wait_until_termination:
  alarm(0);
  for(i = 0; i < MAX_THREADS; ++i) {
    if(thread_table[i].tid == tid) {
      thread_table[curr_thread_index].state = SLEEP; 
      sigsend(getpid(), SIGALRM);
      goto wait_until_termination;

    }
  }
  /*wake up*/
  alarm(UTHREAD_QUANTA);
  return 0;
}

int 
uthread_sleep(int ticks) {
	uint start_time = uptime();
	printf(1,"*****************SLEEP*******************\n");
	printf(1, "Thread id:  %d went in to sleep state for %d ticks\n", thread_table[curr_thread_index].tid, ticks);	
	while (uptime() - start_time < ticks) {
		printf(1, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ...\n");
		printf(1, "Thread id:  %d  is sleeping\n", thread_table[curr_thread_index].tid);
		sigsend(getpid(), SIGALRM);
	}	
	printf(1, "Thread: %d is WAKING UP\n", thread_table[curr_thread_index].tid);
	return 0;
}