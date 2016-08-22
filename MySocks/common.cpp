#include <iostream>
using namespace std;
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"
#include "transfer.h"
#include "protocol.h"

void enx(char *data,int len)
{//加密解密
	for(int i = 0;i < len;i++)
	{
		data[i] ^= 7;//跟固定数据异或
		
	}
}


int recv_n(int sock,char *buf,int len)
{
	char *cur = buf;
	int num = len;
	while(num > 0)
	{
		int ret = recv(sock,cur,num,0);
		if(ret <= 0)
		{
			return -1;
		}
		cur += ret;
		num -= ret;
	}
	enx(buf,len);//解密
	return 0;
}

int send_n(int sock,char *buf,int len)
{
	enx(buf,len);//加密
	const char *cur = buf;
	int num = len;
	while(num > 0)
	{
		int ret = send(sock,buf,num,0);
		if(ret <= 0)
		{
			return -1;
		}
		cur += ret;
		num -= ret;
	}
	return 0;
	
}
