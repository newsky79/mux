
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"


int mux_init_packet_list(mux_dlc_packet_list *packet_list)
{
	packet_list->mux_dlc_packet_data = NULL;
	packet_list->mux_dlc_packet_data_len = 0;
	packet_list->next = NULL;
		
	return 0;
}

int mux_init_packet_list_ctrl(mux_dlc_packet_list_ctrl *packet_list_ctrl, char *name, int name_len)
{
	packet_list_ctrl->head = NULL;
	packet_list_ctrl->tail = NULL;
	os_mem_set(packet_list_ctrl->name,0,MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN);
	os_mem_cpy(packet_list_ctrl->name,name,MIN(name_len,(MUX_DLC_PACKET_LIST_CTRL_ST_NAME_LEN-1)));
	packet_list_ctrl->id_mutex = os_mutex_create("mutex");
	packet_list_ctrl->id_sema_dlc_packet_list_access = os_sema_create("sema",0);
		
	return 0;
}

mux_dlc_packet_list *get_first_packet(mux_dlc_packet_list_ctrl *packet_list_ctrl, unsigned int timeout)
{
	mux_dlc_packet_list *result = NULL;

#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!get_first_packet %s!!",packet_list_ctrl->name);
#endif	

	os_mutex_get(packet_list_ctrl->id_mutex);

	if(packet_list_ctrl == NULL)
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!get_first_packet %s: list pointer is null!!",packet_list_ctrl->name);
#endif	
		os_mutex_put(packet_list_ctrl->id_mutex);
		return NULL;
	}

	if(packet_list_ctrl->head == NULL)
	{
		if(timeout > 0)
		{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!get_first_packet %s: enter os_sema_get!!",packet_list_ctrl->name);
#endif	
			os_mutex_put(packet_list_ctrl->id_mutex);
			os_sema_get(packet_list_ctrl->id_sema_dlc_packet_list_access, timeout);
			os_mutex_get(packet_list_ctrl->id_mutex);
		}
		else
		{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!get_first_packet %s: empty list, return now!!",packet_list_ctrl->name);
#endif	
			os_mutex_put(packet_list_ctrl->id_mutex);
			return NULL;
		}
	}

	if(packet_list_ctrl->head == NULL)
	{
#ifdef MUX_DEBUG_SUPPORT
		os_util_printf("!!get_first_packet %s: empty after wait!!",packet_list_ctrl->name);
#endif	

		os_mutex_put(packet_list_ctrl->id_mutex);
		return NULL;
	}

	result = packet_list_ctrl->head;
	
	if(packet_list_ctrl->head == packet_list_ctrl->tail)
	{
		packet_list_ctrl->head = NULL;
		packet_list_ctrl->tail = NULL;
	}
	else
	{
		packet_list_ctrl->head = packet_list_ctrl->head->next;
	}

	result->next = NULL;
	os_mutex_put(packet_list_ctrl->id_mutex);
	
	return result;
}

int add_packet(mux_dlc_packet_list_ctrl *packet_list_ctrl, mux_dlc_packet_list * packet_node)
{
#ifdef MUX_DEBUG_SUPPORT
	os_util_printf("!!add_packet %s!!",packet_list_ctrl->name);
#endif	

	os_mutex_get(packet_list_ctrl->id_mutex);

	if((packet_list_ctrl == NULL) || (packet_node == NULL))
	{
		os_mutex_put(packet_list_ctrl->id_mutex);
		return -1;
	}

	if(packet_list_ctrl->tail == NULL)
	{
		packet_list_ctrl->head = packet_node;
		packet_list_ctrl->tail= packet_node;
		
	}
	else
	{
		packet_list_ctrl->tail->next = packet_node;
		packet_list_ctrl->tail = packet_list_ctrl->tail->next;
	}
	packet_list_ctrl->tail->next = NULL;

	os_mutex_put(packet_list_ctrl->id_mutex);
	
	os_sema_put(packet_list_ctrl->id_sema_dlc_packet_list_access);
	
	return 0;
}
