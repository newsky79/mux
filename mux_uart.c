
#include "mux_os_type.h"
#include "mux_os_api.h"
#include "mux_all.h"

extern int VirtualDevice_Init(void);
extern int VirtualDevice_release(void);
extern int VirtualDevice_Read(char *buff,int len, unsigned int timeout);
extern int VirtualDevice_Write(char *buff,int len, unsigned int timeout);


int mux_open_uart(char *filename)
{
	return VirtualDevice_Init();
}

int mux_close_uart()
{
	return VirtualDevice_release();
}

int mux_read_uart(void *buffer, unsigned int count, unsigned int timeout)
{
	int ret;
	
#ifdef MUX_DEBUG_SUPPORT
			os_util_printf("!!mux_read_uart : buffer-%X, count-%d, timeout-%X!!",buffer,count,timeout);
#endif	

	ret = VirtualDevice_Read(buffer,count,timeout);
	return ret;
}

int mux_write_uart(const void *buffer, unsigned int count, unsigned int timeout)
{
	int ret;
	ret = VirtualDevice_Write((void *)buffer,count,timeout);
	return ret;
}

int mux_config_uart(void *port_para_ptr, unsigned int timeout)
{
	return 0;
}

