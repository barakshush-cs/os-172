#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"

void FUNCTION1(void *arg) {
	int i;
	for(i = 0; i < 150; i++) {
		printf(1, "FUNCTION1: %d \n",i);
	}
}

void FUNCTION2(void *arg) {
	int i;
	for(i = 0; i < 150; i++) {
		printf(1, "FUNCTION2: %d \n",i);
	}
}

void FUNCTION3(void *arg) {
	int i;
	for(i = 0; i < 150; i++) {
		printf(1, "FUNCTION3: %d \n",i);
	}
}

int main(int argc, char *argv[]) {
	printf(1,"***ult main function started***\n");
	uthread_init();
	uthread_create(FUNCTION1, (void*)100);
	//uthread_create(FUNCTION2, (void*)100);
	//uthread_create(FUNCTION3, (void*)100);
	// TO-DO: mechanism for waiting for thread to finish before exiting main function
	for(;;);
	printf(1, "**end of ulttest main function***\n");
	exit();
}