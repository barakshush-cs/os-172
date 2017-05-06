#define UTHREAD_QUANTA 5
#define SIGALRM        14
#define STACK_SIZE     4096
#define MAX_THREADS    62

typedef enum  {FREE, RUNNING, RUNNABLE, SLEEP,BLOCKED} uthread_state;

int    uthread_init();
int    uthread_create(void (*start_func)(void *), void* arg);
void   uthread_schedule();
void   uthread_exit();
int    uthread_self();
int    uthread_join(int tid);
int    uthread_sleep(int ticks);
int    wake_up();
void   sleep_on_semaphore();
void   storeRegisters();
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

typedef struct uthread {
	
	int	tid;
	/*stack*/
	int esp;      
	/*base pointer*/
	int ebp;
	/*programm counter*/
	int eip;      
	/*stack*/
	char *stack;	   
	/*thread state*/
	uthread_state state;
	/*backedup trape frame*/
	struct my_trapframe tf;
	/*first Run*/
	int firstRun;
	/*sleepContDown*/
	int sleepContDown;

}uthread;
 
