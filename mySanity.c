/*char name[10];
int counter=0;
int binary_sem;
struct counting_semaphore* sem;

void 
writer(void* args) {
	//char* original = "barakmregg";
	for (int i = 0; i < 1; ++i)
	{
		//printf(1,"writer\n");
		bsem_down(binary_sem);
		for (int i = 0; i < 20; ++i)
			printf(1,"writer: %d\n",i);
		bsem_up(binary_sem);		
	}
	while(1);
}
void 
reader(void* param) {
	bsem_down(binary_sem);
	for (int i = 0; i < 20; ++i)
			printf(1,"reader: %d\n",i);
	bsem_up(binary_sem);
}
void 
upCounter(void* args) {
	for (int i = 0; i < 100; ++i)
	{
		bsem_down(binary_sem);
		counter++;
		printf(1,"counter: %d\n",counter);
		bsem_up(binary_sem);		
	}
}
void upCounter2(void* param) {
	for (int i = 0; i < 100; ++i)
	{
		bsem_down(binary_sem);
		counter++;
		printf(1,"counter: %d\n",counter);
		bsem_up(binary_sem);		
	}	
}
void upCounter3(void* param) {
	for (int i = 0; i < 100; ++i)
	{
		bsem_down(binary_sem);
		counter++;
		printf(1,"counter: %d\n",counter);
		bsem_up(binary_sem);		
	}	
}

void upCounterTset2(void* param) {
	for (int i = 0; i < 100; ++i)
	{
		//bsem_down(binary_sem);
		down(sem);
		counter++;
		printf(1,"counter: %d\n",counter);
		up(sem);
		//bsem_up(binary_sem);		
	}	
}
void upCounterTset22(void* param) {
	for (int i = 0; i < 100; ++i)
	{
		//bsem_down(binary_sem);
		down(sem);
		counter = counter + 1;
		printf(1,"counter22: %d\n",counter);
		up(sem);
		//bsem_up(binary_sem);		
	}	
}
int main(int argc, char *argv[])
{
	// struct counting_semaphore* sem;
	// int tid;
	uthread_init();

	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-1st test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-1st test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	binary_sem = bsem_alloc();
	uthread_create(writer, 0);
	uthread_join(uthread_create(reader, 0));	

	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-2nd test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-2nd test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	uthread_create(upCounter, 0);
	uthread_join(uthread_create(upCounter2, 0));
	if(counter==300)
		printf(1,"TSET 2 PASSED%s\n");
	else
		printf(1,"TSET 2 FAILED\n");
	counter=0;
	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-3rd test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	printf(1,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-3rd test-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\n");
	sem = counting_semaphore_alloc(2);
	uthread_create(upCounterTset2, 0);
	uthread_join(uthread_create(upCounterTset22, 0));
	printf(1,"counter: %d\n",counter);
	
	
	while(1);
	return 0;
}*/