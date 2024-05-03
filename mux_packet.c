
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"

#define MUX_TRANS_THREAD_PRIO 100
#define MUX_TRANS_THREAD_STACK_SIZE 8192

#define MUX_RECV_DATA_BUFF_LEN 2048

#define MUX_READ_THREAD_SLEEP_TIME 50

mux_dlc_packet_list_ctrl g_recv_packet_list;
mux_dlc_packet_list_ctrl g_send_packet_list;

OS_THREAD_ID id_recv_thread = -1;
OS_THREAD_ID id_send_thread = -1;

unsigned char *recv_data_buff[MUX_RECV_DATA_BUFF_LEN];
int recv_data_buff_pos;

//从end_pos之后一个字节开始寻找数据包
int find_one_packet(char *data, int len, int *begin_pos, int *end_pos)
{
	char *data_ptr;
	unsigned short packet_len;
	unsigned char ch;
	unsigned short tmp;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!find_one_packet begin!!");
	os_util_printf("!!find_one_packet : len-%d; begin_pos-%d; end_pos-%d!!",len,*begin_pos,*end_pos);
#endif

	if((data == NULL) || (len <=3))
		return -1;

	if(*end_pos >= len-2)
		return -1;

	if(g_mux_info.operationg_mode == MUX_BASIC_MODE)
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!find_one_packet : basic frame!!");
#endif	
		*begin_pos = *end_pos + 1;
		data_ptr = data + *begin_pos;
		// find begin flag
		while((unsigned char)(*data_ptr) != MUX_FRAME_FLAG_BASIC)
		{
			if(data_ptr-data > len)
				return -1;
			
			data_ptr++;
		}

		*begin_pos = data_ptr - data;
		data_ptr++;//address octet
		data_ptr++;//control octet
		data_ptr++;//length octet

		ch = BIT_ORDER_EXCHANGE_BYTE(*data_ptr);
		packet_len = ch & 0x7F;
		if(!(ch & 0x80))
		{
			data_ptr++;//  the second length indicator
			ch = BIT_ORDER_EXCHANGE_BYTE(*data_ptr);
			tmp = (unsigned short)ch;
			tmp >>= 1;
			packet_len |= tmp;
		}
		data_ptr += packet_len;//information end
		data_ptr++;//fcs octet
		data_ptr++;//end flag;
		
		*end_pos= data_ptr-data;
		if(*end_pos >= len)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!find_one_packet : not a unabridged frame!!");
#endif	
			return -1;
		}
	}
	else if(g_mux_info.operationg_mode == MUX_ADVANCED_MODE)
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!find_one_packet : advanced frame!!");
#endif
		*begin_pos = *end_pos + 1;
		data_ptr = data + *begin_pos;
		// find begin flag
		while((unsigned char)(*data_ptr) != MUX_FRAME_FLAG_ADVANCED)
		{
			if(data_ptr-data > len)
			{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!find_one_packet : no begin flag!!");
#endif	
				return -1;
			}
			
			data_ptr++;
		}
		*begin_pos = data_ptr - data;
		data_ptr++;
		while((unsigned char)(*data_ptr) != MUX_FRAME_FLAG_ADVANCED)
		{
			if(data_ptr-data > len)
			{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!find_one_packet : no end flag!!");
#endif	
				return -1;
			}
			
			data_ptr++;
		}
		*end_pos= data_ptr-data;
		if(*end_pos >= len)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!find_one_packet : not a unabridged frame!!");
#endif	
			return -1;
		}
	}
	else
		return -1;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!find_one_packet end!!");
#endif

	return 0;
}

void mux_read_thread(void *param)
{
	int len;
	int packet_begin,packet_end,packet_len;
	unsigned char *dest,*src,*str;
	BOOL find_packet_state;
	int pos_last,len_last;
	mux_dlc_packet_list *packet_node;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_read_thread begin!!");
#endif	

	while(1)
	{
		os_thread_sleep(MUX_READ_THREAD_SLEEP_TIME);
		
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_read_thread : data ptr - %X, start pos-%d!!",recv_data_buff,recv_data_buff_pos);
#endif	
		len = mux_read_uart((void *)(recv_data_buff+recv_data_buff_pos),MUX_RECV_DATA_BUFF_LEN - recv_data_buff_pos, OS_WAIT_FOREVER);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_read_thread : read %d bytes from uart,saved pos - %d!!",len,recv_data_buff_pos);
#endif			
		if(len > 0)
		{
			recv_data_buff_pos += len;
			if(g_mux_service_state != MUX_SERVICE_STARTED)
			{
				packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
				packet_node->next = NULL;
				packet_node->mux_dlc_packet_data = (char *)os_mem_malloc(recv_data_buff_pos+1);
				os_mem_set(packet_node->mux_dlc_packet_data,0,recv_data_buff_pos+1);
				os_mem_cpy(packet_node->mux_dlc_packet_data, recv_data_buff, recv_data_buff_pos);
				packet_node->mux_dlc_packet_data_len = recv_data_buff_pos;

				recv_data_buff_pos = 0;

				add_packet(&g_recv_packet_list, packet_node);
				continue;
			}
			
			packet_begin = -1;
			packet_end = -1;
			packet_len = 0;
			find_packet_state = FALSE;
			while(-1 != find_one_packet((char *)recv_data_buff,recv_data_buff_pos,&packet_begin,&packet_end))
			{
#ifdef MUX_DEBUG_SUPPORT
				os_util_printf("!!mux_read_thread find_one_packet success : begin-%d, end-%d!!",packet_begin,packet_end);
#endif	
				packet_len = packet_end - packet_begin + 1;
				packet_node = (mux_dlc_packet_list *)os_mem_malloc(sizeof(mux_dlc_packet_list));
				packet_node->next = NULL;
				packet_node->mux_dlc_packet_data = (char *)os_mem_malloc(packet_len);
				packet_node->mux_dlc_packet_data_len = packet_len;
				packet_len = packet_begin;
				str = recv_data_buff;
				while(packet_len>0)
				{
					str++;
					packet_len--;
				}
				
#ifdef MUX_DEBUG_SUPPORT
				os_util_printf("!!mux_read_thread recv_data_buff %X, str  %X!!",recv_data_buff,str);
#endif	
				os_mem_cpy(packet_node->mux_dlc_packet_data, str, packet_node->mux_dlc_packet_data_len);
//				packet_node->mux_dlc_packet_data_len = packet_len;

#ifdef MUX_DEBUG_SUPPORT
//				os_util_printf("!!mux_read_thread find_one_packet success : %M!!",packet_node->mux_dlc_packet_data,packet_node->mux_dlc_packet_data_len);
#endif	

				add_packet(&g_recv_packet_list, packet_node);

				find_packet_state = TRUE;
				pos_last = packet_end;
			}
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_read_thread : find_packet_state - %d!!",find_packet_state);
#endif	
			if(!find_packet_state)
				continue;
			
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("mux_read_thread : reposition left data");
			os_util_printf("mux_read_thread : data end-%d, packet end-%d",recv_data_buff_pos,pos_last);
#endif	
			len_last = recv_data_buff_pos - pos_last -1;

			if(len_last <= 0)
			{
				recv_data_buff_pos = 0;
				continue;
			}
			
			pos_last += 1;
			
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("mux_read_thread : len_last-%d, pos_last-%d",len_last,pos_last);
#endif	
			dest = (char *)recv_data_buff;
			src = (char *)(recv_data_buff + pos_last);
			
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("mux_read_thread : dest ptr-%X, src ptr-%X",dest,src);
#endif				
			while(pos_last < recv_data_buff_pos)
			{
				*dest = *src;
				pos_last++;
			}
			recv_data_buff_pos = len_last;
			if(recv_data_buff_pos < 0)
				recv_data_buff_pos = 0;
		}			
	}
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_read_thread end!!");
#endif
}

void mux_send_thread(void *param)
{
	mux_dlc_packet_list *result;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_thread begin!!");
#endif

	while(1)
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_thread enter get_first_packet!!");
#endif
		result = get_first_packet(&g_send_packet_list,OS_WAIT_FOREVER);
		if(result != NULL)
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_send_thread get_first_packet success!!");
#endif
			mux_write_uart((const void *)result->mux_dlc_packet_data,result->mux_dlc_packet_data_len, OS_WAIT_FOREVER);

			os_mem_free(result->mux_dlc_packet_data);
			os_mem_free(result);
			result = NULL;
		}
		else
		{
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_send_thread no packet ready!!");
#endif
//			os_thread_sleep(100);
		}
	}
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_send_thread end!!");
#endif
}

int mux_init_all_packet_list()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_init_all_packet_list!!");
#endif

	mux_init_packet_list_ctrl(&g_recv_packet_list,"recv list",9);
	mux_init_packet_list_ctrl(&g_send_packet_list,"send list",9);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_init_all_packet_list read sema- %d; send sema-%d!!",g_recv_packet_list.id_sema_dlc_packet_list_access,g_send_packet_list.id_sema_dlc_packet_list_access);
#endif

	return 0;
}

int start_packet_thread()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!start_packet_thread!!");
#endif

	os_mem_set(recv_data_buff, 0, MUX_RECV_DATA_BUFF_LEN);
	recv_data_buff_pos = 0;

	mux_init_all_packet_list();
	
	id_recv_thread = os_thread_create("recv", mux_read_thread, MUX_TRANS_THREAD_PRIO, MUX_TRANS_THREAD_STACK_SIZE, NULL);
	if(id_recv_thread == -1)
		return -1;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!start_packet_thread : create read thread success!!");
#endif

	id_send_thread = os_thread_create("send", mux_send_thread, MUX_TRANS_THREAD_PRIO, MUX_TRANS_THREAD_STACK_SIZE, NULL);
	if(id_send_thread == -1)
	{
		os_thread_delete(id_recv_thread);
		return -1;
	}

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!start_packet_thread : create send thread success!!");
#endif

	return 0;
}

int stop_packet_thread()
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!stop_packet_thread!!");
#endif

	if(id_recv_thread>=0)
		os_thread_delete(id_recv_thread);

	if(id_send_thread>=0)
		os_thread_delete(id_send_thread);

	return 0;
}



