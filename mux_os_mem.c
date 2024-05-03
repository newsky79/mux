
#include "mux_os_type.h"
#include "mux_os_api.h"

//Memory management APIs
void *os_mem_malloc(unsigned int bytes)
{
	return malloc(bytes);
}

OS_STATUS os_mem_free(void *mem_ptr)
{
	if(mem_ptr==NULL)
		return OS_FAILURE;
	free(mem_ptr);
	return OS_SUCCESS;
}

void os_mem_set(void *dest_ptr, unsigned char value, unsigned int len)
{
	if(dest_ptr==NULL)
		return;
	
	memset(dest_ptr,value,len);
}

void os_mem_cpy(void *dest_ptr, void *src_ptr, unsigned int copy_len)
{
	if(dest_ptr==NULL || src_ptr ==NULL)
		return;
	
	memcpy(dest_ptr,src_ptr,copy_len);
}


