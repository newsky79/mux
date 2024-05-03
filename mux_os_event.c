
#include "mux_os_type.h"
#include "mux_os_api.h"

#define EVENTS_COUNT 50
#define EVENT_NAME_SIZE 16
BOOL g_EventModuleInit = FALSE;

//Event APIs
#ifdef OS_ENVIRONMENT_LINUX

typedef struct _EVENT_CONTAINER_STRUCT_
{
	unsigned int id;
	unsigned int id_sema;
	char name[EVENT_NAME_SIZE];
	BOOL used_state;
	
}EventContainerStruct;

EventContainerStruct g_aEvent[EVENTS_COUNT];

void InitEvents()
{
	int i;
	
	if(g_EventModuleInit)
		return;

	for(i=0;i<EVENTS_COUNT;i++)
	{
		g_aEvent[i].id = i;
		g_aEvent[i].id_sema = 0;
		g_aEvent[i].used_state = FALSE;

		memset(g_aEvent[i].name,0,EVENT_NAME_SIZE);
	}

	g_EventModuleInit = TRUE;
}

int FindUnusedEventIndex()
{
	int i;
	
	for(i=0;i<EVENTS_COUNT;i++)
	{
		if(!g_aEvent[i].used_state)
			return i;
	}
	return -1;
}


OS_EVENT_ID os_event_create(char *name,BOOL flag)
{
	int event_id;
	int sema_id;
	if(!g_EventModuleInit)
		InitEvents();

	event_id = FindUnusedEventIndex();
	if(event_id == -1)
		return OS_FAILURE;

	sema_id = os_sema_create(name, 0);
	if(OS_FAILURE == event_id)
		return OS_FAILURE;

	g_aEvent[event_id].id_sema = sema_id;
	g_aEvent[event_id].used_state = TRUE;

	memcpy(g_aEvent[event_id].name,name,strlen(name));
	
	return event_id;
}

OS_STATUS os_event_delete(OS_EVENT_ID eid)
{
	unsigned int result;
	
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;

	result = os_sema_delete(g_aEvent[eid].id_sema);
	if(result == OS_FAILURE)
		return OS_FAILURE;

	g_aEvent[eid].id_sema = 0;
	g_aEvent[eid].used_state = FALSE;

	memset(g_aEvent[eid].name,0,EVENT_NAME_SIZE);
	
	return OS_SUCCESS;
}

OS_STATUS os_event_get(OS_EVENT_ID eid, unsigned int timeout)
{
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;
	return os_sema_get(g_aEvent[eid].id_sema, OS_WAIT_FOREVER);
}

OS_STATUS os_event_set(OS_EVENT_ID eid)
{
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;
	
	return os_sema_put(g_aEvent[eid].id_sema);
}

#endif

#ifdef OS_ENVIRONMENT_WIN32

typedef struct _EVENT_CONTAINER_STRUCT_
{
	unsigned int id;
	HANDLE id_event;
	char name[EVENT_NAME_SIZE];
	BOOL used_state;
	
}EventContainerStruct;

EventContainerStruct g_aEvent[EVENTS_COUNT];

void InitEvents()
{
	int i;
	
	if(g_EventModuleInit)
		return;

	for(i=0;i<EVENTS_COUNT;i++)
	{
		g_aEvent[i].id = i;
		g_aEvent[i].id_event = NULL;
		g_aEvent[i].used_state = FALSE;

		memset(g_aEvent[i].name,0,EVENT_NAME_SIZE);
	}

	g_EventModuleInit = TRUE;
}

int FindUnusedEventIndex()
{
	int i;
	
	for(i=0;i<EVENTS_COUNT;i++)
	{
		if(!g_aEvent[i].used_state)
			return i;
	}
	return -1;
}


OS_EVENT_ID os_event_create(char *name,BOOL flag)
{
	int event_id;
	if(!g_EventModuleInit)
		InitEvents();

	event_id = FindUnusedEventIndex();
	if(event_id == -1)
		return OS_FAILURE;

	g_aEvent[event_id].id_event = CreateEvent(NULL,FALSE, FALSE, NULL);
	g_aEvent[event_id].used_state = TRUE;

	memcpy(g_aEvent[event_id].name,name,strlen(name));
	
	return event_id;
}

OS_STATUS os_event_delete(OS_EVENT_ID eid)
{
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;

	CloseHandle(g_aEvent[eid].id_event);
	g_aEvent[eid].used_state = FALSE;

	memset(g_aEvent[eid].name,0,EVENT_NAME_SIZE);
	
	return OS_SUCCESS;
}

OS_STATUS os_event_get(OS_EVENT_ID eid, unsigned int timeout)
{
	DWORD dwRet;
	
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;

	if(timeout == OS_WAIT_FOREVER)
		dwRet = WaitForSingleObject(g_aEvent[eid].id_event,INFINITE);
	else
		dwRet = WaitForSingleObject(g_aEvent[eid].id_event,timeout);

	if(dwRet == WAIT_TIMEOUT)
		return OS_FAILURE;
	
	return OS_SUCCESS;
}

OS_STATUS os_event_set(OS_EVENT_ID eid)
{
	if(eid>=EVENTS_COUNT)
		return OS_FAILURE;

	if(!(g_aEvent[eid].used_state))
		return OS_FAILURE;
	SetEvent(g_aEvent[eid].id_event);

	return OS_SUCCESS;
}

#endif



