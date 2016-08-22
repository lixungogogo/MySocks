#ifndef __COMMON_H__
#define __COMMON_H__
//#pragma once
void enx(char *data,int len);
int recv_n(int sock,char *buf,int len);
int send_n(int sock,char *buf,int len);

struct up
{
	int in;
	int out;
	int bro;
};

struct down 
{
	int in;
	int out;
	int bro;
};
#endif
