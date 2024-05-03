
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_api.h"

char *at_cmds_show[]=
{
	"\t[1] OK",
	"\t[2] +CME ERROR",
	"\t[3] CONNECT",
	"\t[4] NO CARRIER",
};

char *at_cmds[]=
{
	"\r\nOK\r\n",
	"\r\+CME ERROR\r\n",
	"\r\nCONNECT\r\n",
	"\r\NO CARRIER\r\n"
};

int my_event_callback(unsigned char dlci, unsigned char event_type, void *unused_ptr)
{
	char buff[512]; 
	int len;
	char *str;
	printf("dlci : %d; \tevent : %d\r\n",dlci,event_type);

	switch(event_type)
	{
	case MUX_EVENT_TYPE_DATA_RECEIVED:

		os_mem_set((void *)buff,0,512);
		len = mux_read(dlci,buff,512,50);
		if(len > 0)
		{
			printf("received data : ");
			printf(buff);
			printf("\n");
			str = strstr(buff,"AT+CMUX");
			if(str == NULL)
				mux_write(dlci,at_cmds[0],strlen(at_cmds[0]),0);
			else
			{
				str = strstr(buff,"=");
				str++;
				mux_on_recv_cmux_cmd(str,len - (str - buff));
			}
		}

		break;
	}
	return 0;
}

int main(int argc,char **argv)
{
	char input;
	OS_EVENT_ID id_event;
	char buff[512]; 
	int len;
	char *str;

	freopen("C:\\muxserver.log","w+",stdout);

	printf("Welcome !\r\n");
	
	if(0 == mux_init("UART"))
		printf("mux init success !\r\n");
	else
	{
		printf("mux init failed !\r\n");
		return -1;
	}
	mux_register_event_callback(my_event_callback);

	id_event = os_event_create("event",FALSE);
	
	while(1)
	{
		len = mux_read(0,buff,512,OS_WAIT_FOREVER);
		if(len > 0)
		{
			printf("received data : ");
			printf(buff);
			printf("\n");
			str = strstr(buff,"AT+CMUX");
			if(str != NULL)
			{
				str = strstr(buff,"=");
				str++;
				mux_on_recv_cmux_cmd(str,len - (str - buff));

				break;
			}
		}
		Sleep(2000);
	}
//	printf("When you want to stop it, please input character 'X' !\n");
//	printf("Your input : ");
	while(1)
	{
		os_event_get(id_event,OS_WAIT_FOREVER);
	}
	
	mux_exit();
	
	return 0;
}