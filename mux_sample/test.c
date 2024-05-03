
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_type.h"
#include "mux_api.h"

static unsigned char index;
char data[512];
int len;
OS_TIMER_ID id_timer = -1;
unsigned char g_dlci;
OS_EVENT_ID id_event;

char *at_cmds_show[]=
{
	"\t[1] AT",
		"\t[2] AT+CREG=1",
		"\t[3] AT^BMODE=1",
		"\t[4] AT+CFUN=1,0",
		"\t[5] AT+CGDCONT=1,\"IP\",\"cmnet\",,0,0",
		"\t[6] AT+CGEQREQ=1,2,128,2200,0,0,0,1500,\"0E0\",\"0E0\",1,0,1",
		"\t[7] ATD*98*1#",
		"\t[8] +++",
		"\t[9] AT+CFUN=0,0",
};

char *at_cmds[]=
{
	"AT\r",
		"AT+CREG=1\r",
		"AT^BMODE=1\r",
		"AT+CFUN=1\r",
		"AT+CGDCONT=1,\"IP\",\"cmnet\",,0,0\r",
		"AT+CGEQREQ=1,2,128,2200,0,0,0,1500,\"0E0\",\"0E0\",1,0,1\r",
		"ATD*98*1#\r",
		"+++\r",
		"AT+CFUN=0,0\r",
};

void my_timer_func(void *param)
{
	unsigned char dlci;
	
	printf("\n*****my_timer_func*****\n");
	
	dlci = *((unsigned char *)param);
	
	if(dlci == 0)
	{
		mux_open(0);
		mux_open(1);
	}
		
	if(index == 0)
		index=1;

	if(index<10)
	{
		mux_open(index);
		printf("send data : %s\n",at_cmds[index-1]);
		mux_write(index,at_cmds[index-1],strlen(at_cmds[index-1]),OS_WAIT_FOREVER);
		index++;
	}
}

int my_event_callback(unsigned char dlci, unsigned char event_type, void *unused_ptr)
{
	
	printf("\n*****my_event_callback*****\n");
	printf("dlci : %d; \tevent : %d\r\n",dlci,event_type);
	
	if(event_type != MUX_EVENT_TYPE_DATA_RECEIVED)
		return 0;

	printf("\n");
	os_mem_set(data,0,512);
	len = mux_read(dlci,data,512,OS_WAIT_FOREVER);
	printf(data);
	printf("\n");
	g_dlci = dlci;

	if(dlci != 0)
	{
		dlci = dlci;
	}

	if(!strstr(data,"OK"))
		if(!strstr(data,"ERROR"))
			if(!strstr(data,"CONNECT"))
			return 0;

	if(id_timer != -1)
	{
		printf("my_event_callback : restart the timer %d begin\n",id_timer);
		os_timer_restart(id_timer,10);
		printf("my_event_callback : restart the timer %d end\n",id_timer);
	}
	else
	{
		printf("my_event_callback : start timer %d begin\n",id_timer);
		os_timer_start(id_timer);
		printf("my_event_callback : start timer %d end\n",id_timer);
	}
	
	return 0;
}

int main(int argc,char **argv)
{
	char input; 
	char *str;
	int port_id = 0;
	mux_info default_mux_info;
	char data[512];

//	freopen("C:\\muxclient.log","w+",stdout);
//	printf("Welcome !\r\n");
	
	if(0 == mux_init("UART"))
		;
//		printf("mux init success !\r\n");
	else
	{
//		printf("mux init failed !\r\n");
		return -1;
	}
	mux_register_event_callback(my_event_callback);
	id_event = os_event_create("event",FALSE);

	index = 1;
	printf("do create a new timer\n");
	id_timer = os_timer_create("test",my_timer_func,&g_dlci,10,0);

	mux_default_mux_info_st(&default_mux_info);
	default_mux_info.operationg_mode = 1;
	default_mux_info.n1_max_framesize = 4005;
	default_mux_info.t1_ack_timer = 240;
	default_mux_info.t2_resp_timer = 250;
	mux_send_cmux_cmd(&default_mux_info);

	os_event_get(id_event,OS_WAIT_FOREVER);
/*		printf("\r\nPlease select a AT command, or input \'X\' to exit this program:\r\n\r\n");
		for(index=0;index<8;index++)
		{
			printf(at_cmds_show[index]);
			printf("\r\n");
		}
		printf("\r\n");
		printf("Your select : ");
		//input = getchar();
		mux_read(0,data,512,0xFFFFFFFF);
		
		if(input=='X')
			return 0;
		else if((input>='1')&&(input<='8'))
			mux_write(1,at_cmds[input-'1'],strlen(at_cmds[input-'1']),0xFFFFFFFF);
		else
			printf("\r\n\r\nError selection !\r\n");
*/
	printf("\ndelete event\n");
	os_event_delete(id_event);
	
	printf("\nrun mux_exit\n");
	mux_exit();
	
	return 0;
}
