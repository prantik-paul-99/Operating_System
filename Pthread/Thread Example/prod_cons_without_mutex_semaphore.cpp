#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include <unistd.h>

using namespace std;


//semaphore to control sleep and wake up
sem_t empty;
sem_t full;
pthread_mutex_t mtx;
queue<int> q;



void * ProducerFunc(void * arg)
{	
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{
		if(i>3 && i<7) sleep(3);
		else sleep(1);
		sem_wait(&empty);		
		pthread_mutex_lock(&mtx);	
		
		q.push(i);
		printf("producer produced item %d\n",i);
		pthread_mutex_unlock(&mtx);
		sem_post(&full);
	}
}

void * ConsumerFunc(void * arg)
{
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{	
		
		sleep(2);
 		sem_wait(&full);		
		pthread_mutex_lock(&mtx);
		

		int item = q.front();
		q.pop();
		printf("consumer consumed item %d\n",item);	
		pthread_mutex_unlock(&mtx);
		sem_post(&empty);
			
		
	}
}





int main(void)
{	
	pthread_t thread1;
	pthread_t thread2;
	
	//init_semaphore();
	sem_init(&empty,0,5);
	sem_init(&full,0,0);
	pthread_mutex_init(&mtx,NULL);
	
	char * message1 = "i am producer";
	char * message2 = "i am consumer";	
	
	pthread_create(&thread1,NULL,ProducerFunc,(void*)message1 );
	pthread_create(&thread2,NULL,ConsumerFunc,(void*)message2 );

	while(1);
	return 0;
}
