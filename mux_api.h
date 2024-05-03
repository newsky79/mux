#ifndef _MUX_API_H_
#define _MUX_API_H_

#include "mux_os_type.h"
#include "mux_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define OS_DEBUG_SUPPORT

#define MAX_DLC_NUM 10

typedef enum MUX_EVENT_TYPE
{
	MUX_EVENT_TYPE_SERVICE_STARTING,
	MUX_EVENT_TYPE_SERVICE_STOPING,
	MUX_EVENT_TYPE_SERVICE_STARTED,
	MUX_EVENT_TYPE_SERVICE_STOPED,
	MUX_EVENT_TYPE_DLCI_OPENING,
	MUX_EVENT_TYPE_DLCI_CLOSING,
	MUX_EVENT_TYPE_DLCI_OPENED,
	MUX_EVENT_TYPE_DLCI_CLOSED,
	MUX_EVENT_TYPE_DATA_RECEIVED,
	MUX_EVENT_TYPE_ERROR
	
}MuxEventType;

typedef struct mux_info_st/*used to record MUX information*/
{
    unsigned char	operationg_mode;
	unsigned char	subset_frame_type;
	unsigned char	port_speed;
	unsigned short	n1_max_framesize;
	unsigned short	t1_ack_timer;
	unsigned char	n2_retrans_times;
	unsigned short	t2_resp_timer;
	unsigned short	t3_wakeup_resp_timer;
	unsigned char	k_windows_size;
	
} mux_info;
//param1 : dlci, 0 是MUX的系统事件,其他为指定dlci的事件
//param2 : 事件类型，MuxEventType中的一种
//param3 : reserved

typedef int (*MUX_EVENT_CALLBACK)(unsigned char , unsigned char, void *);

int mux_init(char *dev_name);
int mux_exit();

int mux_open(unsigned char dlci);
int mux_close(unsigned char dlci);

int mux_write(unsigned char dlci,char *data,int len,unsigned int timeout);
int mux_read(unsigned char dlci,char *buff,int len,unsigned int timeout);
int mux_control(unsigned char dlci,void *param,unsigned int timeout);

int mux_default_mux_info_st(mux_info *p_mux_info);
int mux_send_cmux_cmd(mux_info *p_mux_info);
int mux_on_recv_cmux_cmd(char *cmd_param,int param_len);

#ifdef __cplusplus
}
#endif 

#endif
