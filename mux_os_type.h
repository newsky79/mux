#ifndef _MUX_OS_TYPE_H_
#define _MUX_OS_TYPE_H_

#include <stdio.h>

//#define OS_ENVIRONMENT_LINUX

#ifndef OS_ENVIRONMENT_LINUX
	#define OS_ENVIRONMENT_WIN32
#endif

#define OS_SELF_PRINTF

#ifdef __cplusplus
extern "C" {
#endif

#define TP_OS_ASSERT(x)

#ifndef OS_ENVIRONMENT_WIN32

typedef int BOOL;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif


#ifndef NULL
#define NULL 0
#endif


typedef unsigned long OS_STATUS;
/*the data type of os_open return*/
typedef long OS_FD;

typedef void (*OS_THREAD_ENTRY)(void* param);
typedef void (*OS_FUNC_ENTRY)(void *);
typedef long OS_EVENT_ID;
typedef long OS_MUTEX_ID;
typedef long OS_QUEUE_ID;
typedef long OS_SEMA_ID;
typedef long OS_THREAD_ID;
typedef long OS_TIMER_ID;

#define OS_MAX_NAME_LEN 32


#define OS_NO_WAIT          0x00000000
#define OS_WAIT_FOREVER     0xffffffff
#define OS_AUTO_ACTIVATE    0x1
#define OS_DONT_ACTIVATE    0x0
#define OS_AUTO_LOAD        0x2
#define OS_DONT_LOAD        0x0

/* API return values.  */
#ifndef OS_SUCCESS
#define OS_SUCCESS          0x00000000
#endif

#ifndef OS_FAILURE
#define OS_FAILURE          0xffffffff
#endif

#ifdef __cplusplus
}
#endif 

#endif

