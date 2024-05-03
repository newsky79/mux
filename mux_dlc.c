
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"


int mux_dlc_ctrl_handle_sabm(unsigned char dlci)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_sabm dlci-%d begin!!",dlci);
#endif

	(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_OPENING,NULL);
	if(0 == mux_send_ctrl_frame(dlci,MUX_FRAME_TYPE_UA,NULL,0))
	{
		g_mux_dlc_info[dlci].state = MUX_STATE_CONNECT;
		(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_OPENED,NULL);
	}
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_sabm dlci-%d end!!",dlci);
#endif
	return 0;
}

int mux_dlc_ctrl_handle_ua(unsigned char dlci)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_ua dlci-%d begin!!",dlci);
#endif

	if(g_mux_dlc_info[dlci].last_operate == MUX_FRAME_TYPE_SABM)
		os_sema_put(g_mux_dlc_info[dlci].dlc_sema);
	else if(g_mux_dlc_info[dlci].last_operate == MUX_FRAME_TYPE_DISC)
		os_sema_put(g_mux_dlc_info[dlci].dlc_sema);

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_ua dlci-%d end!!",dlci);
#endif

	return 0;
}

int mux_dlc_ctrl_handle_dm(unsigned char dlci)
{
	
	return 0;
}

int mux_dlc_ctrl_handle_disc(unsigned char dlci)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_disc dlci-%d begin!!",dlci);
#endif

	(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_CLOSING,NULL);
	if(0 == mux_send_ctrl_frame(dlci,MUX_FRAME_TYPE_UA,NULL,0))
	{
		g_mux_dlc_info[dlci].state = MUX_STATE_DISCONNECTED;
		(*g_mux_event_callback)(dlci,MUX_EVENT_TYPE_DLCI_CLOSED,NULL);
	}
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_disc dlci-%d end!!",dlci);
#endif
	return 0;
}

int mux_dlc_ctrl_handle_uih(unsigned char dlci, unsigned char *data, int data_len)
{
	unsigned char command;
	unsigned char len;
	unsigned char ch;
	unsigned char cmd_dlci;
	unsigned char *str;
	unsigned char *buff = NULL;
	unsigned char *dest;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_uih dlci-%d begin!!",dlci);
#endif

	if(dlci != 0)
		return -1;
	
	str = data;
	ch = *str;
	command = BIT_ORDER_EXCHANGE_BYTE(ch);
	command &= 0x3F;

	str++;
	ch = *str;
	len = ch>>1;
	
	str++;
	ch = *str;
	cmd_dlci = ch>>2;

	buff = (unsigned char *)os_mem_malloc(len + 2);
	if(buff == NULL)
		return -1;
	
	if(command == 0x07)
	{
		dest = buff;
		
		ch = BIT_ORDER_EXCHANGE_BYTE(command);
		ch |= 0x01;
		*dest = ch;
		dest++;

		ch = len<<1;
		ch |= 0x01;
		*dest = ch;
		dest++;

		os_mem_cpy(dest,str,len);
//		mux_send_uih_frame(dlci,MUX_UIH_FRAME_TYPE_RESP,buff,len+2,OS_WAIT_FOREVER);
	}

	os_mem_free((void *)buff);	
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_dlc_ctrl_handle_uih dlci-%d end!!",dlci);
#endif
	return 0;
}


int mux_on_received_ctrl_frame(unsigned char dlci, unsigned char control_field, unsigned char *data, int data_len)
{
	control_field &= 0xEF;
	
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_on_received_ctrl_frame!!");
	os_util_printf("!!mux_on_received_ctrl_frame : dlci-%X; control-%X!!",dlci,control_field);
#endif
	
	if(control_field == MUX_FRAME_TYPE_SABM)//SABM
		mux_dlc_ctrl_handle_sabm(dlci);
	else if(control_field == MUX_FRAME_TYPE_UA)//UA
		mux_dlc_ctrl_handle_ua(dlci);
	else if(control_field == MUX_FRAME_TYPE_DM)//DM
		mux_dlc_ctrl_handle_dm(dlci);
	else if(control_field == MUX_FRAME_TYPE_DISC)//DISC
		mux_dlc_ctrl_handle_disc(dlci);
	else if(control_field == MUX_FRAME_TYPE_UIH)//DISC
		mux_dlc_ctrl_handle_uih(dlci,data,data_len);
	else
	{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!mux_on_received_ctrl_frame error type!!");
#endif
		return -1;
	}
	return 0;
}


