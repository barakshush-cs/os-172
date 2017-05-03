#include "types.h"
#include "user.h"
#include "uthread.h"
#include "x86.h"




uthread_p t_table;
volatile unsigned int t_table_lock=0;
int current_thread=-1;

int
uthread_init(void) {
	t_table = malloc(MAX_THREAD*sizeof(uthread_t));
	if (t_table==0)
		return -1;
	int i = 0 ;
	for (i=0;i<MAX_THREAD;i++) {
		t_table[i].state=T_FREE;
		t_table[i].tid=i;
	}
	current_thread=0;
	t_table[0].state=T_RUNNING;
	t_table[0].stack=0;
	signal(14,(sighandler_t)uthread_yield);
	alarm(THREAD_QUANTA);
	return 0;
}

int
getNextFreeThread(void) {
	int i;
	for (i=0;i<MAX_THREAD;i++) 
		if (t_table[i].state==T_FREE)
			return i;
	return -1;
}

int
getNextRunnableThread(void) {
	int i;
	for (i=0;i<MAX_THREAD;i++) 
		if (t_table[(i+1+current_thread)%MAX_THREAD].state==T_RUNNABLE)
			return (i+1+current_thread)%MAX_THREAD;
	return -1;
}

int  
uthred_self(void) {
    return current_thread;
}


void 
uthread_yield(void) {
	while(xchg(&t_table_lock, 1) != 0) ;
	uthread_p this = &t_table[current_thread];
	this->state=T_RUNNABLE;
	STORE_ESP(this->esp);
    STORE_EBP(this->ebp);
    PUSHAL;
    int next = getNextRunnableThread();
    //printf(2,"*****switcing from %d to %d*******\n",current_thread,next);
    t_table[next].state=T_RUNNING;
    xchg(&t_table_lock, 0);//release
    current_thread=next;
    LOAD_EBP(t_table[current_thread].ebp);
    LOAD_ESP(t_table[current_thread].esp);
    POPAL;
    alarm(THREAD_QUANTA);
    return ;
}

int 
uthread_create(void (*func)(void *), void* arg) {
	while(xchg(&t_table_lock, 1) != 0) ;
	int tid = getNextFreeThread();
	if (tid==-1) {
		printf(2,"No more threads can be created\n");
		return -1;
	}
	t_table[tid].tid=tid;
	t_table[tid].state=T_RUNNABLE;
	xchg(&t_table_lock, 0);//release
	t_table[tid].stack=malloc(STACK_SIZE);
	*(t_table[tid].stack+STACK_ELEMENTS-1) = (int)arg;
	*(t_table[tid].stack+STACK_ELEMENTS-2) = (int)uthread_exit;
	*(t_table[tid].stack+STACK_ELEMENTS-3) = (int)func;
	*(t_table[tid].stack+STACK_ELEMENTS-4)=(int)(t_table[tid].stack+STACK_ELEMENTS-3)     ;//"old ebp" - will go to ebp
	*(t_table[tid].stack+STACK_ELEMENTS-5)=0; // eax ;
	*(t_table[tid].stack+STACK_ELEMENTS-6)=0; // ebx
	*(t_table[tid].stack+STACK_ELEMENTS-7)=0; // ecx
	*(t_table[tid].stack+STACK_ELEMENTS-8)=0; // edx
	*(t_table[tid].stack+STACK_ELEMENTS-9)=0; // edi
	*(t_table[tid].stack+STACK_ELEMENTS-10)=0; // esi
	t_table[tid].esp=(int)(t_table[tid].stack+STACK_ELEMENTS-10);
	t_table[tid].ebp=(int)(t_table[tid].stack+STACK_ELEMENTS-4);  
	return tid; 
}


void 
uthread_exit(void) {
	while(xchg(&t_table_lock, 1) != 0) ;
	int next = getNextRunnableThread();
	if (next==-1) {
		printf(2,"last thread finished!\n");
		exit();
	}
	
	//printf(2,"*****thread %d exited properly*******\n",current_thread);
	//printf(2,"found next tid %d\n",next);
	t_table[current_thread].state=T_FREE;
	t_table[next].state=T_RUNNING;
	xchg(&t_table_lock, 0);//release
	int old_thread = current_thread;
	current_thread=next;
	if (t_table[old_thread].stack)      //special case, for main theard, no  malloc
		free(t_table[old_thread].stack);
    LOAD_EBP(t_table[current_thread].ebp);
    LOAD_ESP(t_table[current_thread].esp);
    POPAL;
    alarm(THREAD_QUANTA);
    return ;
}

int 
uthred_join(int tid) {
	if (tid>MAX_THREAD)
		return 1;
	while(1) {
		if (t_table[tid].state==T_FREE)
			return 0;
		uthread_yield();
	}
}

/*void  binary_semaphore_init(struct binary_semaphore* semaphore, int value){
	semaphore->value=value;
	semaphore->waitingL=0;
}*/

/*
Using Alarm(0) - can lead to one proccess never going to sleep, if he keeps doing
binary_semaphore_up() on semaphore that is already up.

the other option was using busy wait with  while(xchg(lock,1)), to do a lock on the semaphore.
but than the semaphore can be not fair.
1 proccess comes, uses the semaphore. 2nd process comes, tries to use, enter a while loop
his QUANTA runs out, and he goes to sleep. when he wakes up, 3rd process is already using the semaphore
so he enters the while loop again...

*/

