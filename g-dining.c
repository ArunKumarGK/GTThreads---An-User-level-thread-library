#include <sys/time.h>
#include <stdio.h>
#include <gtthread.h> 
#include <errno.h>

#define CHOPSTICKS 5

#define INITIAL 0
#define EATING 1
#define HUNGRY 2
#define THINKING 3 

int philosopher_status[CHOPSTICKS];

gtthread_mutex_t chopstick_mutex[CHOPSTICKS];
 
main()  
{
  int i;
  gtthread_init(100);
  gtthread_t philosopher_thread[CHOPSTICKS]; 
  int dn[CHOPSTICKS];
  void *diner();
  // intializing all the locks
  for (i=0;i<CHOPSTICKS;i++){
    gtthread_mutex_init(&chopstick_mutex[i]);
    philosopher_status[i]=INITIAL;
  }
    // creating the philosphers
  	for (i=0;i<CHOPSTICKS;i++){
      dn[i] = i;
      gtthread_create(&philosopher_thread[i],diner,&dn[i]);
  	}
   gtthread_exit(0);
  //	while(1)
}

void *diner(int *i)
{
int philId, eat, think, tTime, eTime;
//int eating = 0;
printf("I'm Philosopher %d\n",*i);
philId = *i;
int forwardCycle=0;
if(philId%2==0){
	forwardCycle=1;
}

while (1) {
   philosopher_status[philId]=THINKING;
   printf("Philosopher %d is thinking\n", philId);
   think=rand()%10000000;
   for(tTime=0;tTime<think;tTime++);
   philosopher_status[philId]=HUNGRY;
   printf("Philosopher %d is hungry\n", philId);
   while(philosopher_status[philId+CHOPSTICKS-1]==EATING || philosopher_status[philId+1]==EATING);
	   if(forwardCycle){
		   gtthread_mutex_lock(&chopstick_mutex[philId]);
		   gtthread_mutex_lock(&chopstick_mutex[(philId+1)%CHOPSTICKS]);
	   } else{
		   gtthread_mutex_lock(&chopstick_mutex[(philId+1)%CHOPSTICKS]);
		   gtthread_mutex_lock(&chopstick_mutex[philId]);
	   }
	   philosopher_status[philId]=EATING;
	   printf("Philosopher %d is eating\n", philId);
   eat=rand()%10000000;
   for(eTime=0;eTime<eat;eTime++);
   printf("Philosopher %d is done eating\n", philId);
	   if(forwardCycle){
		    gtthread_mutex_unlock(&chopstick_mutex[(philId+1)%CHOPSTICKS]);
		    gtthread_mutex_unlock(&chopstick_mutex[philId]);
		} else{
			gtthread_mutex_unlock(&chopstick_mutex[philId]);
			gtthread_mutex_unlock(&chopstick_mutex[(philId+1)%CHOPSTICKS]);
	   }
   }
gtthread_exit(NULL);
}