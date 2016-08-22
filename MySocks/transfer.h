#ifndef __TRANSFER_H__
#define __TRANSFER_H__ 

#include <pthread.h>
struct TParam
{
	int in;
	int out; 
	pthread_t bro;//两个线程互相照应
};
//void *transfer(void *arg);//传输数据
#endif
