#include "VirtualUartPC.h"

#ifdef _BASE_ENVIRMENT_AXD_

#define ETHERNET_REG_INIT_ADDRESS  0xA0000000
#define ETHERNET_REG_WRITE_ADDRESS 0xA0000004
#define ETHERNET_REG_READ_ADDRESS  0xA0000008
#define ETHERNET_REG_STAT_ADDRESS  0xA000000C

int VirtualDevice_Init(void)
{
	int *loc = (int*)ETHERNET_REG_INIT_ADDRESS;
	*loc=0x0;//进行设备的初始化
	
	return 0;
}

int VirtualDevice_Read(char *buff,int len)
{
	char *portlocn = (char *)ETHERNET_REG_READ_ADDRESS;
	int *stat_ptr = (int *)ETHERNET_REG_STAT_ADDRESS;

	if(*stat_ptr == 0x00)
		return 0;
	else
	{
		buff[0]=*portlocn;

		return 1;
	}

}

int VirtualDevice_Write(char *buff,int len)
{
	int Pos=0;
	char *portlocn = (char *)ETHERNET_REG_WRITE_ADDRESS;
	
	while(Pos<len)
	{
		*portlocn=buff[Pos];
		Pos++;
	}
	return Pos;
}

int VirtualDevice_release(void)
{
	return 0;
}

#elif defined _BASE_ENVIRMENT_BOARD_

#ifdef _PFIT_USE_USB_COMM_

#include "os_type.h"
#include "os_api.h"
#include "comm.h"

SINT32 dev_id = -1;


int VirtualDevice_Init(void)
{
	SINT32 open_flag = 0;

	//open USB device 
	dev_id = tp_man_comm_open ( "USB", open_flag );
	if ( dev_id < 0 )
	{
		//open USB device failure
		return -1;
	}

	return 0;
}

int VirtualDevice_Read(char *buff,int len)
{
	SINT32 ret;

	ret = tp_man_comm_recv ( dev_id, buff, len, 1, -1 );
	if ( ret < 0 )
	{
		//recv error
		return 0;
	}
	else
		return ret;

}

int VirtualDevice_Write(char *buff,int len)
{
	SINT32 ret;

	ret = tp_man_comm_send ( dev_id, buff, len, 1, -1);
	if ( ret < 0 )
	{
		//recv error
		return 0;
	}
	else
		return ret;
}

int VirtualDevice_release(void)
{
	tp_man_comm_close ( dev_id );

	return 0;
}

#elif defined _PFIT_USE_DOWNLOAD_UART_COMM_

#include "os_type.h"
#include "os_api.h"

extern SINT32 dd_uarta_open(UINT32 op_flag);
extern SINT32 dd_uarta_release(VOID);
extern SINT32 dd_uarta_read(CHAR *user_buf, UINT32 count, UINT32 op_flag);
extern SINT32 dd_uarta_write(CHAR *user_buf, UINT32 count, UINT32 op_flag);

int VirtualDevice_Init(void)
{
	SINT32 open_flag = 0;
	int result = -1;

	//open download uart device 
	result = dd_uarta_open ( open_flag );

	return result;
}

int VirtualDevice_Read(char *buff,int len)
{
	SINT32 ret;

	ret = dd_uarta_read ( buff, len, 0 );
	if ( ret < 0 )
	{
		//recv error
		return 0;
	}
	else
		return ret;

}

int VirtualDevice_Write(char *buff,int len)
{
	SINT32 ret;

	ret = dd_uarta_write ( buff, len, 0 );
	if ( ret < 0 )
	{
		//recv error
		return 0;
	}
	else
		return ret;
}

int VirtualDevice_release(void)
{
	dd_uarta_release ( );

	return 0;
}

#endif


#else

#include "winsock2.h"
#include "stdio.h"

static SOCKET vu_socket = INVALID_SOCKET;

//创建Socket连接
SOCKET CreateSocket()
{
	SOCKET mySocket;
	struct sockaddr_in local;
	int ret;

	WSADATA wsaData;	
	int buff_size = 20480;
	
	local.sin_family=AF_INET;
	local.sin_port=htons(2006);
	local.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");

    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	{ 
         WSACleanup();
         return INVALID_SOCKET;
	}

	mySocket = socket(AF_INET,SOCK_STREAM,0);
	if(mySocket!=INVALID_SOCKET)
	{
		ret=connect(mySocket,(LPSOCKADDR)&local,sizeof(local));
		if(ret<0)
		{
			closesocket(mySocket);
			printf("can't connect server\n");
		}
		else
		{
			setsockopt(mySocket,SOL_SOCKET,SO_SNDBUF,(const char *)&buff_size,sizeof(int));
			setsockopt(mySocket,SOL_SOCKET,SO_RCVBUF,(const char *)&buff_size,sizeof(int));
			return mySocket;
		}
	}
	else
		printf("create socket error\n");

	return INVALID_SOCKET;
}

//关闭 Socket 连接
void CloseSocket(SOCKET mSocket)
{
	closesocket(mSocket);
	WSACleanup();
}

int VirtualDevice_Init(void)
{
	if( vu_socket != INVALID_SOCKET )
		CloseSocket(vu_socket);

    vu_socket = CreateSocket();
    if( vu_socket != INVALID_SOCKET ) 
	{
		return 0;
    } 
	else 
	{
		return -1;
    }
}

int VirtualDevice_Read(char *buff,int len)
{
	int ret=0;
	
//	printf("begin read socket\n",ret);
	ret=recv(vu_socket,buff,len,0);
	if(SOCKET_ERROR!=ret)
	{
//		printf("	rcv %d bytes\n",ret);
		return ret;
	}
	else
	{
//		printf("	rcv error\n",ret);
		return 0;
	}
}

int VirtualDevice_Write(char *buff,int len)
{
	if( vu_socket == INVALID_SOCKET )
		return 0;

	send(vu_socket,buff,len,0);

	return len;
}

int VirtualDevice_release(void)
{
	CloseSocket(vu_socket);
	return 0;
}

#endif