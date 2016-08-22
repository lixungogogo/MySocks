#include <iostream>
//#include <stdio.h>
using namespace std;
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include "common.h"
#include "protocol.h"
#include "transfer.h"


const int BUF_LEN = 1048576;

void *transfer(void *arg)//传输数据
{
	cout<<"transfer"<<endl;
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
//	pthread_cancle(bro);	
	return NULL;
}



void usage(const char*proc)
{
//	printf("usage:[ip] %s,[port] %d",proc);
}



int proc_sock(int in)
{
//	printf("proc_sock\n");
	//读取请求
	struct	SKReq sreq;
	assert(0 == recv_n(in,(char*)&sreq,sizeof(sreq)));
//	printf("ver:%d,n:%d\n",sreq.ver,sreq.n);
	
	assert(sreq.ver == 5 && sreq.n == 1);
	
	
	char method[8];
	assert(0 == recv_n(in,method,sreq.n));
	//recv_n(in,method,sizeof(method));
//	printf("method:%s\n",method);
	assert(0 == method[0]);//方式选择为0
//	printf("read req done\n");
	cout<<"req done"<<endl;
	//发送回应
	struct SKRep srep;
	srep.ver = 5;
	srep.m = 0;
	
	assert(0 == send_n(in,(char*)&srep,sizeof(srep)));
	//读取请求，发送回应完毕，协商过程完成。
	
	//下一步，读取地址
	
	struct sockaddr_in out_addr;
	memset(&out_addr,0,sizeof(out_addr));
	out_addr.sin_family = AF_INET;

	struct	AddReq areq;
	assert(0 == recv_n(in,(char*)&areq,sizeof(areq)));
	assert(areq.ver == 5 && areq.cmd == 1 && areq.rsv == 0);
	if(areq.atype == 1)//IPV4
	{
		//IPV4直接得到地址
		assert(0 == recv_n(in,(char*)&out_addr.sin_addr.s_addr,sizeof(out_addr.sin_addr.s_addr)));
		cout<<out_addr.sin_addr.s_addr<<endl;
	}
	else if(areq.atype == 3)	//得到域名，需要解析取出地址
	{
		char addlen;//域名第一个字节为地址长度
		struct hostent *host;
		assert(0 == recv_n(in ,(char *)&addlen,1));//取出地址长度
		char domain[256];
		assert(0 == recv_n(in,domain,addlen));//取出地址
		domain[addlen] = 0;
	
	    host = (struct hostent*)gethostbyname(domain);//得到域名地址结构体
		assert(host!=NULL && host->h_addrtype == AF_INET && host->h_length > 0);
		
		memcpy(&out_addr.sin_addr.s_addr,host->h_addr_list[0],sizeof(out_addr.sin_addr.s_addr));
		//得到域名 
//		printf("add is :%s\n",domain);//输出域名
		cout<<domain<<endl;
	}
	else
	{
		return -1;
	}
	
	short port;//取端口
	assert(0 == recv_n(in,(char*)&out_addr.sin_port,sizeof(out_addr.sin_port)));
	
	//现数据读取完毕,代理连接网站
	int out = socket(AF_INET,SOCK_STREAM,0);
	assert(-1 != out);
	
	assert(0 == connect(out,(struct sockaddr*)&out_addr,sizeof(out_addr)));
	cout<<"connect"<<endl;	
	//返回reply
	struct AddRep arep;
	arep.ver = 5;
	arep.cmd = 0;
	arep.rsv = 0;
	arep.atype = 1;//IPV4
	//初始化
	
	struct sockaddr_in local_addr;
	socklen_t slen = sizeof(local_addr);
	assert(0 == getsockname(out,(struct sockaddr*)&local_addr,&slen));
	//assert(0 == gethostbyname(out,(struct sockaddr*)&local_addr,slen));
	//getsockname函数用于一个已捆绑或已连接套接字，本地地址将被返回
	
	memcpy(&arep.addr,&local_addr.sin_addr.s_addr,sizeof(local_addr.sin_addr.s_addr));
	memcpy(&arep.port,&local_addr.sin_port,sizeof(short));
	//发送地址回应
	assert(0 == send_n(in,(char*)&arep,sizeof(arep)));
	
	
	//建立两个线程互相进行传输
	struct TParam up = {in,out,0};//in->out
	struct TParam down = {out,in,0};//out->in
	
	assert(0 == pthread_create(&down.bro,NULL,transfer,&up));
	assert(0 == pthread_create(&up.bro,NULL,transfer,&down));
	
	pthread_join(up.bro,NULL);
	pthread_join(down.bro,NULL);	

	//pthread_detach(up.bro);
	//pthread_detach(down.bro);
	return 0;
}




int main(int argc,char *argv[])
{
	if(argc != 2)
	{
		usage(argv[0]);
		return 1;	
	}
	int listen_sock = socket(AF_INET,SOCK_STREAM,0);
	assert(listen_sock > 0);
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(atoi(argv[1]));
//	printf("%s\n",argv[1]);	
	cout<<argv[1]<<endl;
	assert(0 == bind(listen_sock,(struct sockaddr*)&local,sizeof(local)));
	assert(0 == listen(listen_sock,5));
//	printf("listen\n");	
	while(1)
	{
		struct sockaddr peer;
		socklen_t len = sizeof(peer);
		int in;
		in = accept(listen_sock,&peer,&len);
	//	printf("accept\n");	
		pid_t pid = 0;
		pid = fork();
		if(pid > 0)//father
		{
			waitpid(pid,NULL,0);//等待子进程退出
			close(in);
		}
		else
		{
			pid_t id = fork();
			if(id > 0)//father
			{
				exit(0);
			}
			else//child
			{
				close(listen_sock);
				return proc_sock(in);
			}
		}
	}
}

