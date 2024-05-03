
#include "mux_os_type.h"
#include "mux_os_api.h"

#define SEMAPHORES_COUNT 200
#define SEMAPHORE_NAME_SIZE 16
BOOL g_SemaphoreModuleInit = FALSE;

//Semaphore APIs

#ifdef OS_ENVIRONMENT_LINUX

#include "semaphore.h"
#include "time.h"
#include "errno.h"
#include "linux/errno.h"

typedef struct _SEMAPHORE_CONTAINER_STRUCT_
{
	unsigned int id;
	sem_t id_sema;
	char name[SEMAPHORE_NAME_SIZE];
	int value;
	BOOL used_state;
	
}SemaphoreContainerStruct;

SemaphoreContainerStruct g_aSemaphores[SEMAPHORES_COUNT];

void InitSemaphores()
{
	int i;
	
	if(g_SemaphoreModuleInit)
		return;

	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		g_aSemaphores[i].id = i;
//		g_aSemaphores[i].id_sema. = 0;
		g_aSemaphores[i].value = 0;
		g_aSemaphores[i].used_state = FALSE;

		memset(g_aSemaphores[i].name,0,SEMAPHORE_NAME_SIZE);
	}

	g_SemaphoreModuleInit = TRUE;
}

int FindUnusedSemaphoreIndex()
{
	int i;
	
	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		if(!g_aSemaphores[i].used_state)
			return i;
	}
	return -1;
}

OS_SEMA_ID os_sema_create(char *name,unsigned int init_count)
{
	int sema_id;
	if(!g_SemaphoreModuleInit)
		InitSemaphores();

	sema_id = FindUnusedSemaphoreIndex();
	if(sema_id == -1)
		return OS_FAILURE;

	sem_init(&(g_aSemaphores[sema_id].id_sema),0,init_count);

	g_aSemaphores[sema_id].used_state = TRUE;
	g_aSemaphores[sema_id].value = init_count;

	memcpy(g_aSemaphores[sema_id].name,name,strlen(name));
	
	return sema_id;
}



OS_STATUS os_sema_delete(OS_SEMA_ID sid)
{
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;
	
	sem_destroy(&(g_aSemaphores[sid].id_sema));

	g_aSemaphores[sid].value = 0;
	g_aSemaphores[sid].used_state = FALSE;

	memset(g_aSemaphores[sid].name,0,SEMAPHORE_NAME_SIZE);
		
	return OS_SUCCESS;
}

// can use following function : 
// int sem_wait_i( &m_deques, mswait )
// return -1, if it failed
OS_STATUS os_sema_get(OS_SEMA_ID sid, unsigned int timeout)
{
	struct timespec time,ts;
	int ret;

#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, timeout-%X\n",sid,timeout);
#endif

	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;
	
	if(0 != sem_trywait(&(g_aSemaphores[sid].id_sema)))
	{
		if(timeout == OS_NO_WAIT)
			return OS_FAILURE;
		else if(timeout == OS_WAIT_FOREVER)
		{
			ret= sem_wait(&(g_aSemaphores[sid].id_sema));
			if(-1 == ret)
			{
				printf("os_sema_get : sid-%d,errno - %d,%s\n",errno,strerror(errno));
				return OS_FAILURE;
			}
			else
				printf("os_sema_get : sid-%d,sema signaled\n",errno,strerror(errno));
		}
		else
		{
			if(-1 == clock_gettime(CLOCK_REALTIME, &ts))
				return OS_FAILURE;
//			os_thread_sleep(timeout);
			
			time.tv_sec = timeout/1000;
			time.tv_nsec = (timeout%1000)*1000000;
				
			time.tv_nsec += ts.tv_nsec;
			if(time.tv_nsec >= 1000000000)
			{
				time.tv_nsec -= 1000000000;
				time.tv_sec += ts.tv_sec;
				time.tv_sec++;
			}
			else
				time.tv_sec += ts.tv_sec;
				
			if(0 != sem_timedwait(&(g_aSemaphores[sid].id_sema),&time))
//			if(0 != sem_trywait(&(g_aSemaphores[sid].id_sema)))
				return OS_FAILURE;
		}
	}
	
	return OS_SUCCESS;
}



OS_STATUS os_sema_put(OS_SEMA_ID sid)
{
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	sem_post(&(g_aSemaphores[sid].id_sema));
	
	return OS_SUCCESS;
}


#endif

#ifdef OS_ENVIRONMENT_WIN32

typedef struct SEMA_EVENT_LIST
{
	HANDLE id_event;
	struct SEMA_EVENT_LIST *next;

}SemaEventList;

typedef struct _SEMAPHORE_CONTAINER_STRUCT_
{
	unsigned int id;
	CRITICAL_SECTION cs;
	SemaEventList *head;
	SemaEventList *tail;
	char name[SEMAPHORE_NAME_SIZE];
	int value;
	BOOL used_state;
	
}SemaphoreContainerStruct;

SemaphoreContainerStruct g_aSemaphores[SEMAPHORES_COUNT];

void InitSemaphores()
{
	int i;
	
	if(g_SemaphoreModuleInit)
		return;
	
	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		g_aSemaphores[i].id = i;
		g_aSemaphores[i].head = NULL;
		g_aSemaphores[i].tail = NULL;
		g_aSemaphores[i].value = 0;
		g_aSemaphores[i].used_state = FALSE;

		InitializeCriticalSection(&(g_aSemaphores[i].cs));
		memset(g_aSemaphores[i].name,0,SEMAPHORE_NAME_SIZE);
	}

	g_SemaphoreModuleInit = TRUE;
}

int FindUnusedSemaphoreIndex()
{
	int i;
	
	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		if(!g_aSemaphores[i].used_state)
			return i;
	}
	return -1;
}

OS_SEMA_ID os_sema_create(char *name,unsigned int init_count)
{
	int sema_id;
	
	if(!g_SemaphoreModuleInit)
		InitSemaphores();

	sema_id = FindUnusedSemaphoreIndex();
	if(sema_id == -1)
		return OS_FAILURE;

	g_aSemaphores[sema_id].used_state = TRUE;
	g_aSemaphores[sema_id].value = init_count;

	memcpy(g_aSemaphores[sema_id].name,name,strlen(name));
	
	return sema_id;
}



OS_STATUS os_sema_delete(OS_SEMA_ID sid)
{
	SemaEventList *node;

	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	g_aSemaphores[sid].value = 0;
	g_aSemaphores[sid].used_state = FALSE;

	node = g_aSemaphores[sid].head;
	while(node != NULL)
	{
		g_aSemaphores[sid].head = node->next;
		CloseHandle(node->id_event);
		os_mem_free((void *)node);
		node = g_aSemaphores[sid].head;;
	}

	g_aSemaphores[sid].head = NULL;
	g_aSemaphores[sid].tail = NULL;
	memset(g_aSemaphores[sid].name,0,SEMAPHORE_NAME_SIZE);
		
	return OS_SUCCESS;
}

// can use following function : 
// int sem_wait_i( &m_deques, mswait )
// return -1, if it failed
OS_STATUS os_sema_get(OS_SEMA_ID sid, unsigned int timeout)
{
	DWORD dwRet;
	SemaEventList *node = NULL;
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, timeout-%X\n",sid,timeout);
#endif
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sid].cs));
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, value-%d\n",sid,g_aSemaphores[sid].value);
#endif
	g_aSemaphores[sid].value--;
	if(g_aSemaphores[sid].value < 0)
	{
		node = (SemaEventList *)os_mem_malloc(sizeof(SemaEventList));
		if(node == NULL)
		{
			g_aSemaphores[sid].value++;
			LeaveCriticalSection(&(g_aSemaphores[sid].cs));
			return OS_FAILURE;
		}
		node->next = NULL;
		
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, create new event node\n",sid);
#endif

		node->id_event = CreateEvent(NULL,FALSE, FALSE, NULL);

#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, event-%d\n",sid,node->id_event);
	printf("os_sema_get : sid-%d, head ptr - %X, tail ptr - %X\n",sid,g_aSemaphores[sid].head,g_aSemaphores[sid].tail);
#endif

		if(g_aSemaphores[sid].head == NULL)
		{
			g_aSemaphores[sid].head = node;
			g_aSemaphores[sid].tail = node;
		}
		else if(g_aSemaphores[sid].head == g_aSemaphores[sid].tail)
		{
			g_aSemaphores[sid].head->next = node;
			g_aSemaphores[sid].tail = g_aSemaphores[sid].head->next;
		}
		else
			g_aSemaphores[sid].tail->next = node;

		LeaveCriticalSection(&(g_aSemaphores[sid].cs));

		if(timeout == OS_WAIT_FOREVER)
		{
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, wait event-%d INFINITE\n",sid,node->id_event);
#endif
			dwRet = WaitForSingleObject(node->id_event,INFINITE);
		}
		else
			dwRet = WaitForSingleObject(node->id_event,timeout);
		
		EnterCriticalSection(&(g_aSemaphores[sid].cs));

#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, wait result-%d\n",sid,dwRet);
#endif

		if(dwRet == WAIT_TIMEOUT)
		{
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, wait timeout\n",sid);
#endif
			dwRet = OS_FAILURE;
		}
		else if(dwRet == WAIT_OBJECT_0)
		{
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, sema signaled\n",sid);
#endif
			dwRet = OS_SUCCESS;
		}
		else
			dwRet = OS_FAILURE;
		
		g_aSemaphores[sid].value++;

#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, head ptr - %X, tail ptr - %X\n",sid,g_aSemaphores[sid].head,g_aSemaphores[sid].tail);
#endif

		if(g_aSemaphores[sid].head != NULL)
		{
			g_aSemaphores[sid].head = g_aSemaphores[sid].head->next;
			if(g_aSemaphores[sid].head == NULL)
				g_aSemaphores[sid].tail = NULL;
		}
		else
			g_aSemaphores[sid].tail = NULL;

#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_get : sid-%d, node ptr - %X, event-%d\n",sid,node,node->id_event);
#endif

		CloseHandle(node->id_event);
		if(node != NULL)
			os_mem_free((void *)node);
		
		LeaveCriticalSection(&(g_aSemaphores[sid].cs));

		return dwRet;
	}
	else
	{
		LeaveCriticalSection(&(g_aSemaphores[sid].cs));
	}
	
	return OS_SUCCESS;
}



OS_STATUS os_sema_put(OS_SEMA_ID sid)
{
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_put : sid-%d\n",sid);
#endif

	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sid].cs));
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_put : sid-%d, value-%d\n",sid,g_aSemaphores[sid].value);
#endif
	g_aSemaphores[sid].value++;
//	if(g_aSemaphores[sid].value <= 0)
	{
		if(g_aSemaphores[sid].head != NULL)
		{
			g_aSemaphores[sid].value--;
			
#ifdef OS_DEBUG_SUPPORT
	printf("os_sema_put : sid-%d, event-%d\n",sid,g_aSemaphores[sid].head->id_event);
#endif

			SetEvent(g_aSemaphores[sid].head->id_event);
		}	
	}
	LeaveCriticalSection(&(g_aSemaphores[sid].cs));
	
	return OS_SUCCESS;
}


#endif

#ifdef OS_ENVIRONMENT_WIN32_BAK

typedef struct _SEMAPHORE_CONTAINER_STRUCT_
{
	unsigned int id;
	HANDLE id_sema;
	CRITICAL_SECTION cs;
	char name[SEMAPHORE_NAME_SIZE];
	int value;
	BOOL used_state;
	
}SemaphoreContainerStruct;

SemaphoreContainerStruct g_aSemaphores[SEMAPHORES_COUNT];

void InitSemaphores()
{
	int i;
	
	if(g_SemaphoreModuleInit)
		return;
	
	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		g_aSemaphores[i].id = i;
		g_aSemaphores[i].id_sema = NULL;
		g_aSemaphores[i].value = 0;
		g_aSemaphores[i].used_state = FALSE;
		InitializeCriticalSection(&g_aSemaphores[i].cs);

		memset(g_aSemaphores[i].name,0,SEMAPHORE_NAME_SIZE);
	}

	g_SemaphoreModuleInit = TRUE;
}

int FindUnusedSemaphoreIndex()
{
	int i;
	
	for(i=0;i<SEMAPHORES_COUNT;i++)
	{
		if(!g_aSemaphores[i].used_state)
			return i;
	}
	return -1;
}

OS_SEMA_ID os_sema_create(char *name,unsigned int init_count)
{
	int sema_id;
	if(!g_SemaphoreModuleInit)
		InitSemaphores();

	sema_id = FindUnusedSemaphoreIndex();
	if(sema_id == -1)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sema_id].cs));
	g_aSemaphores[sema_id].id_sema = CreateSemaphore(NULL,init_count,0x7FFFFFFF,NULL);
	g_aSemaphores[sema_id].used_state = TRUE;
	g_aSemaphores[sema_id].value = init_count;
	memcpy(g_aSemaphores[sema_id].name,name,strlen(name));
	LeaveCriticalSection(&(g_aSemaphores[sema_id].cs));
	
	return sema_id;
}



OS_STATUS os_sema_delete(OS_SEMA_ID sid)
{
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sid].cs));
	CloseHandle(g_aSemaphores[sid].id_sema);
	g_aSemaphores[sid].id_sema = NULL;
	g_aSemaphores[sid].value = 0;
	g_aSemaphores[sid].used_state = FALSE;
	memset(g_aSemaphores[sid].name,0,SEMAPHORE_NAME_SIZE);
	LeaveCriticalSection(&(g_aSemaphores[sid].cs));
	
	return OS_SUCCESS;
}

OS_STATUS os_sema_get(OS_SEMA_ID sid, unsigned int timeout)
{
	DWORD dwRet;
	
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sid].cs));
	g_aSemaphores[sid].value--;
	LeaveCriticalSection(&(g_aSemaphores[sid].cs));
	
	if(timeout == OS_WAIT_FOREVER)
		dwRet = WaitForSingleObject(g_aSemaphores[sid].id_sema,INFINITE);
	else
		dwRet = WaitForSingleObject(g_aSemaphores[sid].id_sema,timeout);

	if(dwRet != WAIT_OBJECT_0)
		return OS_FAILURE;
	
	return OS_SUCCESS;
}



OS_STATUS os_sema_put(OS_SEMA_ID sid)
{
	if(sid>=SEMAPHORES_COUNT)
		return OS_FAILURE;

	if(!(g_aSemaphores[sid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aSemaphores[sid].cs));
	g_aSemaphores[sid].value++;
	LeaveCriticalSection(&(g_aSemaphores[sid].cs));
	
	ReleaseSemaphore(g_aSemaphores[sid].id_sema,1,NULL);

	return OS_SUCCESS;
}


#endif



