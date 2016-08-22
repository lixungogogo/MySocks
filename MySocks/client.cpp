#include <iostream>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include "common.h"
#include "transfer.h"
#include "protocol.h"
const int BUF_LEN = 1048576;
void usage(const char*proc)
{
//	printf("usage:[ip] %s,[port] %d",proc);
	cout<<proc<<endl;
}

int proc_forward(int in,const char *ip,const char*port);
void *transfer(void *arg)//传输数据
{
	struct 	TParam *param = (struct TParam*)arg;
	int in = param->in;
	int out = param->out;
	int bro = param->bro;
	
	char buf[BUF_LEN];
	while(1)
	{
		int ret = recv(in,buf,BUF_LEN,0);//不管收多少数据，只负责收数据
		if(ret <= 0)
		{
			break;
		}
		//在send_n中加密
		ret = send_n(out,buf,ret);
		if(ret < 0)
		{
			break;
		}
	}
	close(in);
	close(out);
	return NULL;
}



int main(int argc,char *argv[])
{
	if(argc != 4)
	{
		usage(argv[0]);
		return 1;
	}
	
	int listen_sock = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in sadd;
	sadd.sin_family = AF_INET;
	sadd.sin_addr.s_addr = INADDR_ANY;
	sadd.sin_port = htons(atoi(argv[1]));
	
	assert(0 == bind(listen_sock,(struct sockaddr*)&sadd,sizeof(sadd)));
	assert(0 == listen(listen_sock,5));
	while(1)
	{
		struct sockaddr caddr;
		socklen_t len = sizeof(caddr);
		int in = accept(listen_sock,(struct sockaddr*)&caddr,&len);
		
		pid_t pid = fork();
		//fork两次解决僵尸进程问题
		assert(pid >= 0);
		if(pid >= 0)//father
		{
			waitpid(pid,NULL,0);//等子进程退出
			close(in);
		}else{
			pid_t pid2 = fork();
			assert(pid2 >= 0);
			
			if(pid2 > 0)
			{
				exit(0);//退出,被爷爷进程回收
			} 
			else //child变为孤儿进程
			{
				close(listen_sock);
				return proc_forward(in,argv[2],argv[3]);
			}
		}
		
	}
}

int proc_forward(int in,const char *ip,const char*port)
{
	int out = socket(AF_INET,SOCK_STREAM,0);
	assert(out != -1);
	
	struct sockaddr_in raddr;
	memset(&raddr,0,sizeof(raddr));
	raddr.sin_family = AF_INET;
	raddr.sin_addr.s_addr = inet_addr(ip);
	raddr.sin_port = htons(atoi(port));
	
	assert(0 == connect(out,(struct sockaddr*)&raddr,sizeof(raddr)));
	
	struct TParam up = {in,out,0};
	struct TParam down = {out,in,0};
	assert(0 == pthread_create(&down.bro,NULL,transfer,&up));
	assert(0 == pthread_create(&up.bro,NULL,transfer,&down));
	
	pthread_join(up.bro,NULL);
	pthread_join(down.bro,NULL);
	return 0;
}
