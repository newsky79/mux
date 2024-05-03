
#include "mux_os_type.h"
#include "mux_os_api.h"

#define THREADS_COUNT 20
#define THREAD_NAME_SIZE 16
BOOL g_ThreadModuleInit = FALSE;

//Thread APIs

#ifdef OS_ENVIRONMENT_LINUX

#include "pthread.h"
#include "time.h"

typedef struct _THREAD_CONTAINER_STRUCT_
{
	unsigned int id;
	pthread_t id_linux;
	char name[THREAD_NAME_SIZE];
	BOOL used_state;
	
}ThreadContainerStruct;

ThreadContainerStruct g_aThreads[THREADS_COUNT];


void InitThreads()
{
	int i;
	if(g_ThreadModuleInit)
		return;

	for(i=0;i<THREADS_COUNT;i++)
	{
		g_aThreads[i].id = i;
		g_aThreads[i].id_linux = 0;
		g_aThreads[i].used_state = FALSE;

		memset(g_aThreads[i].name,0,THREAD_NAME_SIZE);
	}

	g_ThreadModuleInit = TRUE;
}

int FindUnusedThreadIndex()
{
	int i;
	for(i=0;i<THREADS_COUNT;i++)
	{
		if(!g_aThreads[i].used_state)
			return i;
	}
	return -1;
}

OS_THREAD_ID os_thread_create(char *name, OS_THREAD_ENTRY entry, unsigned char pri, unsigned int size, void* param)
{
	pthread_t thread_t;
	pthread_attr_t thread_attr_t;
	thread_attr_t.__stacksize = size;
	int thread_id;
	
	if(!g_ThreadModuleInit)
		InitThreads();

	thread_id = FindUnusedThreadIndex();
	if(thread_id == -1)
		return OS_FAILURE;
		
	pthread_attr_init(&thread_attr_t);
	thread_attr_t.__stacksize = size;
	pthread_create(&thread_t,&thread_attr_t,(void *)entry,(void *)param);

	g_aThreads[thread_id].id_linux = thread_t;
	g_aThreads[thread_id].used_state = TRUE;
	memcpy(g_aThreads[thread_id].name,name,strlen(name));
	
	return thread_id;
}

OS_STATUS os_thread_delete(OS_THREAD_ID tid)
{
	int result;
	
	if(tid>=THREADS_COUNT)
		return OS_FAILURE;

	if(!(g_aThreads[tid].used_state))
		return OS_FAILURE;
	
	result = pthread_kill(g_aThreads[tid].id_linux,0);
	if(result != 0)
		return OS_FAILURE;
	
	g_aThreads[tid].id_linux = 0;
	g_aThreads[tid].used_state = FALSE;
	memset(g_aThreads[tid].name,0,THREAD_NAME_SIZE);
	
	return OS_SUCCESS;
}


OS_STATUS os_thread_sleep(unsigned int timeout)
{
	struct timespec ts;
	ts.tv_sec = timeout/1000;
	ts.tv_nsec = (timeout%1000)*1000000;
	nanosleep(&ts,NULL);
	return OS_SUCCESS;
}


#endif

#ifdef OS_ENVIRONMENT_WIN32

#ifdef __cplusplus
extern "C" {
#endif

#include "windows.h"

typedef struct _THREAD_CONTAINER_STRUCT_
{
	unsigned int id;
	HANDLE id_thread;
	OS_THREAD_ENTRY thread_entry;
	void *thread_param;
	char name[THREAD_NAME_SIZE];
	BOOL used_state;
	
}ThreadContainerStruct;

ThreadContainerStruct g_aThreads[THREADS_COUNT];


void InitThreads()
{
	int i;
	if(g_ThreadModuleInit)
		return;

	for(i=0;i<THREADS_COUNT;i++)
	{
		g_aThreads[i].id = i;
		g_aThreads[i].thread_entry = NULL;
		g_aThreads[i].thread_param = NULL;
		g_aThreads[i].id_thread = NULL;
		g_aThreads[i].used_state = FALSE;
	
		memset(g_aThreads[i].name,0,THREAD_NAME_SIZE);
	}

	g_ThreadModuleInit = TRUE;
}

int FindUnusedThreadIndex()
{
	int i;
	for(i=0;i<THREADS_COUNT;i++)
	{
		if(!g_aThreads[i].used_state)
			return i;
	}
	return -1;
}

DWORD default_win32_thread(void *param)
{
	unsigned int *id;
	id = param;

	(*(g_aThreads[*id].thread_entry))(g_aThreads[*id].thread_param);

	ExitThread(0);
	return 0;
}

OS_THREAD_ID os_thread_create(char *name, OS_THREAD_ENTRY entry, unsigned char pri, unsigned int size, void* param)
{
	int thread_id;
	
	if(!g_ThreadModuleInit)
		InitThreads();

	thread_id = FindUnusedThreadIndex();
	if(thread_id == -1)
		return OS_FAILURE;

	g_aThreads[thread_id].thread_entry = entry;
	g_aThreads[thread_id].thread_param = param;
 	g_aThreads[thread_id].id_thread = CreateThread(NULL,size,(LPTHREAD_START_ROUTINE)default_win32_thread,&g_aThreads[thread_id].id,0,NULL);
	g_aThreads[thread_id].used_state = TRUE;
	memcpy(g_aThreads[thread_id].name,name,strlen(name));
	
	return thread_id;
}

OS_STATUS os_thread_delete(OS_THREAD_ID tid)
{
	int result;
	
	if(tid>=THREADS_COUNT)
		return OS_FAILURE;

	if(!(g_aThreads[tid].used_state))
		return OS_FAILURE;
	result = CloseHandle(g_aThreads[tid].id_thread);
	if(result == 0)
		return OS_FAILURE;
	
	g_aThreads[tid].id_thread = NULL;
	g_aThreads[tid].thread_entry = NULL;
	g_aThreads[tid].thread_param = NULL;
	g_aThreads[tid].used_state = FALSE;
	memset(g_aThreads[tid].name,0,THREAD_NAME_SIZE);
	
	return OS_SUCCESS;
}


OS_STATUS os_thread_sleep(unsigned int timeout)
{
	Sleep(timeout);
	return OS_SUCCESS;
}

#ifdef __cplusplus
}
#endif 

#endif


