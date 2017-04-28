#include "types.h"
#include "user.h"
#include "uthread.h"
#include "x86.h"
struct my_trapframe {
  // registers as pushed by pusha
  uint edi;
  uint esi;
  uint ebp;
  uint oesp;      // useless & ignored
  uint ebx;
  uint edx;
  uint ecx;
  uint eax;

  // rest of trap frame
  ushort gs;
  ushort padding1;
  ushort fs;
  ushort padding2;
  ushort es;
  ushort padding3;
  ushort ds;
  ushort padding4;
  uint trapno;

  // below here defined by x86 hardware
  uint err;
  uint eip;
  ushort cs;
  ushort padding5;
  uint eflags;

  // below here only when crossing rings, such as from user to kernel
  uint esp;
  ushort ss;
  ushort padding6;
};


void FUNC1(void *arg) {
  int i;
  for(i = 0; i < 150; i++) {
    printf(1, "FUNCTION1: %d \n",i);
  }
}

void FUNC2(void *arg) {
  int i;
  for(i = 0; i < 150; i++) {
    printf(1, "FUNCTION2: %d \n",i);
  }
}

void FUNC3(void *arg) {
  int i;
  for(i = 0; i < 150; i++) {
    printf(1, "FUNCTION3: %d \n",i);
  }
}
	void alarm_sch(void *arg){
	 printf(1,"\n***********************ALARM Handler*************************************\n ");
	 alarm(5);
	}



// array of utherad structs
struct uthread uthread_pool[MAX_THREADS];
// next pid to run
int next_tid;
// current running thread index
int c_uthread_index;
// current number of tickets
int current_ticket_num;
// choosen lottery thread
int choosen;

static void 
execute_thread(void (*start_func)(void *), void* arg) {
  printf(1, "+++RUN THREAD+++ \n");
  alarm(UTHREAD_QUANTA);
  start_func(arg);
  uthread_exit();
}

int
uthread_init()
{
  printf(1, "** INIT USER LEVEL THREAD **\n");
  // set all threads state to FREE - init pool
  int i;
  for(i = 0; i < MAX_THREADS; ++i) {
    uthread_pool[i].state = FREE;
  }

  next_tid = 1;

  // initialize main thread
  c_uthread_index = 0;
  // set tid and stack to null
  uthread_pool[c_uthread_index].tid = 0;
  uthread_pool[c_uthread_index].stack = 0; 
  // main thread is ready to run
  uthread_pool[c_uthread_index].state = RUNNING;

  // register the SIGALRM to uthread_yield func
  if(signal(SIGALRM,(sighandler_t) uthread_schedule) != 0) {
    // case signal error
    return -1;
  }

  // execute the alarm syscall with UTHREAD_QUANTA
  alarm(UTHREAD_QUANTA);

  return 0;
}


int 
uthread_create(void (*func)(void *), void* arg)
{
  printf(1, "start thread creation \n");

  // local thread pool index
  int i;

  // disable thread switching
  alarm(0);

  printf(1, "uthread_create after alarm(0) \n");

  // search for free thread to load
  for(i = 0; i < MAX_THREADS; ++i) {
    if (uthread_pool[i].state == FREE) {
      goto load_t;
    }
  }

  // case no free threads available
  // enable thread switching
  alarm(UTHREAD_QUANTA);
  return -1;

load_t:
  // next free tid to assign
  uthread_pool[i].tid = next_tid;
  // update next tid
  next_tid++;
  // allocate stack for thread and returns the stack top to .stack field
  uthread_pool[i].stack = malloc(STACK_SIZE);

  // load thread execute function to stack with arguments and return value
  *( (int*)(uthread_pool[i].stack + STACK_SIZE - 3*sizeof(int)) ) = 0; 
  *( (int*)(uthread_pool[i].stack + STACK_SIZE - 2*sizeof(int)) ) = (int)func;
  *( (int*)(uthread_pool[i].stack + STACK_SIZE - sizeof(int)) ) = (int)arg;

  // set thread state to RUNNABLE, ready to run
  uthread_pool[i].state = RUNNABLE;

  // set esp and ebp to null, while thread is not in RUNNING state yet
  uthread_pool[i].esp = 0;
  uthread_pool[i].ebp = 0;
  

  // enable thread switching
  printf(1, "creation method: returning.. \n");
  alarm(UTHREAD_QUANTA);

  return uthread_pool[i].tid;
}

void 
uthread_exit(void)
{

  int i;

  // disable thread switching
  alarm(0);

  // deallocate thread memory
  if (uthread_pool[c_uthread_index].stack != 0) {
    free(uthread_pool[c_uthread_index].stack);
  }

  // deallocate thread from thread pool
  uthread_pool[c_uthread_index].state = FREE;

  // if any thread is waiting for current thread, get them back to RUNNABLE state
  for(i = 0; i < MAX_THREADS; ++i) {
    if (uthread_pool[i].state == SLEEP) {
      uthread_pool[i].state = RUNNABLE;
    }
  }

  // Check if there are any other uthread_pool that can be switched to
  for(i = 0; i < MAX_THREADS; ++i) {
    if (uthread_pool[i].state != FREE) {
      // found thread that is eligible to run, yield
      uthread_schedule();
    }
  }
  // no ready to run threads, exit
  exit();
}


void 
uthread_schedule(void)
{
	int esp = 0;
  // switch alarm(0) to cancel thread switching while yielding 
  alarm(0);
  printf(1, "******************SCHEDUALE******************\n");

  // if current thread is running (most cases), the thread didn't finisht its job so turn his state to RUNNABLE
  if (uthread_pool[c_uthread_index].state == RUNNING) {
    uthread_pool[c_uthread_index].state = RUNNABLE;
  }

  // save current thread esp and ebp
  asm("movl %%esp, %0;" : "=r" (uthread_pool[c_uthread_index].esp));
  asm("movl %%esp, %0;" : "=r" (esp));
  esp = esp+36;
  uthread_pool[c_uthread_index].tf = (struct my_trapframe)*esp;
  asm("movl %%ebp, %0;" : "=r" (uthread_pool[c_uthread_index].ebp));

  // load the new thread

  // pick the next thread index
    
  int iterator=c_uthread_index;
  iterator = (iterator+1)%MAX_THREADS;
  

  //WE HOPE USER WON'T BE STUPID AND SLEEP ON SLEEPING THREAD
  for(;; iterator = (iterator+1)%MAX_THREADS)
    if(uthread_pool[iterator].state == RUNNABLE){
      break;
    }
  c_uthread_index = iterator;

  // ****************************************************************** //

  // current switched thread is move to RUNNING mode, next to be execute
  uthread_pool[c_uthread_index].state = RUNNING;


  // if esp is null, thread is running its first time, so we have to load a functio to it 
  if(uthread_pool[c_uthread_index].esp == 0) {
    asm("movl %0, %%esp;" : : "r" (uthread_pool[c_uthread_index].stack + STACK_SIZE - 3*sizeof(int)));
    asm("movl %0, %%ebp;" : : "r" (uthread_pool[c_uthread_index].stack + STACK_SIZE - 3*sizeof(int)));
    // jump to execute function to run the thread own function
    asm("jmp *%0;" : : "r" (execute_thread));
  } 
  else  {
    // restore thread stack
    asm("movl %0, %%esp;" : : "r" (uthread_pool[c_uthread_index].esp));
    asm("movl %0, %%ebp;" : : "r" (uthread_pool[c_uthread_index].ebp));

    // reset alarm
    alarm(UTHREAD_QUANTA);
  }
}

int 
uthred_self(void)
{
  // return current running thread tid
  return uthread_pool[c_uthread_index].tid;
}

int  
uthred_join(int tid)
{
  int i;

  // if tid is not declared yet or is a negative number, error occured, return
  if(tid >= next_tid || tid < 0) {
    return -1;
  }

loop:

  // disable thread switching
  alarm(0);

  // run over all threads
  for(i = 0; i < MAX_THREADS; ++i) {
    // searching for a thread with the relevant tid
    if(uthread_pool[i].tid == tid) {
      // put current running thread to sleep
      uthread_pool[c_uthread_index].state = SLEEP;
      // let other thread to run 
      uthread_schedule();

      // if thread still alive, loop over the join procedure again
      goto loop;

    }
  }

  // the joined thread is not alive anyway, reset clock
  alarm(UTHREAD_QUANTA);

  return 0;
}
