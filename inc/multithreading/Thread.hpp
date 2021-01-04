/**
 *  @file   Thread.hpp
 *  @brief  Thread Class
 *  @author Victoria Usachova
 *  @date   2020-01-04
 ***********************************************/

#ifndef THREAD_CLASS
#define THREAD_CLASS

#ifndef WINDOWS
#if defined(WIN32) || defined(WIN64)
#define WINDOWS
#endif
#endif

#ifndef WINDOWS
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
typedef unsigned char BOOL;
#define TRUE 1
#define FALSE 0
typedef long DWORD;
typedef void *LPVOID;
#else
#define _AFXDLL
#include "afx.h"
#include <windows.h>
#include <stdio.h>
#endif

#if defined(AS400) || defined(OS400)
typedef pthread_id_np_t ThreadId_t;
#elif defined(VMS) 
typedef pthread_t ThreadId_t;
#else
#ifdef USE_BEGIN_THREAD
typedef unsigned ThreadId_t;
#else
typedef DWORD ThreadId_t;
#endif
#endif

#include "MutexClass.hpp"
#include "EventClass.hpp"

#define DEFAULT_STACK_SIZE 0
#ifndef WINDOWS
void Sleep( unsigned int mseconds);
#endif

#ifndef ASSERT
#if defined(DEBUG) || defined(_DEBUG)
#define ASSERT(test) if( !(test) ) { \
	fprintf(stderr,"\nASSERT(%s) FAILS, %s line %d\n",#test,__FILE__, __LINE__); exit(0);}
#else
#define ASSERT(test)
#endif
#endif

typedef enum {
	ThreadStateBusy,               // thread is currently handling a task
	ThreadStateWaiting,            // thread is waiting for something to do
    ThreadStateDown,               // thread is not running
    ThreadStateShuttingDown,       // thread is in the process of shutting down
    ThreadStateFault               // an error has occured and the thread could not
	                               // be launched
} ThreadState_t;

/// @brief Interface for creating thread and manipulating it.
/// @details Interface must be implemented to create runnable object.
/// @author Vizzzka
struct IRunnable
{
	/// @brief thread function to run
	virtual void run() = 0;
};

/// @brief Interface for creating thread and manipulating it.
/// @details Interface must be implemented to create callable object.
/// @author Vizzzka
template <class T> struct ICallable {
	/// @brief thread function to call
	virtual T call() = 0;
};

/// @brief Class for creating thread and manipulating it.
/// @author Vizzzka
class CThread 
#ifdef WINDOWS
	: public CObject // use CObject as a base class so object can be used in lists and object arrays
#endif
{
private:
	IRunnable *_threadObj;          // Win32 and POSIX compatible thread parameter and procedure
	CEventClass   m_event;         // event controller
	CMutexClass	  m_mutex;         // mutex that protects threads internal data
	int           m_StopTimeout;   // specifies a timeout value for stop
	                               // if a thread fails to stop within m_StopTimeout
	                               // seconds an exception is thrown
	BOOL		  m_bRunning;      // set to TRUE if thread is running
#ifdef WINDOWS
	HANDLE		  m_thread;		   // thread handle
#else
	pthread_t     m_thread;        // thread handle
#endif
	ThreadId_t	  m_dwId;          // id of this thread
	ThreadState_t m_state;         // current state of thread see thread state data
	                               // structure.
	DWORD         m_dwIdle;        // used for Sleep periods
	DWORD		  m_stackSize;     // thread stack size
#define NO_ERRORS			       0
#define MUTEX_CREATION		       0x01
#define EVENT_CREATION		       0x02
#define THREAD_CREATION		       0x04
#define UNKNOWN					   0x08
#define MEMORY_FAULT               0x20
#define EVENT_AND_TYPE_DONT_MATCH  0x40
#define STACK_OVERFLOW             0x80
#define STACK_EMPTY                0x100
#define STACK_FULL                 0x200

public:
	/// @brief Construct create thread from given Runnable object.
	///
	/// @pre ptr object must implement IRunnable interface.
	///
	/// @param ptr pointer to object from which thread would be created
	///
	CThread(IRunnable *ptr);

	/// Class destructor
	~CThread(void);

	/// Copy constructor
	CThread(CThread const&) = delete;

	/// Copy-assignment operator
	CThread& operator=(CThread const&) = delete;

#ifdef WINDOWS
#ifdef USE_BEGIN_THREAD
	friend unsigned __stdcall _THKERNEL(LPVOID lpvData);
#else
	friend DWORD WINAPI _THKERNEL( LPVOID lpvData );
#endif
#else
	friend LPVOID _THKERNEL(LPVOID lpvData);
#endif

	/// @brief waits until thread ends its execution
	/// @details if thread is not running do nothing
	void        Join();

	/// @brief start the execution of the thread
	/// @details Could not be started from itself, throws exception in this case.
	/// If thread is already started do nothing and returns TRUE.
	/// Close thread handle if needed. Change thread status to ThreadStateWaiting before
	/// starting and to ThreadStateFault if thread was not created sucsessfully
	/// @return BOOl. Either thread was sucsessfully created or not
	BOOL		Start();

	/// @brief Get unique id of the thread
	/// @param pId pointer to where thread id would be stored
	void		GetId(ThreadId_t *pId) { memcpy(pId,&m_dwId,sizeof(ThreadId_t)); }

	/// @brief Get current state of the tread
	/// @details 	ThreadStateWaiting, ThreadStateDown, ThreadStateShuttingDown, ThreadStateFault
	/// @return ThreadState_t
	ThreadState_t ThreadState();

	/// @brief ping the thread
	BOOL		PingThread(DWORD dwTimeout=0);

	/// @brief Check is thread running or not
	/// @return BOOL
	BOOL        isAlive();

	/// @brief Get priority of thread execution
	/// @return int
	DWORD         GetPriority();

#ifdef WINDOWS
	/// @brief Set priority of thread execution
	/// @param dwPriority Priority of the thread to set.
	/// Default value is THREAD_PRIORITY_NORMAL
	void		SetPriority(DWORD dwPriority=THREAD_PRIORITY_NORMAL);
#else
	/// @brief Set priority of thread execution
	/// @param dwPriority Priority of the thread to set.
	/// Default value is THREAD_PRIORITY_NORMAL
	void		SetPriority(DWORD dwPriority=0);
#endif

	/// @brief Get id of the current thread
	/// @return ThreadId_t
	static ThreadId_t ThreadId()
	{
		ThreadId_t thisThreadsId ;
#if defined(AS400) || defined(OS400)
		pthread_t thread;
#endif

#ifdef WINDOWS
		thisThreadsId = (ThreadId_t)GetCurrentThreadId();
#else

#if defined(AS400) || defined(OS400)
		thread = pthread_self();
		pthread_getunique_np(&thread,&thisThreadsId);
#elif defined(ALPHA) || defined(DEC) || defined(VMS)
#ifdef VMS
		thisThreadsId = pthread_self();
#else
		thisThreadsId = pthread_getsequence_np(pthread_self());
#endif
#else
		thisThreadsId = pthread_self();
#endif
#endif
		return thisThreadsId;
	}

	static BOOL ThreadIdsEqual(ThreadId_t *p1,
	                           ThreadId_t *p2)
	{
#if defined(AS400)||defined(OS400)
		return(( memcmp(p1,p2,sizeof(ThreadId_t))==0)?TRUE:FALSE);
#elif defined(VMS)
		return (( pthread_equal(*p1,*p2) )?TRUE:FALSE );
#else
		return ((*p1 == *p2)?TRUE:FALSE);
#endif

	}

private:
	BOOL        FromSameThread();
};
#endif

