#ifndef _MUX_FUNC_H_
#define _MUX_FUNC_H_

#include "mux_os_type.h"

#define MUX_NO_TIME_EXPIRE_MONITOR

#ifdef __cplusplus
extern "C" {
#endif

#define BIT_ORDER_EXCHANGE_BYTE(x) \
	((((x)&0x01)<<7)|(((x>>1)&0x01)<<6)|(((x>>2)&0x01)<<5)|(((x>>3)&0x01)<<4)| \
	(((x>>4)&0x01)<<3)|(((x>>5)&0x01)<<2)|(((x>>6)&0x01)<<1)|(((x>>7)&0x01)))

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)>(y)?(y):(x))

int mux_open_uart(char *filename);
int mux_close_uart();
int mux_read_uart(void *buffer, unsigned int count, unsigned int timeout);
int mux_write_uart(const void *buffer, unsigned int count, unsigned int timeout);
int mux_config_uart(void *port_para_ptr, unsigned int timeout);

int start_packet_thread();
int stop_packet_thread();

int mux_init_all_packet_list();
int mux_init_packet_list_ctrl(mux_dlc_packet_list_ctrl *packet_list_ctrl, char *name, int name_len);
mux_dlc_packet_list *get_first_packet(mux_dlc_packet_list_ctrl *packet_list_ctrl, unsigned int timeout);
int add_packet(mux_dlc_packet_list_ctrl *packet_list_ctrl, mux_dlc_packet_list * packet_node);

int mux_frame_init();
int mux_frame_exit();
int mux_send_ctrl_frame(unsigned char dlci,unsigned char control_field, unsigned char *msg, int msg_len);
int mux_send_uih_frame(unsigned char dlci,unsigned char cmd,char *data,int len,unsigned int timeout);

int mux_on_received_ctrl_frame(unsigned char dlci, unsigned char control_field, unsigned char *data, int data_len);

#ifdef __cplusplus
}
#endif 

#endif
