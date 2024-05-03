#ifndef _MUX_OS_API_H_
#define _MUX_OS_API_H_

#include "mux_os_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_ENVIRONMENT_WIN32
#include "windows.h"
#endif

#define OS_DEBUG_SUPPORT

#ifdef OS_ENVIRONMENT_WIN32
#ifdef OS_SELF_PRINTF
int os_util_printf(const char *,...);
#else
#define os_util_printf printf("\n"),printf
#endif
#endif

#ifdef OS_ENVIRONMENT_LINUX
#ifdef OS_SELF_PRINTF
int os_util_printf(const char *,...);
#else
#define os_util_printf printf("\n"),printf
#endif
#endif

//Thread APIs
OS_THREAD_ID os_thread_create(char *name, OS_THREAD_ENTRY entry, unsigned char pri, unsigned int size, void* param);
OS_STATUS os_thread_delete(OS_THREAD_ID tid);
OS_STATUS os_thread_sleep(unsigned int timeout);


//Semaphore APIs
OS_SEMA_ID os_sema_create(char *name,unsigned int init_count);
OS_STATUS os_sema_delete(OS_SEMA_ID sid);
OS_STATUS os_sema_get(OS_SEMA_ID sid, unsigned int timeout);
OS_STATUS os_sema_put(OS_SEMA_ID sid);

//Event APIs
OS_EVENT_ID os_event_create(char *name,BOOL flag);
OS_STATUS os_event_delete(OS_EVENT_ID eid);
OS_STATUS os_event_get(OS_EVENT_ID eid, unsigned int timeout);
OS_STATUS os_event_set(OS_EVENT_ID eid);

OS_MUTEX_ID os_mutex_create(char *name);
OS_STATUS os_mutex_delete(OS_MUTEX_ID mid);
OS_STATUS os_mutex_get(OS_MUTEX_ID mid);
OS_STATUS os_mutex_put(OS_MUTEX_ID mid);

//Memory management APIs
void *os_mem_malloc(unsigned int bytes);
OS_STATUS os_mem_free(void *mem_ptr);
void os_mem_set(void *dest_ptr, unsigned char value, unsigned int len);
void os_mem_cpy(void *dest_ptr, void *src_ptr, unsigned int copy_len);


//Timer APIs
OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag);
OS_STATUS os_timer_delete(OS_TIMER_ID id);
OS_STATUS os_timer_start(OS_TIMER_ID id);
OS_STATUS os_timer_stop(OS_TIMER_ID id);
OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval);

#ifdef __cplusplus
}
#endif 

#endif

