#define gtthread_t unsigned long int

#include <sys/time.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>

#include <ucontext.h>
#include <unistd.h>

#define STACKSIZE 10000

typedef struct gtthread{
	int active;
	int finished;
	int exited;
	int deleted;
	gtthread_t threadId;
	ucontext_t context;
	void *returnval;
	struct gtthread *next;
} gtthread;

typedef struct gtthread_mutex_t{
long lock;
gtthread_t owner;
}gtthread_mutex_t;



// Function Signature Definition 

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);

int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
