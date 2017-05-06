#include "types.h"
#include "user.h"
#include "uthread.h"


#define TRUE 1
#define Q_SIZE 100


/*semaphores*/
struct counting_semaphore* cfull;
struct counting_semaphore* cempty;
int                        bmutex;


/*GLOBALS*/
int producer_curr_val = 1;
int q[Q_SIZE];
int producer_next_index = 0;
int consumer_next_index = 0;


void consumer(void* arg) {
	int consumedNumber = 0;
	while (consumedNumber<1000) {
		down(cfull);
		bsem_down(bmutex);
		consumedNumber = q[consumer_next_index];
		printf(1, "Consumer consumedNumber:  %d\n", consumedNumber);
		consumer_next_index = (consumer_next_index + 1) % Q_SIZE;
		bsem_up(bmutex);
		up(cempty);
		uthread_sleep(consumedNumber);
		bsem_down(bmutex);
		printf(1, "Thread - %d - slept for - %d - ticks.\n", uthread_self(), consumedNumber);
		bsem_up(bmutex);
	}
	exit();
}

void producer(void* arg) {
	while (producer_curr_val < 1000) {
		down(cempty);
		bsem_down(bmutex);
		printf(1, "Producer producer_curr_val:  %d \n", producer_curr_val);
		q[producer_next_index] = producer_curr_val;
		producer_curr_val++;
		producer_next_index = (producer_next_index + 1) % Q_SIZE;		
		bsem_up(bmutex);
		up(cfull);
	}
}

int main(int argc, char *argv[])
{
	int tid,cons1,cons2,cons3;
	uthread_init();

	cempty = counting_semaphore_alloc(Q_SIZE);
	cfull   = counting_semaphore_alloc(0);
	bmutex = bsem_alloc();
	tid    = uthread_create(producer, 0);

	cons1 = uthread_create(consumer, 0);
	cons2 = uthread_create(consumer, 0);
	cons3 = uthread_create(consumer, 0);

	uthread_join(cons1);
	uthread_join(cons2);
	uthread_join(cons3);	
	uthread_join(tid);

    exit();
	return 0;
}

