#define UTHREAD_QUANTA 5
#define SIGALRM 14
#define NTICKET 5

#define STORE_ESP(var)  asm("movl %%esp, %0;" : "=r" ( var ))

// Loads the contents of var into esp
#define LOAD_ESP(var)   asm("movl %0, %%esp;" : : "r" ( var ))


// Saves the value of esp to var
#define STORE_EBP(var)  asm("movl %%ebp, %0;" : "=r" ( var ))

// Loads the contents of var into esp
#define LOAD_EBP(var)   asm("movl %0, %%ebp;" : : "r" ( var ))

// Calls the function func
#define CALL(addr)              asm("call *%0;" : : "r" ( addr ))

// Pushes the contents of var to the stack
#define PUSH(var)               asm("movl %0, %%edi; push %%edi;" : : "r" ( var ))



// thread states
typedef enum  {FREE, RUNNING, RUNNABLE, SLEEP} uthread_state;

// stack size
#define STACK_SIZE  4096
// max number of thread allowed
#define MAX_THREADS  7

// uthread required functions
int uthread_init();

int uthread_create(void (*start_func)(void *), void* arg);

void uthread_schedule();

void uthread_exit();

int uthread_self();

int uthread_join(int tid);

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

struct uthread {
	// thread id
	int	tid;
	// stack pointer
	int esp;      
	// base pointer
	int ebp;      
	// thread stack
	char *stack;	   
	// thread state
	uthread_state state;
	struct my_trapframe tf;
};
 
