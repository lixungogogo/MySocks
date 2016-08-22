#include <stdio.h>
#include <pthread.h>

void *A(void *arg)
{
	printf("1\n");
}

int main()
{

	pid_t id = 0;
	pthread_create(&id,NULL,A,NULL);
	pthread_cancle(id);
	return 0;
}

