#ifndef _MUX_TYPE_H_
#define _MUX_TYPE_H_

#include "mux_os_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MUX_CONTROL_CHANNEL_DLCI 0

#define MUX_STATE_CLOSE_DOWN             0
#define MUX_STATE_DISCONNECTED           1
#define MUX_STATE_DISCONNECTED_NEGO      2
#define MUX_STATE_CONNECT_PENDING        3
#define MUX_STATE_CONNECT                4
#define MUX_STATE_DISCONNECTED_PENDING   5

#define MUX_LAST_OPERATE_NULL 0x00
#define MUX_LAST_OPERATE_FAIL 0xFF

/*Frame or command type*/
#define MUX_FRAME_TYPE_SABM     0x2f /*Set Asynchronous Balanced Mode(the real type value need to &0xef)*/
#define MUX_FRAME_TYPE_UA       0x63 /*Unnumbered Acknowle-dge ment(the real type value need to &0xef)*/
#define MUX_FRAME_TYPE_DM       0x0f /*Disconnected Mode (the real type value need to &0xef)*/
#define MUX_FRAME_TYPE_DISC     0x43 /*Disconnect (the real type value need to &0xef)*/
#define MUX_FRAME_TYPE_UIH      0xef /*Unnumbered Information with Header check (the real type value need to &0xef)*/
#define MUX_MCC_MSG_TYPE_PN     0x81 /*(the real type value need to &0xfd)*/
#define MUX_MCC_MSG_TYPE_CLD    0xc1 /*(the real type value need to &0xfd)*/
#define MUX_MCC_MSG_TYPE_TEST   0x21 /*(the real type value need to &0xfd)*/
#define MUX_MCC_MSG_TYPE_MSC    0xe1 /*(the real type value need to &0xfd)*/
#define MUX_MCC_MSG_TYPE_NSC    0x11 /*(the real type value need to &0xfd)*/
#define MUX_MCC_MSG_TYPE_SNC    0xd1 /*(the real type value need to &0xfd)*/

#define MUX_FRAME_FLAG_ADVANCED     0x7e    /*The closing and opening flag for MUX frame.*/
#define MUX_FRAME_FLAG_BASIC 0xf9
#define MUX_CONTROL_OCTET  0x7d    /*A transparency identifier that identifies an octet occurring within
                                    a frame to which the following transparency procedure is applied.*/

#define MUX_BASIC_MODE    0
#define MUX_ADVANCED_MODE 1
#define MUX_DEFAULT_MODE  MUX_BASIC_MODE

#define MUX_FRAME_UIH   0
#define MUX_FRAME_UI    1
#define MUX_FRAME_I     2
#define MUX_FRAME_DEFAULT   MUX_FRAME_UIH

#define MUX_PORT_SPEED_9600   1
#define MUX_PORT_SPEED_19200  2
#define MUX_PORT_SPEED_38400  3
#define MUX_PORT_SPEED_57600  4
#define MUX_PORT_SPEED_115200 5
#define MUX_PORT_SPEED_230400 6
#define MUX_PORT_SPEED_DEFAULT MUX_PORT_SPEED_115200

#define MUX_DEFAULT_MAX_FRAME_SIZE_BASIC 31
#define MUX_DEFAULT_MAX_FRAME_SIZE_ADVANCED 31
#define MUX_MAX_MAX_FRAME_SIZE 32768
#define MUX_MIN_MAX_FRAME_SIZE 1

#define MUX_T1_DEFAULT_VALUE 10
#define MUX_T1_MAX_VALUE 255
#define MUX_T1_MIN_VALUE 1
#define MUX_T1_STEPVALUE 10

#define MUX_RETRANS_DEFAULT_TIMES 3
#define MUX_RETRANS_MAX_TIMES 100

#define MUX_T2_DEFAULT_VALUE 30
#define MUX_T2_MAX_VALUE 255
#define MUX_T2_MIN_VALUE 2
#define MUX_T2_STEPVALUE 10

#define MUX_T3_DEFAULT_VALUE 10
#define MUX_T3_MAX_VALUE 255
#define MUX_T3_MIN_VALUE 1
#define MUX_T3_STEPVALUE 10

#define MUX_WINDOWS_SIZE_DEFAULT_VALUE 2
#define MUX_WINDOWS_SIZE_MAX_VALUE 7
#define MUX_WINDOWS_SIZE_MIN_VALUE 1

#define MUX_UIH_FRAME_TYPE_DEFAULT 0
#define MUX_UIH_FRAME_TYPE_CMD 1
#define MUX_UIH_FRAME_TYPE_RESP 2

#define MUX_CL_TYPE_1   0
#define MUX_CL_TYPE_2   1 
#define MUX_CL_TYPE_3   2
#define MUX_CL_TYPE_4   3

#define MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN 16

typedef enum MUX_SERVICE_STATE
{
	MUX_SERVICE_DISABLED,
	MUX_SERVICE_STARTING,
	MUX_SERVICE_STARTED,
	MUX_SERVICE_STOPING,
	MUX_SERVICE_ERROR
}MuxServiceState;

typedef struct mux_dlc_para_st/*used to record DLC parameters*/
{
    unsigned short  max_frame_size;
    unsigned short  t1_time_out_value;
    unsigned char   frame_type;/*0:UIH;1:UI;2:I*/
    unsigned char   cl_type;/*convergence layer type*/ 
    unsigned char   priority;/*0~63*/
    unsigned char   n2_num; 
	
} mux_dlc_para;

typedef struct mux_dlc_packet_list_st
{
	struct mux_dlc_packet_list_st *next;

	unsigned char *mux_dlc_packet_data;
	int mux_dlc_packet_data_len;
		
}mux_dlc_packet_list;


typedef struct mux_dlc_packet_list_ctrl_st
{
	struct mux_dlc_packet_list_st *head;
	struct mux_dlc_packet_list_st *tail;

	OS_MUTEX_ID id_mutex;
	OS_SEMA_ID id_sema_dlc_packet_list_access;
	char name[MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN];
		
}mux_dlc_packet_list_ctrl;

//used to record DLC information
typedef struct mux_dlc_info_st
{
	char dlci; 
    BOOL used_flag; //0: not in use; 1: in use
    unsigned char  state; 
	unsigned char  last_operate;
    OS_SEMA_ID dlc_sema;//   DLC Mutex 
    OS_TIMER_ID dlc_t1_timer;

    BOOL flow_control_flag;//0:not in flow control; 1:in flow control          
    unsigned int max_bit_rate;// 0~8640kbps 
    unsigned int tx_len;//used to note the data has transferred for FC 
    unsigned int  times_of_t1_expiry;
	unsigned int  times_of_t2_expiry;
    unsigned int  times_of_ttest_expiry;//pulse test
  
    mux_dlc_packet_list_ctrl packet_list;
    mux_dlc_para dlc_para_st;// can be modified by PN   
	
} mux_dlc_info;

#ifdef __cplusplus
}
#endif 

#endif
