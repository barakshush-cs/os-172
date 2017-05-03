
#define THREAD_QUANTA 5


#define STORE_ESP(var)  asm volatile("movl %%esp, %0;" : "=r" ( var ))
#define LOAD_ESP(var)   asm volatile("movl %0, %%esp;" : : "r" ( var ))
#define STORE_EBP(var)  asm volatile("movl %%ebp, %0;" : "=r" ( var ))
#define LOAD_EBP(var)   asm volatile("movl %0, %%ebp;" : : "r" ( var ))
#define CALL(addr)              asm volatile("call *%0;" : : "r" ( addr ))
#define PUSH(var)               asm volatile("movl %0, %%edi; push %%edi;" : : "r" ( var ))
#define RET                     asm volatile("ret");
#define PUSHAL                  asm volatile("push %eax");\
								asm volatile("push %ebx");\
								asm volatile("push %ecx");\
								asm volatile("push %edx");\
								asm volatile("push %edi");\
								asm volatile("push %esi");

#define POPAL                   asm volatile("pop %esi");\
								asm volatile("pop %edi");\
								asm volatile("pop %edx");\
								asm volatile("pop %ecx");\
								asm volatile("pop %ebx");\
								asm volatile("pop %eax");



/* Possible states of a thread; */
typedef enum  {T_FREE, T_RUNNING, T_RUNNABLE, T_SLEEPING} uthread_state;

#define STACK_SIZE  4096
#define STACK_ELEMENTS 1024
#define MAX_THREAD  64

typedef struct uthread uthread_t, *uthread_p;

struct uthread {
	int				tid;
	int 	       	esp;        /* current stack pointer */
	int 	       	ebp;        /* current base pointer */
	int 		   *stack;	    /* the thread's stack */
	uthread_state   state;     	/* running, runnable, sleeping */
};
 
 struct binary_semaphore{
 unsigned int value;
 int waitingL;
 struct uthread *waitProc[MAX_THREAD];
};

int uthread_init(void);
int  uthread_create(void (*func)(void *), void*);
void uthread_exit(void);
void uthread_yield(void);
int  uthred_self(void);
int  uthred_join(int);
void  binary_semaphore_init(struct binary_semaphore* , int);
void  binary_semaphore_down(struct binary_semaphore*);
void   binary_semaphore_up(struct binary_semaphore*);