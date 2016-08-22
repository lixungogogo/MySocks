#pragma pack(push)
#pragma pack(1)//网络字节没有空白，字节对齐为1
typedef struct SKReq
{
	char ver;
	char n;
}SKReq;

typedef struct SKRep
{
	char ver;
	char m;
}SKRep;

typedef struct AddReq
{
	//VER|CMD|RSV|ATYP|DST.ADDR|DST.PORT
	char ver;
	char cmd;
	char rsv;
	char atype;
}AddReq;

typedef struct AddRep
{
	char ver;
	char cmd;
	char rsv;
	char atype;
	int addr;
	short port;
}AddRep;

