#include "gtthread.h"

struct itimerval timeQuantum;

struct gtthread *threadQueue;
int threadCount = 0;
int initCount;

static ucontext_t scheduler_context;
struct sigaction preemption_handler;

int GlobalThreadId = 0;
int allthreadsfinished = 0;

gtthread *mainThread, *newThread, *currentThread;
static int first = 1;

int numthreads;
static int count = 0;


static void scheduler() {
	while(count != 0){
		if(currentThread->finished != 1 && currentThread->exited != 1){
		swapcontext(&scheduler_context,&currentThread->context);
		}
		if(count == 1 && mainThread->exited == 1){
			exit(0);}
		if(currentThread->next != NULL)
		{	currentThread=currentThread->next;	}
	}
}


void printList()
{
	struct gtthread *temp = mainThread;
	if(mainThread != NULL)
	{
		printf("\n");
		do {
			printf(" Printing ThreadIDs %ld \n", temp->threadId);
			temp = temp->next;
		} while(temp != mainThread && temp != NULL);
	}
}



static void sighandler(int sig_nr) {
	if (sig_nr == SIGPROF) {
		swapcontext(&currentThread->context,&scheduler_context);
	}
	else return;
}

static void threadCtrl(void *func(), void *arg)
{
	currentThread->active = 1;
	currentThread->returnval = func(arg);	
	if(currentThread->exited!=1){
		currentThread->finished=1;
		gtthread_cancel(currentThread->threadId);
	}
	setcontext(&scheduler_context);
	return;
}

static void mainCtrl()
{
	mainThread->active = 1;
	setcontext(&mainThread->context);
	if(mainThread->exited != 1){
		mainThread->finished = 1;
		gtthread_cancel(mainThread->threadId);
	}
	setcontext(&scheduler_context);
	return;
}

void gtthread_init(long period){
	if(initCount == 0){
		initCount++;
		if (getcontext(&scheduler_context) == 0) {
			scheduler_context.uc_stack.ss_sp = malloc(STACKSIZE);
			scheduler_context.uc_stack.ss_size = STACKSIZE;
			scheduler_context.uc_stack.ss_flags = 0;
			scheduler_context.uc_link = NULL; 
		} 
		else {	printf("Init : Error allocating Scheduler Stack size \n"); exit(-1); }
		timeQuantum.it_value.tv_usec = (long) period;
		timeQuantum.it_interval = timeQuantum.it_value;

		// using sighandler(userdefined) as scheduler function
		preemption_handler.sa_handler = sighandler;
		preemption_handler.sa_flags = SA_RESTART | SA_SIGINFO;
		sigemptyset(&preemption_handler.sa_mask);
		// handle only SIGPROF signals 
		if (sigaction(SIGPROF, &preemption_handler, NULL) == -1) {
			printf("Init : An error occurred while initializing the signal handler for swapping to the scheduler context...\n");
			exit(-1);
		}
		
		mainThread = (gtthread *)malloc(sizeof(gtthread));
		getcontext(&mainThread->context);
				
		mainThread->threadId = (unsigned long int)GlobalThreadId;
		mainThread->active = 0;
		mainThread->finished = 0;
		mainThread->deleted = 0;
		mainThread->exited = 0;
				
		mainThread->context.uc_link=&scheduler_context;
		mainThread->context.uc_stack.ss_sp = malloc(STACKSIZE);
		mainThread->context.uc_stack.ss_size = STACKSIZE;
		mainThread->context.uc_stack.ss_flags = 0;	
		mainThread->next = NULL;
		
		if (mainThread->context.uc_stack.ss_sp == 0 ){	printf( "Create : Error: Could not allocate stack.", 0 );}
		if (setitimer(ITIMER_PROF, &timeQuantum, NULL) == 0) { /*printf("Create :The timer was initialized...\n");*/} else {
			printf("Create :An error occurred while initializing timer. Please check the value of period to init()...\n");
			exit(-1);}

		GlobalThreadId++;
	        currentThread = mainThread;	
		makecontext(&mainThread->context, (void(*)(void)) mainCtrl, 0, NULL, NULL);
		numthreads++;
		count++;
	} else {
		printf("You can initialize only once\n");
		exit(-1);
	}
	return;
}


int gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg){
	if(initCount < 1) {
		printf("\nCreate : Error: gtthread_init() not called! Exiting.\n");
		exit(-1);
	}
	*thread = (unsigned long int)GlobalThreadId;

	if(GlobalThreadId == 1){
		
		*thread = (unsigned long int)GlobalThreadId;
		
		newThread = (gtthread *)malloc(sizeof(gtthread));
		getcontext(&newThread->context);
		newThread->threadId = (unsigned long int)GlobalThreadId;
		newThread->active = 0;
		newThread->finished = 0;
		newThread->deleted = 0;
		newThread->exited = 0;
				
		newThread->context.uc_link=&scheduler_context;
		newThread->context.uc_stack.ss_sp = malloc(STACKSIZE);
		newThread->context.uc_stack.ss_size = STACKSIZE;
		newThread->context.uc_stack.ss_flags = 0;	
		
		if(newThread->context.uc_stack.ss_sp == 0  ){
			printf( "Create : Error: Could not allocate stack.\n", 0 );
			return -1;}
		newThread->next = mainThread;
		mainThread->next = newThread;
		makecontext(&newThread->context, (void(*)(void)) threadCtrl, 2, start_routine, arg);
		currentThread = newThread;
		GlobalThreadId++;
		numthreads++; 
		count++;

		getcontext(&mainThread->context);
		if(first){first=0;	
			scheduler();
		}
		return 0;

	} else{
		gtthread *temp = newThread;
		newThread = (gtthread *)malloc(sizeof(gtthread));
		getcontext(&newThread->context);
		newThread->threadId = (unsigned long int)GlobalThreadId;
		newThread->active = 0;
		newThread->finished = 0;
		newThread->deleted = 0;
		newThread->exited = 0;
		
		newThread->context.uc_link=&scheduler_context;
		newThread->context.uc_stack.ss_sp = malloc(STACKSIZE);
		newThread->context.uc_stack.ss_size = STACKSIZE;
		newThread->context.uc_stack.ss_flags = 0;	
		GlobalThreadId++;

		temp->next = newThread;
		newThread->next = mainThread;

		if(newThread->context.uc_stack.ss_sp == 0){
			printf( "Create : Error: Could not allocate stack.", 0 );
			return -1;}

		makecontext(&newThread->context, (void(*)(void)) threadCtrl, 2, start_routine, arg);
		count++;
		swapcontext(&currentThread->context,&scheduler_context);
		return 0;
	}
}



int  gtthread_join(gtthread_t thread, void **status){
	gtthread *temp = mainThread;
	do {
		if(temp != NULL && temp->threadId == thread){break;}
		temp = temp->next;
		} while(temp != mainThread && temp != NULL);
	
	if(temp != NULL){
		if((temp->exited == 1 || temp->finished == 1 || temp->deleted==1))
			{  
				if(status != NULL)
				*status = (void *)temp->returnval;
			 return 0;}
		while(temp->exited != 1 && temp->finished != 1){}
	}
	return -1;
}



void gtthread_exit(void *retval){
	currentThread->returnval = retval;
	currentThread->exited=1;
	if(currentThread->threadId != mainThread->threadId)
	gtthread_cancel(currentThread->threadId);
	setcontext(&scheduler_context);
}

void gtthread_yield(void){
  int flag = 0;
  getcontext(&currentThread->context);
  if(flag == 0){flag = 1;
  setcontext(&scheduler_context);}
  return;}

int  gtthread_equal(gtthread_t t1, gtthread_t t2){
	if(t1 == t2) return 1;
	else return 0;
}



int  gtthread_cancel(gtthread_t localId){
	if(mainThread == NULL){
		printf("Cancel : Error. Thread Queue is empty\n");
		exit(0);
	}
	gtthread *temp = mainThread;
	gtthread *toBeDeleted;
	do {
		if(temp->threadId == localId){
			free(temp->context.uc_stack.ss_sp);
			temp->active=0;
			count--; 
			temp->deleted=1;
			break;
		}
		temp = temp->next;
	} while(temp != NULL && temp != mainThread);
	return 0;
}


gtthread_t gtthread_self(void){
	return currentThread->threadId;
}

int  gtthread_mutex_init(gtthread_mutex_t *mutex){
	if(mutex->lock == 1)return -1;
	mutex->lock = 0;
	mutex->owner = -1;
	return 0;
}


int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	if((mutex->owner) == currentThread->threadId) {return -1;}
		while(mutex->lock !=0 && mutex->owner != currentThread->threadId)
		gtthread_yield();
		mutex->lock = 1;
		mutex->owner = currentThread->threadId;
		return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
	if(mutex->lock == 1 && mutex->owner == currentThread->threadId)
		{ mutex->lock = 0;
		  mutex->owner = -1;
		  return 0;
		}
	return -1;
}