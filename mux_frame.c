
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"

#define MUX_FRAME_PROCESS_THREAD_PRIO 100
#define MUX_FRAME_PROCESS_THREAD_STACK_SIZE 8192

#define MUX_FRAME_THREAD_SLEEP_TIME 100

OS_THREAD_ID id_frame_process_thread = -1;
//static mux_dlc_info g_mux_dlc_info[MAX_DLC_NUM];

static unsigned char crctable[256]={
0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,

0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19, 0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,

0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,

0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

unsigned char mux_calculate_fcs(unsigned char *data, int len)
{
	unsigned char fcs=0xFF;/*the FCS value*/
    unsigned char sub=0;/*temporary variable*/

    /*len is the number of bytes in the message, p points to message*/
    while (len--) 
    {
        sub=fcs^*data++;
        fcs=crctable[sub];
    }
    /*Ones complement*/
    fcs=0xFF-fcs;
    
    return fcs;
}

int mux_check_fcs(unsigned char *data, int len, unsigned char fcs)
{
	unsigned char lfcs=0xFF;
    unsigned char sub=0;/*temporary variable*/     

    while (len--) 
    {
        sub=lfcs ^ (*data++);
        lfcs=crctable[sub];
    }

    lfcs = crctable[lfcs ^ fcs];
    if (lfcs ==0xCF) /*FCS is OK*/
    {
        return 0;
    }
    else/*FCS is not OK*/
    {
        return -1;
    }
}

int mux_frame_process_basic_frame(unsigned char *data, int len)
{
	unsigned char dlci;
	unsigned char control;
	unsigned short data_len;
	unsigned short packet_len;
	unsigned short tmp;

	unsigned char *frame_ptr;
	unsigned char ch;

	mux_dlc_packet_list *packet_node = NULL;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame begin!!");
	os_util_printf("!!mux_frame_process_basic_frame data-%d; len-%d!!",data,len);
#endif


	if((data == NULL) || (len < 6))
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : error param!!");
#endif
		return -1;
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : valid param!!");
#endif

	frame_ptr = data;
	packet_len = len;

	if(*frame_ptr != MUX_FRAME_FLAG_BASIC) // begin flag
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : begin flag error!!");
#endif
		return -1;
	}
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : valid begin flag!!");
#endif

	ch = *frame_ptr; //address
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame : address-%X!!",ch);
#endif

//	dlci = BIT_ORDER_EXCHANGE_BYTE(ch);
	dlci = ch>>2;
	dlci &= 0x3F;
	
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame : dlci-%d!!",dlci);
#endif

	ch = *frame_ptr; // control
	frame_ptr++;
	control = ch;
//	control = BIT_ORDER_EXCHANGE_BYTE(ch);
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame : control-%X!!",control);
#endif

	ch = BIT_ORDER_EXCHANGE_BYTE(*frame_ptr);
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame : data_len 1st char-%X!!",ch);
#endif
	data_len = ch & 0x7F;
	if(!(ch & 0x80))
	{
		frame_ptr++;//  the second length indicator
		ch = BIT_ORDER_EXCHANGE_BYTE(*frame_ptr);
		tmp = (unsigned short)ch;
		tmp >>= 1;
		data_len |= tmp;
	}
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!mux_frame_process_basic_frame : data_len-%d!!",data_len);
#endif

	if(data_len > 0x7F)
	{
		if(data_len + 7 != packet_len)
			return -1;
	}
	else if(data_len > 0)
	{
		if(data_len + 6 != packet_len)
			return -1;
	}

	packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
	if(packet_node == NULL)
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!mux_frame_process_basic_frame failed : packet list node memory malloc error!!");
#endif
		return -1;
	}

	packet_node->next = NULL;
	packet_node->mux_dlc_packet_data_len = data_len;
	if(data_len > 0)
	{
		packet_node->mux_dlc_packet_data = (char *)os_mem_malloc(data_len+1);
		if(packet_node->mux_dlc_packet_data == NULL)
		{
			os_mem_free((void *)packet_node);

#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_frame_process_basic_frame failed : data buff malloc error!!");
#endif
			return -1;
		}
		os_mem_set(packet_node->mux_dlc_packet_data, 0, data_len+1);
		os_mem_cpy(packet_node->mux_dlc_packet_data, frame_ptr, data_len);
	}

	frame_ptr = frame_ptr + data_len;

	if(data_len > 0x7F)
	{
		 if(-1 == mux_check_fcs(data+1,4,*frame_ptr))
		 {
		 	os_mem_free((void *)packet_node->mux_dlc_packet_data);
			os_mem_free((void *)packet_node);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : fcs error!!");
#endif
		 	return -1;
		 }
	}
	else
	{
		 if(-1 == mux_check_fcs(data+1,3,*frame_ptr))
		 {
		 	os_mem_free((void *)packet_node->mux_dlc_packet_data);
			os_mem_free((void *)packet_node);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame failed : fcs error!!");
#endif
		 	return -1;
		 }
	}
	frame_ptr++;

	if(*frame_ptr != MUX_FRAME_FLAG_BASIC) // end flag
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!mux_frame_process_basic_frame failed : end flag error!!");
#endif
		return -1;
	}

	if(data_len > 0)
	{
/*		if(dlci == 0)
		{
			if(g_mux_service_state == MUX_SERVICE_STARTING)
			{
				if(NULL != strstr(packet_node->mux_dlc_packet_data,"OK"))
					g_mux_service_state = MUX_SERVICE_STARTED;
			}
			add_packet(&(g_mux_dlc_info[dlci].packet_list), packet_node);
			(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DATA_RECEIVED,NULL);
		}
		else */if(g_mux_dlc_info[dlci].state != MUX_STATE_CONNECT)
		{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame dlci %d not opened!!",dlci);
#endif
			mux_send_ctrl_frame(dlci,MUX_FRAME_TYPE_DM,NULL,0);
			return -1;
		}
		else
		{
			add_packet(&(g_mux_dlc_info[dlci].packet_list), packet_node);
			(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DATA_RECEIVED,NULL);
		}
		
		
	}
	else
	{
		mux_on_received_ctrl_frame(dlci,control,NULL,0);
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_basic_frame end!!");
#endif

	return 0;
		
}

int mux_frame_process_advanced_frame(unsigned char *data, int len)
{
	unsigned char dlci;
	unsigned char control;
	unsigned short data_len;
	unsigned short packet_len;

	unsigned char *frame_ptr;
	unsigned char ch;

	mux_dlc_packet_list *packet_node = NULL;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame begin!!");
	os_util_printf("!!mux_frame_process_advanced_frame data-%d; len-%d!!",data,len);
//	os_util_printf("!!mux_frame_process_advanced_frame %M!!",data,len);
#endif


	if((data == NULL) || (len < 5))
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame failed : error param!!");
#endif
		return -1;
	}
	
	frame_ptr = data;
	packet_len = len;

	if(*frame_ptr != MUX_FRAME_FLAG_ADVANCED) // begin flag
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame failed : begin flag error!!");
#endif
		return -1;
	}
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame : valid begin flag!!");
#endif

	ch = *frame_ptr; //address
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame : address-%X!!",ch);
#endif

//	dlci = BIT_ORDER_EXCHANGE_BYTE(ch);
	dlci = ch>>2;
	dlci &= 0x3F;
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame : dlci-%d!!",dlci);
#endif

	ch = *frame_ptr; // control
	control = ch;
//	control = BIT_ORDER_EXCHANGE_BYTE(ch);
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame : control-%X!!",control);
#endif

	data_len = len - 5;
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame : data_len-%X!!",data_len);
#endif
	if(data_len > 0)
	{
		packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
		if(packet_node == NULL)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_frame_process_advanced_frame failed : packet list node memory malloc error!!");
#endif
			return -1;
		}

		packet_node->next = NULL;
		packet_node->mux_dlc_packet_data_len = data_len;
		if(data_len > 0)
		{
			packet_node->mux_dlc_packet_data = (char *)os_mem_malloc(data_len+1);
			if(packet_node->mux_dlc_packet_data == NULL)
			{
				os_mem_free((void *)packet_node);

#ifdef MUX_DEBUG_SUPPORT
				os_util_printf("!!mux_frame_process_advanced_frame failed : data buff malloc error!!");
#endif
				return -1;
			}
			os_mem_set(packet_node->mux_dlc_packet_data, 0, data_len+1);
			os_mem_cpy(packet_node->mux_dlc_packet_data, frame_ptr, data_len);
		}
	}

	frame_ptr = frame_ptr + data_len;

	if(-1 == mux_check_fcs(data+1,2,*frame_ptr))
	{
	 	os_mem_free((void *)packet_node->mux_dlc_packet_data);
		os_mem_free((void *)packet_node);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame failed : fcs error!!");
#endif
		return -1;
	}
	frame_ptr++;

	if(*frame_ptr != MUX_FRAME_FLAG_ADVANCED) // end flag
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!mux_frame_process_advanced_frame failed : end flag error!!");
#endif
		return -1;
	}

	if(dlci != 0)
	{
		if(data_len > 0)
		{
			if(g_mux_dlc_info[dlci].state != MUX_STATE_CONNECT)
			{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame dlci %d not opened!!",dlci);
#endif
				mux_send_ctrl_frame(dlci,MUX_FRAME_TYPE_DM,NULL,0);
				return -1;
			}
			else
			{
				add_packet(&(g_mux_dlc_info[dlci].packet_list), packet_node);
				(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DATA_RECEIVED,NULL);
			}
		}
		else
		{
			if(g_mux_dlc_info[dlci].last_operate != MUX_LAST_OPERATE_NULL)
				mux_on_received_ctrl_frame(dlci,control,NULL,0);
			else
				return -1;
		}
		
		
	}
	else
	{
		if(g_mux_dlc_info[dlci].last_operate != MUX_LAST_OPERATE_NULL)
		{
			if(data_len > 0)
				mux_on_received_ctrl_frame(dlci,control,packet_node->mux_dlc_packet_data,packet_node->mux_dlc_packet_data_len);
			else
				mux_on_received_ctrl_frame(dlci,control,NULL,0);
		}
		else if(data_len > 0)
			mux_on_received_ctrl_frame(dlci,control,packet_node->mux_dlc_packet_data,packet_node->mux_dlc_packet_data_len);
		else
			return -1;
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_advanced_frame end!!");
#endif

	return 0;
		
}

void mux_frame_process_thread(void *param)
{
	mux_dlc_packet_list *packet_node = NULL;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_thread begin!!");
#endif

	while(1)
	{
		if(packet_node != NULL)
		{
			os_mem_free(packet_node->mux_dlc_packet_data);
			os_mem_free((void *)packet_node);
			packet_node = NULL;
		}

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_thread enter get_first_packet!!");
#endif

		packet_node = get_first_packet(&g_recv_packet_list,OS_WAIT_FOREVER);
		if(NULL == packet_node)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_frame_process_thread no packet ready!!");
#endif
//			os_thread_sleep(MUX_FRAME_THREAD_SLEEP_TIME);
			continue;
		}
		if(g_mux_service_state != MUX_SERVICE_STARTED)
		{
			if(strstr(packet_node->mux_dlc_packet_data,"OK"))
			{
				g_mux_service_state = MUX_SERVICE_STARTED;
				
#ifdef MUX_DEBUG_SUPPORT
				os_util_printf("!!mux_frame_process_thread mux service started!!");
#endif
			}
			add_packet(&(g_mux_dlc_info[0].packet_list), packet_node);
			packet_node = NULL;
			(*g_mux_event_callback)(0,MUX_EVENT_TYPE_DATA_RECEIVED,NULL);
			continue;
		}
		else if(g_mux_service_state == MUX_SERVICE_STARTED)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_frame_process_thread get_first_packet success!!");
#endif		

			if(g_mux_info.operationg_mode == MUX_BASIC_MODE)
				mux_frame_process_basic_frame(packet_node->mux_dlc_packet_data,packet_node->mux_dlc_packet_data_len);
			else
				mux_frame_process_advanced_frame(packet_node->mux_dlc_packet_data,packet_node->mux_dlc_packet_data_len);

#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_frame_process_thread free node!!");
#endif	

			os_mem_free(packet_node->mux_dlc_packet_data);
			os_mem_free(packet_node);
			packet_node = NULL;
		}
		else
		{
			os_mem_free(packet_node->mux_dlc_packet_data);
			os_mem_free(packet_node);
			packet_node = NULL;
		}
	}
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_process_thread end!!");
#endif
}

int mux_frame_init()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_init!!");
#endif

	id_frame_process_thread = os_thread_create("frame", mux_frame_process_thread, MUX_FRAME_PROCESS_THREAD_PRIO, MUX_FRAME_PROCESS_THREAD_STACK_SIZE, NULL);
	if(id_frame_process_thread == -1)
		return -1;

	return 0;
}

int mux_frame_exit()
{
	if(id_frame_process_thread>=0)
		os_thread_delete(id_frame_process_thread);

	return 0;
}

int mux_frame_assemble_basic_data_frame(unsigned char dlci,unsigned char cmd,unsigned char *data,int len,unsigned int timeout,mux_dlc_packet_list *packet_node)
{
	int packet_len;
	unsigned char ch;
	unsigned char *frame_ptr;
	
	if(packet_node == NULL)
		return -1;
	
	if((data == NULL) &&(len >0))
		return -1;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame begin!!");
#endif

	packet_node->next = NULL;
	packet_len = len + 6;
	if(len > 0x7F)
		packet_len++;
	
	packet_node->mux_dlc_packet_data_len = packet_len;
	packet_node->mux_dlc_packet_data=(char *)os_mem_malloc(packet_len);
	frame_ptr = packet_node->mux_dlc_packet_data;
	*frame_ptr = MUX_FRAME_FLAG_BASIC; // begin flag
	frame_ptr++;
//	ch = dlci | 0x80; // address
//	*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
	ch = BIT_ORDER_EXCHANGE_BYTE(0xC0);
	ch = (dlci<<2) | ch;
	*frame_ptr = ch;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : address byte-%X!!",ch);
#endif

	frame_ptr++;
	*frame_ptr = (unsigned char)MUX_FRAME_TYPE_UIH;
	frame_ptr++;
	
	// length indicator
	ch = ((char)len)&0x7F;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : data_len-%d!!",len);
#endif

	if(len > 0x7F)
	{
		*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : data_len 1st byte-%X!!",*frame_ptr);
#endif

		frame_ptr++;

		ch = (char)((len>>7)&0xFF);
		*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : data_len 2nd byte-%X!!",*frame_ptr);
#endif
		frame_ptr++;
	}
	else
	{
		ch |= 0x80;
		*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : data_len 1st byte-%X!!",*frame_ptr);
#endif

		frame_ptr++;
	}
	os_mem_cpy(frame_ptr, data, len);
	frame_ptr = frame_ptr + len;
	
	if(len > 0x7F)
		*frame_ptr = mux_calculate_fcs(packet_node->mux_dlc_packet_data+1,4);
	else
		*frame_ptr = mux_calculate_fcs(packet_node->mux_dlc_packet_data+1,3);
	frame_ptr++;
	
	*frame_ptr = MUX_FRAME_FLAG_BASIC; // end flag

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame end!!");
#endif

	return 0;
}

int mux_frame_assemble_advanced_data_frame(unsigned char dlci,unsigned char cmd,unsigned char *data,int len,unsigned int timeout,mux_dlc_packet_list *packet_node)
{
	int packet_len;
	unsigned char ch;
	unsigned char *frame_ptr;
	
	if(packet_node == NULL)
		return -1;
	
	if((data == NULL) &&(len >0))
		return -1;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_data_frame begin!!");
#endif

	packet_node->next = NULL;
	packet_len = len + 5;
	if(len > 0x7F)
		packet_len++;
	
	packet_node->mux_dlc_packet_data_len = packet_len;
	packet_node->mux_dlc_packet_data=(char *)os_mem_malloc(packet_len);
	frame_ptr = packet_node->mux_dlc_packet_data;
	*frame_ptr = MUX_FRAME_FLAG_ADVANCED; // begin flag
	frame_ptr++;
//	ch = dlci | 0xC0; // address
//	*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
	ch = BIT_ORDER_EXCHANGE_BYTE(0xC0);
	ch = (dlci<<2) | ch;
	*frame_ptr = ch;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_data_frame : address byte-%X!!",ch);
#endif

	frame_ptr++;
	*frame_ptr = (unsigned char)MUX_FRAME_TYPE_UIH;

	if(cmd == MUX_UIH_FRAME_TYPE_CMD)
		*frame_ptr |= 0x10;
	else if(cmd == MUX_UIH_FRAME_TYPE_RESP)
		*frame_ptr &= 0xEF;
	
	frame_ptr++;
	
	os_mem_cpy(frame_ptr, data, len);
	frame_ptr = frame_ptr + len;
	
	*frame_ptr = mux_calculate_fcs(packet_node->mux_dlc_packet_data+1,2);
	frame_ptr++;
	
	*frame_ptr = MUX_FRAME_FLAG_ADVANCED; // end flag

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_data_frame end!!");
#endif

	return 0;
}


int mux_send_uih_frame(unsigned char dlci,unsigned char cmd,char *data,int len,unsigned int timeout)
{
	mux_dlc_packet_list *packet_node;
	int ret;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_uih_frame!!");
#endif

	packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
	if(packet_node == NULL)
		return -1;

	if(g_mux_info.operationg_mode == MUX_BASIC_MODE)
		ret = mux_frame_assemble_basic_data_frame(dlci,cmd,data,len,timeout,packet_node);
	else
		ret = mux_frame_assemble_advanced_data_frame(dlci,cmd,data,len,timeout,packet_node);
	
	if(-1 == ret)
	{
		os_mem_free((void *)packet_node);
		return -1;
	}

	add_packet(&g_send_packet_list, packet_node);
	return 0;
}

int mux_frame_assemble_basic_ctrl_frame(unsigned char dlci,unsigned char control_field,mux_dlc_packet_list *packet_node)
{
	int packet_len;
	unsigned char ch;
	unsigned char *frame_ptr;
	
	if(packet_node == NULL)
		return -1;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_ctrl_frame begin!!");
#endif

	packet_node->next = NULL;
	packet_len = 6;
	
	packet_node->mux_dlc_packet_data_len = packet_len;
	packet_node->mux_dlc_packet_data=(char *)os_mem_malloc(packet_len);
	frame_ptr = packet_node->mux_dlc_packet_data;
	*frame_ptr = MUX_FRAME_FLAG_BASIC; // begin flag
	frame_ptr++;
//	ch = dlci | 0x80; // address
//	*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
	ch = BIT_ORDER_EXCHANGE_BYTE(0xC0);
	ch = (dlci<<2) | ch;
	*frame_ptr = ch;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_data_frame : address byte-%X!!",ch);
#endif

	frame_ptr++;
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_ctrl_frame : control type-%X!!",control_field);
#endif
	*frame_ptr = control_field;
	frame_ptr++;
	
	// length indicator
	ch = 0x80;
	*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_ctrl_frame : data_len 1st byte-%X!!",*frame_ptr);
#endif

	frame_ptr++;
	// fcs
	*frame_ptr = mux_calculate_fcs(packet_node->mux_dlc_packet_data+1,2);
	frame_ptr++;
	
	*frame_ptr = MUX_FRAME_FLAG_BASIC; // end flag

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_basic_ctrl_frame end!!");
#endif

	return 0;
}

int mux_frame_assemble_advanced_ctrl_frame(unsigned char dlci,unsigned char control_field, unsigned char *msg, int msg_len,mux_dlc_packet_list *packet_node)
{
	int packet_len;
	unsigned char ch;
	unsigned char *frame_ptr;
	
	if(packet_node == NULL)
		return -1;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame begin!!");
#endif

	packet_node->next = NULL;
	packet_len = 5+msg_len;
	
	packet_node->mux_dlc_packet_data_len = packet_len;
	packet_node->mux_dlc_packet_data=(char *)os_mem_malloc(packet_len);
	os_mem_set(packet_node->mux_dlc_packet_data,0,packet_node->mux_dlc_packet_data_len);
	frame_ptr = packet_node->mux_dlc_packet_data;
	*frame_ptr = MUX_FRAME_FLAG_ADVANCED; // begin flag
	frame_ptr++;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame : dlci-%d!!",dlci);
#endif

//	ch = dlci | 0xC0; // address
//	*frame_ptr = BIT_ORDER_EXCHANGE_BYTE(ch);
	ch = BIT_ORDER_EXCHANGE_BYTE(0xC0);
	ch = (dlci<<2) | ch;
	*frame_ptr = ch;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame : address byte-%X!!",*frame_ptr);
#endif

	frame_ptr++;
	*frame_ptr = control_field|0x10;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame : control type-%X!!",*frame_ptr);
#endif
	frame_ptr++;
	
	// message field
	if(msg_len > 0)
	{
		os_mem_cpy(frame_ptr,msg,msg_len);
		frame_ptr += msg_len;
	}
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame : msg_len-%d!!",msg_len);
#endif
	// fcs
	*frame_ptr = mux_calculate_fcs(packet_node->mux_dlc_packet_data+1,2);
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame : fcs-%X!!",*frame_ptr);
#endif	

	frame_ptr++;
	*frame_ptr = MUX_FRAME_FLAG_ADVANCED; // end flag

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_frame_assemble_advanced_ctrl_frame end!!");
#endif

	return 0;
}

void ctrl_frame_expired_func(void *param)
{
	unsigned char dlci;
	dlci = *((unsigned char*)param);
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!ctrl_frame_expired_func : dlci-%d; last opeater-%d!!",dlci,g_mux_dlc_info[dlci].last_operate);
#endif

	g_mux_dlc_info[dlci].last_operate = MUX_LAST_OPERATE_FAIL;

	os_sema_put(g_mux_dlc_info[dlci].dlc_sema);
}

int mux_send_ctrl_frame(unsigned char dlci,unsigned char control_field, unsigned char *msg, int msg_len)
{
	mux_dlc_packet_list *packet_node;
	BOOL wait_resp = TRUE;
	int ret;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame!!");
#endif

	packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
	if(packet_node == NULL)
		return -1;

	if(g_mux_info.operationg_mode == MUX_BASIC_MODE)
		ret = mux_frame_assemble_basic_ctrl_frame(dlci,control_field,packet_node);
	else
		ret = mux_frame_assemble_advanced_ctrl_frame(dlci,control_field,msg,msg_len,packet_node);;

	if(-1 == ret)
	{
		os_mem_free((void *)packet_node);
		return -1;
	}
	if(control_field == MUX_FRAME_TYPE_SABM)
	{
		(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_OPENING,NULL);
		
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame : set dlci state-%X!!",MUX_STATE_CONNECT_PENDING);
#endif

		g_mux_dlc_info[dlci].state = MUX_STATE_CONNECT_PENDING;
	}
	else if(control_field == MUX_FRAME_TYPE_DISC)
	{
		(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_CLOSING,NULL);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame : set dlci state-%X!!",MUX_STATE_DISCONNECTED_PENDING);
#endif

		g_mux_dlc_info[dlci].state = MUX_STATE_DISCONNECTED_PENDING;
	}

	if((control_field != MUX_FRAME_TYPE_UA) && (control_field != MUX_FRAME_TYPE_DM))
	{
		wait_resp = TRUE;
		g_mux_dlc_info[dlci].last_operate = control_field;
	}
	else
		wait_resp = FALSE;
	
#ifndef MUX_NO_TIME_EXPIRE_MONITOR

	if(wait_resp && (g_mux_dlc_info[dlci].dlc_t1_timer == -1) )
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame : create t1 timer!!");
#endif	
		g_mux_dlc_info[dlci].dlc_t1_timer = os_timer_create("timer",
															ctrl_frame_expired_func,
															(void *)(&(g_mux_dlc_info[dlci].dlci)),
															g_mux_info.t1_ack_timer*MUX_T1_STEPVALUE,
															0);
	}
#endif

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame : entering add_packet!!");
#endif

	add_packet(&g_send_packet_list, packet_node);

#ifndef MUX_NO_TIME_EXPIRE_MONITOR

	if(wait_resp)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_ctrl_frame : start t1 timer!!");
#endif		
		os_timer_start(g_mux_dlc_info[dlci].dlc_t1_timer);
	}
	
#endif	

	return 0;
}
