
#include "mux_os_type.h"
#include "mux_os_api.h"

#define MUTEX_COUNT 150
#define MUTEX_NAME_SIZE 16
BOOL g_MutexModuleInit = FALSE;

//Mutex APIs

#ifdef OS_ENVIRONMENT_LINUX

#include "pthread.h"
#include "time.h"

typedef struct _MUTEX_CONTAINER_STRUCT_
{
	unsigned int id;
	pthread_mutex_t id_mutex;
	char name[MUTEX_NAME_SIZE];
	BOOL used_state;
	
}MutexContainerStruct;

MutexContainerStruct g_aMutexs[MUTEX_COUNT];

void InitMutexs()
{
	int i;
	
	if(g_MutexModuleInit)
		return;
	
	for(i=0;i<MUTEX_COUNT;i++)
	{
		g_aMutexs[i].id = i;
		g_aMutexs[i].used_state = FALSE;
		memset(g_aMutexs[i].name,0,MUTEX_NAME_SIZE);
	}

	g_MutexModuleInit = TRUE;
}

int FindUnusedMutexIndex()
{
	int i;
	
	for(i=0;i<MUTEX_COUNT;i++)
	{
		if(!g_aMutexs[i].used_state)
			return i;
	}
	return -1;
}

OS_MUTEX_ID os_mutex_create(char *name)
{
	int mutex_id;
	
	if(!g_MutexModuleInit)
		InitMutexs();

	mutex_id = FindUnusedMutexIndex();
	if(mutex_id == -1)
		return OS_FAILURE;

	pthread_mutex_init(&(g_aMutexs[mutex_id].id_mutex), NULL);
	
	return mutex_id;
}

OS_STATUS os_mutex_delete(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;

	g_aMutexs[mid].used_state = FALSE;
	memset(g_aMutexs[mid].name,0,MUTEX_NAME_SIZE);
	pthread_mutex_destroy( &(g_aMutexs[mid].id_mutex) );
		
	return OS_SUCCESS;
}

OS_STATUS os_mutex_get(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;
	
	pthread_mutex_lock( &(g_aMutexs[mid].id_mutex) );
	
	return OS_SUCCESS;
}

OS_STATUS os_mutex_put(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;

	pthread_mutex_unlock( &(g_aMutexs[mid].id_mutex) );
	
	return OS_SUCCESS;
}


#endif

#ifdef OS_ENVIRONMENT_WIN32

typedef struct _MUTEX_CONTAINER_STRUCT_
{
	unsigned int id;
	CRITICAL_SECTION cs;
	HANDLE id_sema;
	char name[MUTEX_NAME_SIZE];
	BOOL used_state;
	
}MutexContainerStruct;

MutexContainerStruct g_aMutexs[MUTEX_COUNT];

void InitMutexs()
{
	int i;
	
	if(g_MutexModuleInit)
		return;
	
	for(i=0;i<MUTEX_COUNT;i++)
	{
		g_aMutexs[i].id = i;
		g_aMutexs[i].id_sema = NULL;
		g_aMutexs[i].used_state = FALSE;
		InitializeCriticalSection(&(g_aMutexs[i].cs));
		memset(g_aMutexs[i].name,0,MUTEX_NAME_SIZE);
	}

	g_MutexModuleInit = TRUE;
}

int FindUnusedMutexIndex()
{
	int i;
	
	for(i=0;i<MUTEX_COUNT;i++)
	{
		if(!g_aMutexs[i].used_state)
			return i;
	}
	return -1;
}

OS_MUTEX_ID os_mutex_create(char *name)
{
	int mutex_id;
	
	if(!g_MutexModuleInit)
		InitMutexs();

	mutex_id = FindUnusedMutexIndex();
	if(mutex_id == -1)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aMutexs[mutex_id].cs));
	g_aMutexs[mutex_id].id_sema = CreateSemaphore(NULL,1,1,NULL);
	g_aMutexs[mutex_id].used_state = TRUE;
	memcpy(g_aMutexs[mutex_id].name,name,strlen(name));
	LeaveCriticalSection(&(g_aMutexs[mutex_id].cs));
	
	return mutex_id;
}

OS_STATUS os_mutex_delete(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;

	EnterCriticalSection(&(g_aMutexs[mid].cs));
	g_aMutexs[mid].used_state = FALSE;
	CloseHandle(g_aMutexs[mid].id_sema);
	memset(g_aMutexs[mid].name,0,MUTEX_NAME_SIZE);
	LeaveCriticalSection(&(g_aMutexs[mid].cs));
		
	return OS_SUCCESS;
}

OS_STATUS os_mutex_get(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;

	WaitForSingleObject(g_aMutexs[mid].id_sema,INFINITE);
	
	return OS_SUCCESS;
}

OS_STATUS os_mutex_put(OS_MUTEX_ID mid)
{
	if(mid>=MUTEX_COUNT)
		return OS_FAILURE;

	if(!(g_aMutexs[mid].used_state))
		return OS_FAILURE;

	ReleaseSemaphore(g_aMutexs[mid].id_sema,1,NULL);
	
	return OS_SUCCESS;
}

#endif


