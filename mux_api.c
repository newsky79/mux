
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"

typedef enum CMUX_PARAM_INDEX
{
	CMUX_PARAM_INDEX_MODE,
	CMUX_PARAM_INDEX_SUBSET,
	CMUX_PARAM_INDEX_PORT_SPEED,
	CMUX_PARAM_INDEX_N1_MAX_FRAMESIZE,
	CMUX_PARAM_INDEX_T1_ACK_TIMER,
	CMUX_PARAM_INDEX_N2_RETRANS_TIMES,
	CMUX_PARAM_INDEX_T2_RESP_TIMER,
	CMUX_PARAM_INDEX_T3_WAKEUP_RESP_TIMER,
	CMUX_PARAM_INDEX_K_WINDOWS_SIZE,
	CMUX_PARAM_INDEX_ERROR
}CmuxParamIndex;

mux_info g_mux_info;
mux_dlc_info g_mux_dlc_info[MAX_DLC_NUM];
MUX_EVENT_CALLBACK g_mux_event_callback = NULL;

BOOL g_mux_init_state = FALSE;
MuxServiceState g_mux_service_state;

int mux_register_event_callback(MUX_EVENT_CALLBACK func)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_register_event_callback!!");
#endif

	g_mux_event_callback = func;
	return 0;
}

void mux_init_all_vars()
{
	int dlci;
	char name[MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN];

	for(dlci = 0; dlci < MAX_DLC_NUM; dlci++)
	{
		g_mux_dlc_info[dlci].dlci = dlci;
		g_mux_dlc_info[dlci].used_flag = FALSE;
		g_mux_dlc_info[dlci].flow_control_flag = FALSE;
		g_mux_dlc_info[dlci].dlc_sema = os_sema_create("sema", 0);
		g_mux_dlc_info[dlci].dlc_t1_timer = -1;
		g_mux_dlc_info[dlci].max_bit_rate = MUX_PORT_SPEED_115200;
		g_mux_dlc_info[dlci].tx_len = 0;
		g_mux_dlc_info[dlci].state = MUX_STATE_CLOSE_DOWN;
		g_mux_dlc_info[dlci].last_operate = MUX_LAST_OPERATE_NULL;
		g_mux_dlc_info[dlci].times_of_t1_expiry = 0;
		g_mux_dlc_info[dlci].times_of_t2_expiry = 0;
		g_mux_dlc_info[dlci].times_of_ttest_expiry = 0;

		g_mux_dlc_info[dlci].dlc_para_st.frame_type = MUX_FRAME_DEFAULT;
		g_mux_dlc_info[dlci].dlc_para_st.max_frame_size = MUX_DEFAULT_MAX_FRAME_SIZE_BASIC;
		g_mux_dlc_info[dlci].dlc_para_st.cl_type = MUX_CL_TYPE_1;
		g_mux_dlc_info[dlci].dlc_para_st.n2_num = 0;
		g_mux_dlc_info[dlci].dlc_para_st.t1_time_out_value = MUX_T1_DEFAULT_VALUE*MUX_T1_STEPVALUE;
		g_mux_dlc_info[dlci].dlc_para_st.priority = (dlci>>3)<<3+7;

		os_mem_set(name,0,MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN);
		sprintf(name,"dlci %d list",dlci);
		mux_init_packet_list_ctrl(&(g_mux_dlc_info[dlci].packet_list),name,strlen(name));
	}
}


int mux_start_service()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_start_service begin!!");
#endif

	if(!g_mux_init_state)
		return -1;
	
	if(start_packet_thread()==-1)
	{
		return -1;
	}
	
	mux_frame_init();
	
//	g_mux_dlc_info[MUX_CONTROL_CHANNEL_DLCI].used_flag = TRUE;
//	g_mux_dlc_info[MUX_CONTROL_CHANNEL_DLCI].state = MUX_STATE_CONNECT;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_start_service end!!");
#endif

	return 0;
}

int mux_stop_service()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_stop_service begin!!");
#endif

	if(!g_mux_init_state)
		return -1;
	
	stop_packet_thread();
	mux_frame_exit();
	g_mux_dlc_info[MUX_CONTROL_CHANNEL_DLCI].used_flag = FALSE;
	g_mux_dlc_info[MUX_CONTROL_CHANNEL_DLCI].state = MUX_STATE_CLOSE_DOWN;
	
	g_mux_service_state = MUX_SERVICE_DISABLED;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_stop_service end!!");
#endif

	return 0;
}

int mux_default_mux_info_st(mux_info *p_mux_info)
{
	if(p_mux_info == NULL)
		return -1;
	
	p_mux_info->operationg_mode = MUX_DEFAULT_MODE;
	p_mux_info->subset_frame_type = MUX_FRAME_DEFAULT;
	p_mux_info->port_speed = MUX_PORT_SPEED_DEFAULT;
	p_mux_info->n1_max_framesize = MUX_DEFAULT_MAX_FRAME_SIZE_BASIC;
	p_mux_info->t1_ack_timer = MUX_T1_DEFAULT_VALUE;
	p_mux_info->n2_retrans_times = MUX_RETRANS_DEFAULT_TIMES;
	p_mux_info->t2_resp_timer = MUX_T2_DEFAULT_VALUE;
	p_mux_info->t3_wakeup_resp_timer = MUX_T3_DEFAULT_VALUE;
	p_mux_info->k_windows_size = MUX_WINDOWS_SIZE_DEFAULT_VALUE;
	
	return 0;
}

int mux_init(char *dev_name)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_init start!!");
#endif

	if(g_mux_init_state)
		return 0;
	
	if(mux_open_uart(dev_name) == -1)
		return -1;
	
	g_mux_service_state = MUX_SERVICE_DISABLED;
	mux_init_all_vars();
	
	g_mux_init_state = TRUE;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_init end!!");
#endif

	return 0;
}

int mux_exit()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_exit!!");
#endif

	if(!g_mux_init_state)
		return 0;
	
	if(g_mux_service_state != MUX_SERVICE_DISABLED)
		mux_stop_service();
	
	mux_close_uart();
	g_mux_init_state = FALSE;
	return 0;
}


int mux_open(unsigned char dlci)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open %d begin!!",dlci);
#endif

	if(!g_mux_init_state)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open : uninited!!");
#endif
		return -1;
	}

	if(g_mux_service_state != MUX_SERVICE_STARTED)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open : service not started!!");
#endif	
		return -1;
	}
/*
	if(dlci == MUX_CONTROL_CHANNEL_DLCI)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open dlci error-%d!!",dlci);
#endif	
		return -1;
	}
*/
	if(g_mux_dlc_info[dlci].state == MUX_STATE_CONNECT)
		return 0;

	if(g_mux_dlc_info[dlci].last_operate != MUX_LAST_OPERATE_NULL)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open begin : dlci %d is busy in %d!!",dlci,g_mux_dlc_info[dlci].last_operate);
#endif
		return -1;
	}
	
	mux_send_ctrl_frame(dlci, MUX_FRAME_TYPE_SABM,NULL,0);

	os_sema_get(g_mux_dlc_info[dlci].dlc_sema,OS_WAIT_FOREVER);
	
#ifndef MUX_NO_TIME_EXPIRE_MONITOR
	os_timer_delete(g_mux_dlc_info[dlci].dlc_t1_timer);
	g_mux_dlc_info[dlci].dlc_t1_timer = -1;
#endif

	if(g_mux_dlc_info[dlci].last_operate == MUX_LAST_OPERATE_FAIL)
	{
		g_mux_dlc_info[dlci].last_operate = MUX_LAST_OPERATE_NULL;
		return -1;
	}
	g_mux_dlc_info[dlci].state = MUX_STATE_CONNECT;
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open %d end!!",dlci);
#endif

	return 0;
}

int mux_close(unsigned char dlci)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_close begin!!");
#endif

	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state != MUX_SERVICE_STARTED)
		return -1;
/*
	if(dlci == MUX_CONTROL_CHANNEL_DLCI)
		return -1;
*/
	if((g_mux_dlc_info[dlci].state == MUX_STATE_CLOSE_DOWN) || (g_mux_dlc_info[dlci].state == MUX_STATE_DISCONNECTED))
		return 0;

	if(g_mux_dlc_info[dlci].last_operate != MUX_LAST_OPERATE_NULL)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_open begin : dlci %d is busy in %d!!",dlci,g_mux_dlc_info[dlci].last_operate);
#endif
		return -1;
	}
	
	mux_send_ctrl_frame(dlci, MUX_FRAME_TYPE_DISC,NULL,0);

	os_sema_get(g_mux_dlc_info[dlci].dlc_sema,OS_WAIT_FOREVER);

	os_timer_delete(g_mux_dlc_info[dlci].dlc_t1_timer);
	g_mux_dlc_info[dlci].dlc_t1_timer = -1;

	if(g_mux_dlc_info[dlci].last_operate == MUX_LAST_OPERATE_FAIL)
	{
		g_mux_dlc_info[dlci].last_operate = MUX_LAST_OPERATE_NULL;
		return -1;
	}
	g_mux_dlc_info[dlci].state = MUX_STATE_DISCONNECTED;

	return 0;
}


int mux_write(unsigned char dlci,char *data,int len,unsigned int timeout)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_write begin!!");
#endif

	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state == MUX_SERVICE_DISABLED)
		return mux_write_uart((void *)data,len,timeout);
	else if(g_mux_service_state == MUX_SERVICE_STARTED)
	{
		if(dlci == MUX_CONTROL_CHANNEL_DLCI)
			return -1;

		if(0 == mux_send_uih_frame(dlci,MUX_UIH_FRAME_TYPE_DEFAULT,data,len,timeout))
			return len;
	}
	else
	{
		return -1;
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_write end!!");
#endif

	return 0;
}

int mux_read(unsigned char dlci,char *buff,int len,unsigned int timeout)
{
	mux_dlc_packet_list *packet_node = NULL;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_read begin!!");
#endif

	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state == MUX_SERVICE_DISABLED)
		return mux_read_uart((void *)buff,len,timeout);
	else if(g_mux_service_state == MUX_SERVICE_STARTED)
	{
		packet_node = get_first_packet(&(g_mux_dlc_info[dlci].packet_list), timeout);
		if(packet_node == NULL)
			return 0;

		len = MIN(packet_node->mux_dlc_packet_data_len,len);
		os_mem_cpy(buff, packet_node->mux_dlc_packet_data, len);
		os_mem_free(packet_node->mux_dlc_packet_data);
		os_mem_free(packet_node);
		packet_node = NULL;
		return len;
	}
	else
	{
		return -1;
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_read end!!");
#endif

	return 0;
}


int mux_control(unsigned char dlci,void *param,unsigned int timeout)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_control begin!!");
#endif

	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state == MUX_SERVICE_DISABLED)
		mux_config_uart(param,timeout);
	else if(g_mux_service_state == MUX_SERVICE_STARTED)
	{
		if(dlci == MUX_CONTROL_CHANNEL_DLCI)
			return -1;
	
	}
	else
	{
		return -1;
	}

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_control end!!");
#endif
	
	return 0;
}

int mux_send_cmux_cmd(mux_info *p_mux_info)
{
	char cmd_data[128];
	int len;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_cmux_cmd begin!!");
#endif

	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state != MUX_SERVICE_DISABLED)
		return -1;

	g_mux_service_state = MUX_SERVICE_STARTING;
	
	os_mem_set(cmd_data,0,128);
/*	len = sprintf(cmd_data,"AT+CMUX=%d,%d,%d,%d,%d,%d,%d,%d,%d\r",
			p_mux_info->operationg_mode,
			p_mux_info->subset_frame_type,
			p_mux_info->port_speed,
			p_mux_info->n1_max_framesize,
			p_mux_info->t1_ack_timer,
			p_mux_info->n2_retrans_times,
			p_mux_info->t2_resp_timer,
			p_mux_info->t3_wakeup_resp_timer,
			p_mux_info->k_windows_size);
*/
	len = sprintf(cmd_data,"AT+CMUX=%d,%d,%d,%d,%d,%d,%d\r",
			p_mux_info->operationg_mode,
			p_mux_info->subset_frame_type,
			p_mux_info->port_speed,
			p_mux_info->n1_max_framesize,
			p_mux_info->t1_ack_timer,
			p_mux_info->n2_retrans_times,
			p_mux_info->t2_resp_timer);

	mux_start_service();
	
	mux_write_uart(cmd_data, len, (p_mux_info->t1_ack_timer)*MUX_T1_STEPVALUE);
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_cmux_cmd end!!");
#endif
	os_mem_cpy(&g_mux_info,p_mux_info,sizeof(mux_info));
	return 0;
}

int mux_on_recv_cmux_cmd(char *cmd_param,int param_len)
{
	char *pos_ptr;
	char buff[10];
	int buff_index;
	CmuxParamIndex param_type_index;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_on_recv_cmux_cmd begin!!");
#endif

	if((cmd_param == NULL) || (param_len <= 0))
		return -1;
	
	if(!g_mux_init_state)
		return -1;

	if(g_mux_service_state != MUX_SERVICE_DISABLED)
		return -1;

	g_mux_service_state = MUX_SERVICE_STARTING;

	pos_ptr = cmd_param;
	param_type_index = CMUX_PARAM_INDEX_MODE;
	os_mem_set((void *)buff,0,10);
	buff_index = 0;

	while(0 < param_len--)
	{
		if(*pos_ptr == ',')
		{
			switch(param_type_index)
			{
			case CMUX_PARAM_INDEX_MODE:
				g_mux_info.operationg_mode = atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_SUBSET:
				g_mux_info.subset_frame_type= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_PORT_SPEED:
				g_mux_info.port_speed= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_N1_MAX_FRAMESIZE:
				g_mux_info.n1_max_framesize= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_T1_ACK_TIMER:
				g_mux_info.t1_ack_timer= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_N2_RETRANS_TIMES:
				g_mux_info.n2_retrans_times= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_T2_RESP_TIMER:
				g_mux_info.t2_resp_timer= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_T3_WAKEUP_RESP_TIMER:
				g_mux_info.t3_wakeup_resp_timer= atoi(buff);
				break;
				
			case CMUX_PARAM_INDEX_K_WINDOWS_SIZE:
				g_mux_info.k_windows_size= atoi(buff);
				break;

			default:
					g_mux_service_state = MUX_SERVICE_DISABLED;
					return -1;
			}

			os_mem_set((void *)buff,0,10);
			buff_index = 0;
			param_type_index++;
		}
		else
		{
			buff[buff_index++] = *pos_ptr;
		}

		pos_ptr++;
	}

	if(param_type_index == CMUX_PARAM_INDEX_K_WINDOWS_SIZE)
		g_mux_info.k_windows_size= atoi(buff);

	mux_start_service();
	g_mux_service_state = MUX_SERVICE_STARTED;

	mux_send_uih_frame(MUX_CONTROL_CHANNEL_DLCI,MUX_UIH_FRAME_TYPE_DEFAULT,"\r\nOK\r\n", 6,0);
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_on_recv_cmux_cmd end!!");
#endif

	return 0;
}


