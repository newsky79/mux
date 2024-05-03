#ifndef _MUX_VARS_H_
#define _MUX_VARS_H_

#include "mux_os_type.h"
#include "mux_type.h"

#ifdef __cplusplus
extern "C" {
#endif

extern mux_info g_mux_info;
extern mux_dlc_info g_mux_dlc_info[MAX_DLC_NUM];

extern MuxServiceState g_mux_service_state;

extern mux_dlc_packet_list_ctrl g_recv_packet_list;
extern mux_dlc_packet_list_ctrl g_send_packet_list;

extern MUX_EVENT_CALLBACK g_mux_event_callback;

#ifdef __cplusplus
}
#endif 

#endif
