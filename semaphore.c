#include "types.h"
#include "user.h"
#include "x86.h"
#include "uthread.h"
#define FREE_SEMAPHORE -1
#define MAX_BSEM 128
/*
Semaphore is an **abstract data type**(like a variable) which is used to 
restrict the access to a shared memory(**environment**) in multiprogramming 
operating systems. As an example, let's assume you want to enter a barbershop. 
There are **n** places available for you to sit on. Each customer that enters the 
shop and takes a place, the total number of places in decremented. When there are no sits left, 
the total number of available places will be 0. A **semaphore** variable, is that shared resource 
among all the customers(**processes**). There are two functions used with semaphore implementations 
in the operating systems which are **wait()** and **signal()**. When a process enters it's 
**critical section**, it is requesting permission to access the **shared memory**. 
Before entering the critical section, the process will call the **wait** function on the 
shared variable which is called a **semaphore** which is a **simple variable** in this case. 
When the wait function is called, the semaphore variable is **decremented** and when the process 
finishes it's job with the shared resource, it simply calls the **signal** function in order to say 
it is relieving an space from the shared memory so other processes are able to use that. When the signal
is called, the value of shared variable, **semaphore**, will be incremented.
 */




typedef enum BinSemStatus{
   locked,
   unlocked
} BinSemStatus;

// Binary Semaphores: Binary semaphores are binary, they can have two values only; 
// one to represent that a process/thread is in the critical section(code that 
// access the shared resource) and others should wait, the other indicating the 
// critical section is free
struct semaphore {
	BinSemStatus status;
	int bid;
};

// Counting semaphores take more than two values, they can have any value you want. 
// The max value X they take allows X process/threads to access the shared resource 
// simultaneously
// struct counting_semaphore {
// 	int binary_descriptor1;
// 	int binary_descriptor2;
// 	int value;
// };


struct semaphore semaphore_table[MAX_BSEM];
int init = 0;

static void init_table(){
	if (init)
		return;
	init = 1;
	for (int i = 0; i < MAX_BSEM; ++i)
	{
		semaphore_table[i].status = unlocked;
		semaphore_table[i].bid = FREE_SEMAPHORE;
	}
}
static int find_free_entry(){
	for (int i = 0; i < MAX_BSEM; ++i)
	{
		if (semaphore_table[i].bid == FREE_SEMAPHORE)
		{
			return i;
		}
	}
	return -1;
}

// the bsem_alloc function should allocate a new binary semaphore
// and return its descriptor.
// suppose that semaphor alocation made from the main thread only 
int 
bsem_alloc(){
	int free_entery_index;
	
	init_table();
	printf(1, "Semaphore allocation\n");
		
	free_entery_index = find_free_entry();
	if (free_entery_index == -1){
		printf(1, "failed to allocate new semaphor,semaphore table is FULL!\n");
		return -1;
	}
	// the index of the semphor is also its id
	semaphore_table[free_entery_index].bid = free_entery_index;
	// the newly allocated binary semaphore should be in unlocked state.
	semaphore_table[free_entery_index].status = unlocked;
	return semaphore_table[free_entery_index].bid;
}
// the bsem_free function should free the binary semaphore with
// the given descriptor. Note that the behavior of freeing a semaphore while other threads
// “blocked” because of it is undefined and should not be supported.
void 
bsem_free(int bid){
	if (semaphore_table[bid].status==locked)
	{
		semaphore_table[bid].bid = -1;
		semaphore_table[bid].status = unlocked;		
	}
	else
	{
		printf(1, "BINARY SEMAPHOR FREE failed!\n");
		return;
	}
}

// attempt to acquire (lock) the semaphore, in case that it is
// already acquired (locked), block the current thread until it is unlocked and then acquire it.
void 
bsem_down(int bid){
try_acquire:
	//printf(1, "-=-=-=-=-=-=-=DOWN=-=-=-=-=-=-=-=-\n");
	if (semaphore_table[bid].bid == -1) {
		printf(1, "ERROR bsem_down,semaphore id %d do not exist\n", bid);
		exit();
	}
	/*Sleeping-Wait*/
	while(1){
		alarm(0);
		if (semaphore_table[bid].status==unlocked)
		{
			goto unlock;
		}	
		sleep_on_semaphore();
		sigsend(getpid(), SIGALRM);//yeild and sleep
	}
unlock:
	if (unlocked==semaphore_table[bid].status) {
		semaphore_table[bid].status=locked;
		//printf(1, "unlocked!!!!!\n");
		alarm(UTHREAD_QUANTA);
	} else {
		alarm(UTHREAD_QUANTA);
		goto try_acquire;
	}
}
// releases (unlock) the semaphore.
void 
bsem_up(int bid){
	alarm(0);
	if ((-1) == semaphore_table[bid].bid)
	{
		printf(1,"bsem_up ERROR 1, semphore do not exist\n");
		return;
	}
	if (unlocked == semaphore_table[bid].status)
	{
		printf(1,"bsem_up ERROR 2 unlocked\n");
		return;
	}
	wake_up();
	semaphore_table[bid].status = unlocked;
	sigsend(getpid(), SIGALRM);
}

struct counting_semaphore*  
counting_semaphore_alloc(int value) {

	struct counting_semaphore* sem = malloc(sizeof(struct counting_semaphore));
	sem->value = value;
	sem->binary_descriptor1= bsem_alloc();//locks the value
	sem->binary_descriptor2= bsem_alloc();//locks the sing of the value
	if (value <= 0) {
		bsem_down(sem->binary_descriptor2);
	}
	return sem;
}
void sem_free(struct counting_semaphore* sem) {
	free(sem);
}

// If the value representing the count of
// the semaphore variable is not negative, decrement it by 1. If the semaphore variable is now
// negative, the thread executing acquire is blocked until the value is greater or equal to 1.
// Otherwise, the thread continues execution.
void 
down(struct counting_semaphore *sem){
	alarm(0);
	bsem_down(sem->binary_descriptor2);
	bsem_down(sem->binary_descriptor1);
	sem->value--;
	if (sem->value>0){
		bsem_up(sem->binary_descriptor2);
	}
	bsem_up(sem->binary_descriptor1);
	alarm(UTHREAD_QUANTA);
}

// Increments the value of semaphore
// variable by 1.
void up(struct counting_semaphore* sem) {
	alarm(0);
	bsem_down(sem->binary_descriptor1);
	sem->value++;
	if (sem->value == 1) {
		bsem_up(sem->binary_descriptor2);
	}
	bsem_up(sem->binary_descriptor1);
	alarm(UTHREAD_QUANTA);
}