
#include "mux_os_type.h"
#include "mux_os_api.h"

#ifdef OS_ENVIRONMENT_LINUX

#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int vu_socket = -1;

//创建Socket连接
int CreateSocket()
{
	int mySocket;
	struct sockaddr_in local;
	int ret;

	int buff_size = 20480;
	
	local.sin_family=AF_INET;
	local.sin_port=htons(5432);
	local.sin_addr.s_addr=inet_addr("192.168.129.1");
//	local.sin_addr.s_addr=inet_addr("192.168.129.199");

	mySocket = socket(AF_INET,SOCK_STREAM,0);
	if(mySocket>=0)
	{
		ret=connect(mySocket,(struct sockaddr *)&local,sizeof(local));
		if(ret<0)
		{
			close(mySocket);
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

	return mySocket;
}

//关闭 Socket 连接
void CloseSocket(int mSocket)
{
	close(mSocket);
}

int VirtualDevice_Init(void)
{
	if( vu_socket == -1 )
		CloseSocket(vu_socket);

    vu_socket = CreateSocket();
    if( vu_socket >= 0 ) 
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

	if(vu_socket<0)
		return -1;
	
//	printf("begin read socket\n",ret);
	ret=recv(vu_socket,buff,len,0);
	if(ret > 0)
	{
		printf("	rcv %d bytes\n",ret);
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
	int ret=0;
	
	if( vu_socket < 0 )
		return -1;

	printf("\tsend %d bytes\n",len);
	ret = send(vu_socket,buff,len,0);

	return len;
}

int VirtualDevice_release(void)
{
	if( vu_socket < 0 )
		return -1;

	CloseSocket(vu_socket);
	vu_socket = -1;
	
	return 0;
}


#endif


#ifdef OS_ENVIRONMENT_WIN32

#ifdef WIN32_SOCKET_CLIENT

static SOCKET vu_socket = INVALID_SOCKET;

//创建Socket连接
SOCKET CreateSocket()
{
	SOCKET mySocket;
	struct sockaddr_in local;
	int ret;

	WSADATA wsaData;	
	int buff_size = 4096;
	
	local.sin_family=AF_INET;
	local.sin_port=htons(5432);
	local.sin_addr.S_un.S_addr=inet_addr("192.168.129.1");

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
//	else
//		printf("create socket error\n");

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

int VirtualDevice_Read(char *buff,int len,unsigned int timeout)
{
	int ret=0;
	
	if(vu_socket == INVALID_SOCKET)
		return 0;
	
	printf("\nbegin read socket buff ptr-%X, len-%d\n",buff,len);
	while(1)
	{
		ret=recv(vu_socket,buff,len,0);
		printf("\nread socket result-%d\n",buff,len);
		if(SOCKET_ERROR!=ret)
		{
			if(ret > 0)
			{
				printf("\n\n\trcv %d bytes\n\n",ret);
				return ret;
			}
		}
		else
		{
//			printf("	rcv error\n",ret);
			return 0;
		}
	}
}

int VirtualDevice_Write(char *buff,int len,unsigned int timeout)
{
	if( vu_socket == INVALID_SOCKET )
		return 0;

	printf("\n\n\tsend %d bytes\n\n",len);
	send(vu_socket,buff,len,0);

	return len;
}

int VirtualDevice_release(void)
{
	if( vu_socket != INVALID_SOCKET )
		CloseSocket(vu_socket);
	
	vu_socket = INVALID_SOCKET;
	return 0;
}

#else

static SOCKET vu_socket = INVALID_SOCKET;
static SOCKET listen_socket = INVALID_SOCKET;

//创建Socket连接
SOCKET CreateSocket()
{
	SOCKET mySocket;
	struct sockaddr_in local;
	int ret;
//	char hostname[128];
	WSADATA wsaData;	
	
//	os_mem_set(hostname,0,128);
//	ret = gethostname(hostname,128);
//	getaddrinfo(hostname,0,&local,NULL);
	local.sin_family=AF_INET;
	local.sin_port=htons(5432);
//	local.sin_addr.S_un.S_addr=inet_addr("127.0.0.1"); // 该地址用于Windows下单机调试MUX的TE和UE
	local.sin_addr.S_un.S_addr=inet_addr("192.168.129.1"); // 该地址用于让Linux上运行的TE端，可以连接Windows上运行的UE端

    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	{ 
         WSACleanup();
         return INVALID_SOCKET;
	}

	mySocket = socket(AF_INET,SOCK_STREAM,0);
	if(mySocket !=INVALID_SOCKET)
	{
		ret=bind(mySocket,(LPSOCKADDR)&local,sizeof(local));
		if(ret<0)
		{
			closesocket(mySocket);
			mySocket = INVALID_SOCKET;
			printf("bind socket failed !\n");
			return -1;
		}
		else
		{
			ret = listen(mySocket,1);
			if(ret < 0)
			{
				closesocket(mySocket);
				mySocket = INVALID_SOCKET;
				printf("listen failed !\n");
				return -1;
			}
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

void WaitForConnect(void *param)
{
	struct sockaddr_in local;
	int len;
	int buff_size = 20480;

	local.sin_family=AF_INET;
	len = sizeof(local);

	vu_socket = accept(listen_socket,(LPSOCKADDR)&local,&len);

	if(vu_socket < 0)
	{
		printf("accept socket failed !\n");
		vu_socket = INVALID_SOCKET;
	}

	printf("accept a socket success !\n");

	setsockopt(vu_socket,SOL_SOCKET,SO_SNDBUF,(const char *)&buff_size,sizeof(int));
	setsockopt(vu_socket,SOL_SOCKET,SO_RCVBUF,(const char *)&buff_size,sizeof(int));

	return;
}

int VirtualDevice_Init(void)
{
	if( listen_socket != INVALID_SOCKET )
		CloseSocket(vu_socket);

    listen_socket = CreateSocket();
    if( listen_socket != INVALID_SOCKET ) 
	{
		printf("create accept thread !\n");
		os_thread_create("socket",WaitForConnect,100,1024,NULL);

		return 0;
    } 
	else 
	{
		return -1;
    }
}

int VirtualDevice_Read(char *buff,int len,unsigned int timeout)
{
	int ret=0;
	if(vu_socket == INVALID_SOCKET)
		return 0;
//	printf("begin read socket\n",ret);
	while(1)
	{
		ret=recv(vu_socket,buff,len,0);
		if(SOCKET_ERROR!=ret)
		{
			if(ret > 0)
			{
				printf("\trcv %d bytes\n",ret);
				return ret;
			}
		}
		else
		{
			//			printf("	rcv error\n",ret);
			return 0;
		}
	}
}

int VirtualDevice_Write(char *buff,int len,unsigned int timeout)
{
	if( vu_socket == INVALID_SOCKET )
		return 0;

	printf("\tsend %d bytes\n",len);
	send(vu_socket,buff,len,0);

	return len;
}

int VirtualDevice_release(void)
{
	if( vu_socket != INVALID_SOCKET )
		CloseSocket(vu_socket);

	if( listen_socket != INVALID_SOCKET )
		CloseSocket(listen_socket);

	vu_socket = -1;
	listen_socket = -1;
	return 0;
}

#endif

#endif

