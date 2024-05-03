
#include "mux_os_type.h"
#include "mux_os_api.h"

#define TIMERS_COUNT 50
#define TIMER_NAME_SIZE 16
#define OS_TIMER_THREAD_PRIO 70
#define MUX_TRANS_THREAD_STACK_SIZE 8192

BOOL g_TimerModuleInit = FALSE;

//Timer APIs

#ifdef OS_ENVIRONMENT_LINUX

#include "time.h"
#include "signal.h"
#include "errno.h"
#include "linux/errno.h"
#include <sys/time.h>

#define OS_LINUX_TIMER_INTERVAL 100

#define SIGTIMER (SIGALRM) 

typedef struct _TIMER_CONTAINER_STRUCT_
{
	unsigned int id;
	unsigned int interval;
	void *pParam;
	OS_FUNC_ENTRY Func;
	char name[TIMER_NAME_SIZE];
	BOOL running;
	OS_MUTEX_ID id_mutex;
	unsigned int count;
	BOOL used_state;
	
}TimerContainerStruct;

TimerContainerStruct g_aTimers[TIMERS_COUNT];

OS_THREAD_ID g_timer_thread_id = -1;

void os_timer_thread(void *param)
{
	unsigned int id;
	while(1)
	{
		for(id = 0; id < TIMERS_COUNT; id++)
		{
			os_mutex_get(g_aTimers[id].id_mutex);
			if(g_aTimers[id].used_state && g_aTimers[id].running)
			{
				if(g_aTimers[id].count == 0)
				{
					g_aTimers[id].running = FALSE;
					os_mutex_put(g_aTimers[id].id_mutex);
					(*(g_aTimers[id].Func))(g_aTimers[id].pParam);
					os_mutex_get(g_aTimers[id].id_mutex);
				}
				else
					g_aTimers[id].count--;
			}
			os_mutex_put(g_aTimers[id].id_mutex);
		}
		os_thread_sleep(OS_LINUX_TIMER_INTERVAL);
	}
}

void InitTimers()
{
	int i;
	
	if(g_TimerModuleInit)
		return;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		g_aTimers[i].id = i;
		g_aTimers[i].interval = 0;
		g_aTimers[i].pParam= NULL;
		g_aTimers[i].Func= NULL;
		g_aTimers[i].count = 0;
		g_aTimers[i].running = FALSE;
		g_aTimers[i].used_state = FALSE;

		g_aTimers[i].id_mutex = os_mutex_create("timer mutex");
		memset(g_aTimers[i].name,0,TIMER_NAME_SIZE);
	}
	g_timer_thread_id= os_thread_create("timer",os_timer_thread,OS_TIMER_THREAD_PRIO,MUX_TRANS_THREAD_STACK_SIZE,NULL);
	
	g_TimerModuleInit = TRUE;
}

int FindUnusedTimerIndex()
{
	int i;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		if(!g_aTimers[i].used_state)
			return i;
	}
	return -1;
}

OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag)
{
	timer_t   tid;  
	struct   sigevent   se;  
	int id_timer;
	int ret;

//	sigset_t sigset;
	struct sigaction sigact;
	
	if(!g_TimerModuleInit)
		InitTimers();
	id_timer = FindUnusedTimerIndex();
	if(id_timer == -1)
		return OS_FAILURE;
	printf("\n os_timer_create : timer id-%d; interval-%d\n",id_timer,interval);

	g_aTimers[id_timer].used_state = TRUE;
	g_aTimers[id_timer].running = FALSE;
	g_aTimers[id_timer].interval = (interval+OS_LINUX_TIMER_INTERVAL-1)/OS_LINUX_TIMER_INTERVAL;
	g_aTimers[id_timer].count = 0;
	g_aTimers[id_timer].Func = expir_func;
	g_aTimers[id_timer].pParam = expire_para_ptr;
	memcpy(g_aTimers[id_timer].name,name,strlen(name));

	printf("\nos_timer_create : timer_create success-%d\n",id_timer);
	
	return id_timer;
}

OS_STATUS os_timer_delete(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;

	g_aTimers[id].id = id;
	g_aTimers[id].interval = 0;
	g_aTimers[id].pParam= NULL;
	g_aTimers[id].Func= NULL;
	g_aTimers[id].count = 0;
	g_aTimers[id].running = FALSE;
	g_aTimers[id].used_state = FALSE;
		
	memset(g_aTimers[id].name,0,TIMER_NAME_SIZE);
	
	return 0;
}

OS_STATUS os_timer_start(OS_TIMER_ID id)
{
	struct   itimerspec   ts,   ots;
	unsigned int interval;
	int ret;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	

 	g_aTimers[id].running = TRUE;
	g_aTimers[id].count = g_aTimers[id].interval;
	
	printf("\n os_timer_start : %d success\n",id);
	return OS_SUCCESS;
}

OS_STATUS os_timer_stop(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	if(g_aTimers[id].used_state && g_aTimers[id].running)
	{
		g_aTimers[id].running = FALSE;
		g_aTimers[id].count = 0;
	}
	
	return OS_SUCCESS;
}

OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval)
{
	struct   itimerspec   ts,   ots;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	g_aTimers[id].interval = (interval+OS_LINUX_TIMER_INTERVAL-1)/OS_LINUX_TIMER_INTERVAL;
	
	g_aTimers[id].running = TRUE;
	g_aTimers[id].count = g_aTimers[id].interval;

	printf("\n os_timer_restart : %d success\n",id);
	return OS_SUCCESS;
}

#endif

#ifdef OS_ENVIRONMENT_LINUX_BAK2

#include "time.h"
#include "signal.h"
#include "errno.h"
#include "linux/errno.h"
#include <sys/time.h>

#define OS_LINUX_TIMER_INTERVAL 100

#define SIGTIMER (SIGALRM) 

typedef struct _TIMER_CONTAINER_STRUCT_
{
	unsigned int id;
	unsigned int interval;
	void *pParam;
	OS_FUNC_ENTRY Func;
	char name[TIMER_NAME_SIZE];
	BOOL running;
	unsigned int count;
	BOOL used_state;
	
}TimerContainerStruct;

TimerContainerStruct g_aTimers[TIMERS_COUNT];


void DeafultTimerHandle(int signo)
{
	unsigned int id;
	printf("DeafultTimerHandle\n");
	
	signal(SIGALRM, DeafultTimerHandle);

	for(id = 0; id < TIMERS_COUNT; id++)
	{
		if(g_aTimers[id].used_state && g_aTimers[id].running)
		{
			if(g_aTimers[id].count == 0)
			{
				printf("DeafultTimerHandle : timer %d expired\n",id);
				g_aTimers[id].running = FALSE;
				(g_aTimers[id].Func)(g_aTimers[id].pParam);
			}
			else
				g_aTimers[id].count--;
		}
	}
}

void start_os_timer()
{
	struct itimerval value;

	printf("start_os_timer\n");
	signal(SIGALRM, DeafultTimerHandle);

	value.it_value.tv_sec = OS_LINUX_TIMER_INTERVAL/1000;
    value.it_value.tv_usec = (OS_LINUX_TIMER_INTERVAL%1000)*1000000;
    value.it_interval.tv_sec = OS_LINUX_TIMER_INTERVAL/1000;
    value.it_interval.tv_usec = (OS_LINUX_TIMER_INTERVAL%1000)*1000000;
	
    setitimer(ITIMER_REAL, &value, NULL);
}

void InitTimers()
{
	int i;
	
	if(g_TimerModuleInit)
		return;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		g_aTimers[i].id = i;
		g_aTimers[i].interval = 0;
		g_aTimers[i].pParam= NULL;
		g_aTimers[i].Func= NULL;
		g_aTimers[i].count = 0;
		g_aTimers[i].running = FALSE;
		g_aTimers[i].used_state = FALSE;

		memset(g_aTimers[i].name,0,TIMER_NAME_SIZE);
	}
	start_os_timer();
	
	g_TimerModuleInit = TRUE;
}

int FindUnusedTimerIndex()
{
	int i;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		if(!g_aTimers[i].used_state)
			return i;
	}
	return -1;
}

OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag)
{
	timer_t   tid;  
	struct   sigevent   se;  
	int id_timer;
	int ret;

//	sigset_t sigset;
	struct sigaction sigact;
	
	if(!g_TimerModuleInit)
		InitTimers();
	id_timer = FindUnusedTimerIndex();
	if(id_timer == -1)
		return OS_FAILURE;
	printf("\n os_timer_create : timer id-%d; interval-%d\n",id_timer,interval);

	g_aTimers[id_timer].used_state = TRUE;
	g_aTimers[id_timer].running = FALSE;
	g_aTimers[id_timer].interval = (interval+OS_LINUX_TIMER_INTERVAL-1)/OS_LINUX_TIMER_INTERVAL;
	g_aTimers[id_timer].count = 0;
	g_aTimers[id_timer].Func = expir_func;
	g_aTimers[id_timer].pParam = expire_para_ptr;
	memcpy(g_aTimers[id_timer].name,name,strlen(name));

	printf("\nos_timer_create : timer_create success-%d\n",id_timer);
	
	return id_timer;
}

OS_STATUS os_timer_delete(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;

	g_aTimers[id].id = id;
	g_aTimers[id].interval = 0;
	g_aTimers[id].pParam= NULL;
	g_aTimers[id].Func= NULL;
	g_aTimers[id].count = 0;
	g_aTimers[id].running = FALSE;
	g_aTimers[id].used_state = FALSE;
		
	memset(g_aTimers[id].name,0,TIMER_NAME_SIZE);
	
	return 0;
}

OS_STATUS os_timer_start(OS_TIMER_ID id)
{
	struct   itimerspec   ts,   ots;
	unsigned int interval;
	int ret;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	

 	g_aTimers[id].running = TRUE;
	g_aTimers[id].count = g_aTimers[id].interval;
	
	printf("\n os_timer_start : %d success\n",id);
	return OS_SUCCESS;
}

OS_STATUS os_timer_stop(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	if(g_aTimers[id].used_state && g_aTimers[id].running)
	{
		g_aTimers[id].running = FALSE;
		g_aTimers[id].count = 0;
	}
	
	return OS_SUCCESS;
}

OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval)
{
	struct   itimerspec   ts,   ots;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	g_aTimers[id].interval = (interval+OS_LINUX_TIMER_INTERVAL-1)/OS_LINUX_TIMER_INTERVAL;
	
	g_aTimers[id].running = TRUE;
	g_aTimers[id].count = g_aTimers[id].interval;

	printf("\n os_timer_restart : %d success\n",id);
	return OS_SUCCESS;
}

#endif

#ifdef OS_ENVIRONMENT_LINUX_BAK

#include "time.h"
#include "signal.h"
#include "errno.h"
#include "linux/errno.h"


#define SIGTIMER (SIGALRM) 

typedef struct _TIMER_CONTAINER_STRUCT_
{
	unsigned int id;
	timer_t id_timer;
	unsigned int interval;
	struct sigevent st_signal;
	void *pParam;
	OS_FUNC_ENTRY Func;
	char name[TIMER_NAME_SIZE];
	BOOL running;
	BOOL used_state;
	
}TimerContainerStruct;

TimerContainerStruct g_aTimers[TIMERS_COUNT];

/*
siginfo_t {
    int      si_signo;  // Signal number /
    int      si_errno;  // An errno value /
    int      si_code;   // Signal code /
    pid_t    si_pid;    // Sending process ID /
    uid_t    si_uid;    // Real user ID of sending process /
    int      si_status; // Exit value or signal /
    clock_t  si_utime;  // User time consumed /
    clock_t  si_stime;  // System time consumed /
    sigval_t si_value;  // Signal value /
    int      si_int;    // POSIX.1b signal /
    void *   si_ptr;    // POSIX.1b signal /
    void *   si_addr;   // Memory location which caused fault /
    int      si_band;   // Band event /
    int      si_fd;     // File descriptor /
}
*/
/*
void DeafultTimerHandle(sigval_t sig_v)
{
	unsigned int id;
//	id = *(unsigned int *)(info->si_value.sival_ptr);
	id = sig_v.sival_int;
	
	printf("\nDeafultTimerHandle : timer id-%d\n",id);
	if(id>=TIMERS_COUNT)
		return;

	printf("\nDeafultTimerHandle : execute timer id-%d\n",id);
	if(g_aTimers[id].used_state)
		(g_aTimers[id].Func)(g_aTimers[id].pParam);
}
*/

void DeafultTimerHandle(int signo, siginfo_t* info, void* context)
{
	unsigned int id;
//	id = *(unsigned int *)(info->si_value.sival_ptr);
	id = (unsigned int)(info->si_value.sival_int);
	
	printf("\nDeafultTimerHandle : timer id-%d\n",id);
	if(id>=TIMERS_COUNT)
		return;

	printf("\nDeafultTimerHandle : execute timer id-%d\n",id);
	if(g_aTimers[id].used_state)
		(g_aTimers[id].Func)(g_aTimers[id].pParam);
}


void InitTimers()
{
	int i;
	


	if(g_TimerModuleInit)
		return;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		g_aTimers[i].id = i;
		g_aTimers[i].id_timer = 0;
		g_aTimers[i].interval = 0;
		g_aTimers[i].pParam= NULL;
		g_aTimers[i].Func= NULL;
		g_aTimers[i].st_signal.sigev_notify = 0;
		g_aTimers[i].st_signal.sigev_notify_function = NULL;
		g_aTimers[i].st_signal.sigev_value.sival_int = -1;
		g_aTimers[i].running = FALSE;
		g_aTimers[i].used_state = FALSE;

		memset(g_aTimers[i].name,0,TIMER_NAME_SIZE);
	}

	g_TimerModuleInit = TRUE;
}

int FindUnusedTimerIndex()
{
	int i;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		if(!g_aTimers[i].used_state)
			return i;
	}
	return -1;
}

OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag)
{
	timer_t   tid;  
	struct   sigevent   se;  
	int id_timer;
	int ret;

//	sigset_t sigset;
	struct sigaction sigact;
	
	if(!g_TimerModuleInit)
		InitTimers();
	id_timer = FindUnusedTimerIndex();
	if(id_timer == -1)
		return OS_FAILURE;
/*
typedef struct sigevent {

	sigval_t sigev_value;
	int sigev_signo;
	int sigev_notify;
	union {
		int _pad[SIGEV_PAD_SIZE];
		 int _tid;

		struct {
			void (*_function)(sigval_t);
			void *_attribute;	// really pthread_attr_t
		} _sigev_thread;
	} _sigev_un;

} sigevent_t;
*/
	printf("\n os_timer_create : timer id-%d; interval-%d\n",id_timer,interval);


	sigfillset (&sigact.sa_mask);
	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = DeafultTimerHandle;
	
	sigaction(SIGTIMER, &sigact, NULL);

	memset(&(g_aTimers[id_timer].st_signal),0,sizeof(g_aTimers[id_timer].st_signal));  
	g_aTimers[id_timer].st_signal.sigev_notify = SIGEV_SIGNAL;//SIGEV_THREAD;
//	g_aTimers[id_timer].st_signal.sigev_signo = SIGTIMER;
//	g_aTimers[id_timer].st_signal.sigev_notify_function = DeafultTimerHandle;  
//	g_aTimers[id_timer].st_signal.sigev_value.sival_ptr = &(g_aTimers[id_timer].id);  
	g_aTimers[id_timer].st_signal.sigev_value.sival_int = g_aTimers[id_timer].id;  

	printf("\nos_timer_create : timer %d pre id_timer-%d\n",id_timer,g_aTimers[id_timer].id_timer);
	ret = timer_create(CLOCK_REALTIME,&(g_aTimers[id_timer].st_signal),&(g_aTimers[id_timer].id_timer));
	if(ret !=  0)  
	{  
		printf("\nos_timer_create : timer_create failed; error no-%d\n",ret);
		return OS_FAILURE;  
	}  
	printf("\nos_timer_create : timer %d after id_timer-%d\n",id_timer,g_aTimers[id_timer].id_timer);
	printf("\nnos_timer_create : errno=%d, %s\n", ret, strerror(ret));
	
	g_aTimers[id_timer].used_state = TRUE;
	g_aTimers[id_timer].interval = interval;
	g_aTimers[id_timer].Func = expir_func;
	g_aTimers[id_timer].pParam = expire_para_ptr;
	memcpy(g_aTimers[id_timer].name,name,strlen(name));

	printf("\nos_timer_create : timer_create success-%d\n",id_timer);
	
	return id_timer;
}

OS_STATUS os_timer_delete(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;

	if( 0 != timer_delete(g_aTimers[id].id_timer))
		return OS_FAILURE;
	
	g_aTimers[id].id_timer = 0;
	g_aTimers[id].interval = 0;
	g_aTimers[id].pParam= NULL;
	g_aTimers[id].Func= NULL;
	g_aTimers[id].st_signal.sigev_notify = 0;
	g_aTimers[id].st_signal.sigev_notify_function = NULL;
	g_aTimers[id].st_signal.sigev_value.sival_int = -1;
	g_aTimers[id].running = FALSE;
	g_aTimers[id].used_state = FALSE;

	memset(g_aTimers[id].name,0,TIMER_NAME_SIZE);
	
	return 0;
}

OS_STATUS os_timer_start(OS_TIMER_ID id)
{
	struct   itimerspec   ts,   ots;
	unsigned int interval;
	int ret;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	interval = g_aTimers[id].interval;
	
	ts.it_value.tv_sec   =   interval/1000;  
	ts.it_value.tv_nsec   =   (interval%1000)*1000000;  
	ts.it_interval.tv_sec   =  interval/1000;  
	ts.it_interval.tv_nsec   =  (interval%1000)*1000000;  

	printf("\n os_timer_start : timer id-%d; linux timer id-%d\n",id,g_aTimers[id].id_timer);
	printf("\n os_timer_start : timer id-%d; interval-%d\n",id,g_aTimers[id].interval);
//	if(timer_settime(g_aTimers[id].id_timer,TIMER_ABSTIME,&ts,&ots)   <   0)  
	ret = timer_settime(g_aTimers[id].id_timer,TIMER_ABSTIME,&ts,NULL);
	if(ret != 0)  
	{  
		ret = errno;
		printf("\nerrno=%d, %s\n", ret, strerror(ret));
		
		if(ret == EINVAL)
			printf("\n os_timer_start : timer %d failed ,invalid linux timer id\n",id);
		else if(ret == EFAULT)
			printf("\n os_timer_start : timer %d failed ,invalid 3rd parameter\n",id);
		else
			printf("\n os_timer_start : timer %d failed ,unknown error-%d\n",id,ret);
			
		return OS_FAILURE;  
	}  
	printf("\n os_timer_start : %d success\n",id);
	return OS_SUCCESS;
}

OS_STATUS os_timer_stop(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	if(g_aTimers[id].used_state && g_aTimers[id].running)
		g_aTimers[id].running = FALSE;
	
	return OS_SUCCESS;
}

OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval)
{
	struct   itimerspec   ts,   ots;

	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	g_aTimers[id].interval = interval;
	
	ts.it_value.tv_sec   =   interval/1000;  
	ts.it_value.tv_nsec   =   (interval%1000)*1000000;  
	ts.it_interval.tv_sec   =   interval/1000;  
	ts.it_interval.tv_nsec   =   (interval%1000)*1000000;  
	printf("\n os_timer_restart : timer id-%d; interval-%d\n",id,g_aTimers[id].interval);
	if(timer_settime(g_aTimers[id].id_timer,0/*TIMER_ABSTIME*/,&ts,&ots)   <   0)  
	{  
		printf("\n os_timer_restart : %d failed\n",id);
		return OS_FAILURE;  
	}  
	printf("\n os_timer_restart : %d success\n",id);
	return OS_SUCCESS;
}

#endif

#ifdef OS_ENVIRONMENT_WIN32

#define _WIN32_WINNT 0x0500

#include "mmsystem.h"

#define OS_WIN32_TIMER_INTERVAL 100

typedef struct _TIMER_CONTAINER_STRUCT_
{
	unsigned int id;
	CRITICAL_SECTION cs;
	OS_FUNC_ENTRY timer_entry;
	void *timer_param;
	unsigned int interval;
	char name[TIMER_NAME_SIZE];
	BOOL used_state;
	BOOL running;
	unsigned int count;
	
}TimerContainerStruct;

TimerContainerStruct g_aTimers[TIMERS_COUNT];
OS_THREAD_ID g_timer_thread_id = -1;

void os_timer_thread(void *param)
{
	unsigned int id;
	while(1)
	{
		for(id = 0; id < TIMERS_COUNT; id++)
		{
			EnterCriticalSection(&(g_aTimers[id].cs));
			if(g_aTimers[id].used_state && g_aTimers[id].running)
			{
				if(g_aTimers[id].count == 0)
				{
					g_aTimers[id].running = FALSE;
					LeaveCriticalSection(&(g_aTimers[id].cs));
					(*(g_aTimers[id].timer_entry))(g_aTimers[id].timer_param);
					EnterCriticalSection(&(g_aTimers[id].cs));
				}
				else
					g_aTimers[id].count--;
			}
			LeaveCriticalSection(&(g_aTimers[id].cs));
		}
		os_thread_sleep(OS_WIN32_TIMER_INTERVAL);
	}
}

void InitTimers()
{
	int i;
	
	if(g_TimerModuleInit)
		return;

	for(i=0;i<TIMERS_COUNT;i++)
	{
		g_aTimers[i].id = i;
		g_aTimers[i].timer_entry = NULL;
		g_aTimers[i].timer_param = NULL;
		g_aTimers[i].interval = 0;
		g_aTimers[i].used_state = FALSE;
		g_aTimers[i].running = FALSE;
		g_aTimers[i].count = 0;

		InitializeCriticalSection(&(g_aTimers[i].cs));
		memset(g_aTimers[i].name,0,TIMER_NAME_SIZE);
	}

	g_timer_thread_id= os_thread_create("timer",os_timer_thread,OS_TIMER_THREAD_PRIO,MUX_TRANS_THREAD_STACK_SIZE,NULL);
}

int FindUnusedTimerIndex()
{
	int i;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		if(!g_aTimers[i].used_state)
			return i;
	}
	return -1;
}

OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag)
{
	int id_timer;
	
	if(!g_TimerModuleInit)
		InitTimers();
	id_timer = FindUnusedTimerIndex();
	if(id_timer == -1)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aTimers[id_timer].cs));
	g_aTimers[id_timer].id = id_timer;
	g_aTimers[id_timer].timer_entry = expir_func;
	g_aTimers[id_timer].timer_param = expire_para_ptr;
	g_aTimers[id_timer].interval = (interval+99)/100;
	g_aTimers[id_timer].used_state = TRUE;
	memcpy(g_aTimers[id_timer].name,name,strlen(name));
	LeaveCriticalSection(&(g_aTimers[id_timer].cs));
	
	return id_timer;
}

OS_STATUS os_timer_delete(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aTimers[id].cs));
	if(!(g_aTimers[id].used_state))
	{
		LeaveCriticalSection(&(g_aTimers[id].cs));
		return OS_FAILURE;
	}
 
	g_aTimers[id].timer_entry = NULL;
	g_aTimers[id].timer_param = NULL;
	g_aTimers[id].interval = 0;
	g_aTimers[id].used_state = FALSE;
	g_aTimers[id].running = FALSE;
	g_aTimers[id].count = 0;

	memset(g_aTimers[id].name,0,TIMER_NAME_SIZE);
	LeaveCriticalSection(&(g_aTimers[id].cs));
	
	return 0;
}

OS_STATUS os_timer_start(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aTimers[id].cs));
	if(!(g_aTimers[id].used_state))
	{
		LeaveCriticalSection(&(g_aTimers[id].cs));
		return OS_FAILURE;
	}

	if(g_aTimers[id].running == FALSE)
	{
		g_aTimers[id].running = TRUE;
		g_aTimers[id].count = g_aTimers[id].interval;
		LeaveCriticalSection(&(g_aTimers[id].cs));
		return OS_SUCCESS;
	}
	LeaveCriticalSection(&(g_aTimers[id].cs));
		
	return OS_FAILURE;
}

OS_STATUS os_timer_stop(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aTimers[id].cs));
	if(!(g_aTimers[id].used_state))
	{
		LeaveCriticalSection(&(g_aTimers[id].cs));
		return OS_FAILURE;
	}
	
	if(g_aTimers[id].running == TRUE)
		g_aTimers[id].running = FALSE;
	LeaveCriticalSection(&(g_aTimers[id].cs));
	
	return OS_SUCCESS;
}

OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	EnterCriticalSection(&(g_aTimers[id].cs));
	if(!(g_aTimers[id].used_state))
	{
		LeaveCriticalSection(&(g_aTimers[id].cs));
		return OS_FAILURE;
	}
	
	g_aTimers[id].interval = (interval+99)/100;
	
	g_aTimers[id].running = TRUE;
	g_aTimers[id].count = g_aTimers[id].interval;
	LeaveCriticalSection(&(g_aTimers[id].cs));
	
	return OS_SUCCESS;
}

#endif

#ifdef OS_ENVIRONMENT_WIN32_BAK

#define _WIN32_WINNT 0x0500

#include "mmsystem.h"

typedef struct _TIMER_CONTAINER_STRUCT_
{
	unsigned int id;
	MMRESULT id_timer;
	OS_FUNC_ENTRY timer_entry;
	void *timer_param;
	unsigned int interval;
	char name[TIMER_NAME_SIZE];
	BOOL used_state;
	
}TimerContainerStruct;

TimerContainerStruct g_aTimers[TIMERS_COUNT];

void InitTimers()
{
	int i;
	
	if(g_TimerModuleInit)
		return;

	for(i=0;i<TIMERS_COUNT;i++)
	{
		g_aTimers[i].id = i;
		g_aTimers[i].id_timer = NULL;
		g_aTimers[i].timer_entry = NULL;
		g_aTimers[i].timer_param = NULL;
		g_aTimers[i].interval = 0;
		g_aTimers[i].used_state = FALSE;

		memset(g_aTimers[i].name,0,TIMER_NAME_SIZE);
	}
}

int FindUnusedTimerIndex()
{
	int i;
	
	for(i=0;i<TIMERS_COUNT;i++)
	{
		if(!g_aTimers[i].used_state)
			return i;
	}
	return -1;
}

void os_timer_callback(UINT timer_id, UINT uMsg, DWORD dwUser,DWORD dw1,DWORD dw2)
{
	unsigned int *id_ptr;
	id_ptr = (unsigned int *)dwUser;
	printf("** os_timer_callback : id-%d\n",*id_ptr);
	if(g_aTimers[*id_ptr].timer_entry != NULL)
		(*(g_aTimers[*id_ptr].timer_entry))(g_aTimers[*id_ptr].timer_param);
}

OS_TIMER_ID os_timer_create(char *name,OS_FUNC_ENTRY expir_func, void* expire_para_ptr, unsigned int interval,unsigned int flag)
{
	int id_timer;
	
	if(!g_TimerModuleInit)
		InitTimers();
	id_timer = FindUnusedTimerIndex();
	if(id_timer == -1)
		return OS_FAILURE;

	g_aTimers[id_timer].timer_entry = expir_func;
	g_aTimers[id_timer].timer_param = expire_para_ptr;
	g_aTimers[id_timer].interval = interval;
	g_aTimers[id_timer].used_state = TRUE;
	memcpy(g_aTimers[id_timer].name,name,strlen(name));
		
	return id_timer;
}

OS_STATUS os_timer_delete(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
 
	if(g_aTimers[id].id_timer != NULL)
		timeKillEvent(g_aTimers[id].id_timer);

	g_aTimers[id].id_timer = NULL;
	g_aTimers[id].timer_entry = NULL;
	g_aTimers[id].timer_param = NULL;
	g_aTimers[id].interval = 0;
	g_aTimers[id].used_state = FALSE;

	memset(g_aTimers[id].name,0,TIMER_NAME_SIZE);
	
	return 0;
}

OS_STATUS os_timer_start(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;

//	if(g_aTimers[id].id_timer != NULL)
//		timeKillEvent(g_aTimers[id].id_timer);
		
	g_aTimers[id].id_timer = timeSetEvent(g_aTimers[id].interval,
										1,
										(LPTIMECALLBACK)os_timer_callback,
										&(g_aTimers[id].id),
										TIME_ONESHOT);//|TIME_CALLBACK_FUNCTION);
		
	return OS_SUCCESS;
}

OS_STATUS os_timer_stop(OS_TIMER_ID id)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	if(g_aTimers[id].id_timer != NULL)
		timeKillEvent(g_aTimers[id].id_timer);

	g_aTimers[id].id_timer = NULL;
	
	return OS_SUCCESS;
}

OS_STATUS os_timer_restart(OS_TIMER_ID id,unsigned int interval)
{
	if(id>=TIMERS_COUNT)
		return OS_FAILURE;

	if(!(g_aTimers[id].used_state))
		return OS_FAILURE;
	
	g_aTimers[id].interval = interval;
	
//	if(g_aTimers[id].id_timer != NULL)
//		timeKillEvent(g_aTimers[id].id_timer);
	
	g_aTimers[id].id_timer = timeSetEvent(g_aTimers[id].interval,
										1,
										(LPTIMECALLBACK)os_timer_callback,
										&(g_aTimers[id].id),
										TIME_ONESHOT);//|TIME_CALLBACK_FUNCTION);
	
	return OS_SUCCESS;
}

#endif

