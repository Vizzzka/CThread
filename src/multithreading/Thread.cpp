#include "multithreading/Thread.hpp"
#ifdef USE_BEGIN_THREAD
#include <process.h>
#endif


#include <iostream>

using namespace std;

/**
 *
 * _THKERNEL
 * thread callback function used by CreateThread
 *
 *
 **/
#ifdef WINDOWS
#ifdef USE_BEGIN_THREAD
unsigned __stdcall
#else
DWORD WINAPI
#endif
#else
LPVOID
#endif
_THKERNEL( LPVOID lpvData /* CThread Object */)
{
	CThread *pThread = (CThread *)lpvData;

	pThread->m_mutex.Lock();
#ifndef WINDOWS
		pThread->m_dwId = CThread::ThreadId();
#endif
	pThread->m_mutex.Unlock();


	((IRunnable*)pThread->_threadObj)->run();

	pThread->m_mutex.Lock();
		pThread->m_state = ThreadStateDown;
		pThread->m_bRunning = FALSE;
	pThread->m_mutex.Unlock();

#ifdef WINDOWS
	return 0;
#else
	return (LPVOID)0;
#endif
}

/**
 *
 * FromSameThread
 * determines if the calling thread is the same
 * as the thread assoiated with the object
 *
 **/
BOOL
CThread::FromSameThread()
{
	ThreadId_t id = ThreadId();
	if( ThreadIdsEqual(&id,&m_dwId) ) return TRUE;
	return FALSE;
}

/**
 *
 * SetPriority
 * sets a threads run priority, see SetThreadPriority
 * Note: only works for Windows family of operating systems
 *
 *
 **/
void CThread::SetPriority(DWORD dwPriority)
{

#ifdef WINDOWS
	SetThreadPriority(m_thread,dwPriority);
#endif
}

BOOL CThread::isAlive() {
	return m_bRunning;
}

DWORD CThread::GetPriority() {
#ifdef WINDOWS
	return GetThreadPriority(m_thread);
#endif
	return 0;
}

	  
/**
 * CThread
 * instanciates thread object and
 * starts thread.
 *
 **/
CThread::CThread(IRunnable *ptr)
:m_bRunning(FALSE)
#ifdef WINDOWS
,m_thread(NULL)
#endif
,m_dwId(0L)
,m_state(ThreadStateDown)
,m_dwIdle(100)
,m_stackSize(DEFAULT_STACK_SIZE)
,m_StopTimeout(30)
{

	if( !m_mutex.m_bCreated )
	{
		perror("mutex creation failed");
		m_state = ThreadStateFault;
		return;
	}


	if( !m_event.m_bCreated )
	{
		perror("event creation failed");
		m_state = ThreadStateFault;
		return;
	}

	_threadObj = ptr;

}

/**
 *
 * Start
 * start thread
 *
 **/
BOOL
CThread::Start()
{
	try 
	{
		if( FromSameThread() )
		{
			throw "\n\tit is illegal for a thread to attempt to start itself!\n";
		}

		m_mutex.Lock();
		if( m_bRunning ) 
		{
			m_mutex.Unlock();
			return TRUE;
		}

		m_mutex.Unlock();


#ifdef WINDOWS
		if( m_thread ) CloseHandle(m_thread);

		m_mutex.Lock();
		m_state = ThreadStateWaiting;
		m_bRunning = TRUE;
		m_mutex.Unlock();
#ifdef USE_BEGIN_THREAD
		m_thread = (HANDLE )_beginthreadex(NULL,(unsigned int)m_stackSize,_THKERNEL,(LPVOID)this,0,&m_dwId);
#else
		m_thread = CreateThread(NULL,m_stackSize ,_THKERNEL,(LPVOID)this,0,&m_dwId);
#endif
		if( !m_thread )
		{
			perror("thread creation failed");
			m_bRunning = FALSE;
			m_state = ThreadStateFault;
			return FALSE;
		}
#else
		pthread_attr_t attr;

		pthread_attr_init(&attr);

#ifdef VMS
		if( m_stackSize == 0 )
			pthread_attr_setstacksize(&attr,PTHREAD_STACK_MIN*10);
#endif
		if( m_stackSize != 0 )
			pthread_attr_setstacksize(&attr,m_stackSize);

		m_mutex.Lock();
		m_state = ThreadStateWaiting;
		m_bRunning = TRUE;
		m_mutex.Unlock();

		int error = pthread_create(&m_thread,&attr,_THKERNEL,(LPVOID)this);

		if( error != 0 )
		{
			m_bRunning = FALSE;
			m_state = ThreadStateFault;

#if defined(HPUX) || defined(SUNOS) || defined(LINUX)
			switch(error)/* show the thread error */
			{
			case EINVAL:
				cerr << "error: attr in an invalid thread attributes object\n";
				break;
			case EAGAIN:
				cerr << "error: the necessary resources to create a thread are not\n";
				cerr << "available.\n";
				break;
			case EPERM:
				cerr << "error: the caller does not have the privileges to create\n";
				cerr << "the thread with the specified attr object.\n";
				break;
#if defined(HPUX)
			case ENOSYS:

				cerr << "error: pthread_create not implemented!\n";
				if( __is_threadlib_linked()==0 )
				{
					cerr << "error: threaded library not being used, improper linkage \"-lpthread -lc\"!\n";
				}
				break;
#endif
			default:
				cerr << "error: an unknown error was encountered attempting to create\n";
				cerr << "the requested thread.\n";
				break;
			}
#else
			cerr << "error: could not create thread, pthread_create failed (" << error << ")!\n";
#endif
			return FALSE;	
		}
#endif
	}
	catch (char *psz)
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CThread::Start",MB_ICONHAND);
#else
		cerr << "Fatal exception CThread::Start():" << psz;
#endif
		exit(-1);
	}
	return TRUE;
}


/**
 *
 * ThreadState
 * return the current state of the thread
 *
 **/
ThreadState_t 
CThread::ThreadState()
{
	ThreadState_t currentState;
	m_mutex.Lock();
		currentState = m_state;
	m_mutex.Unlock();
	return currentState;
}

/**
 *
 * ~CThread
 * destructor.  Stop should be called prior to destruction to
 * allow for gracefull thread termination.
 *
 **/
CThread::~CThread()
{
#ifdef WINDOWS
	CloseHandle(m_thread);
#endif
}


/**
 *
 * PingThread
 * used to determine if a thread is running
 *
 **/
BOOL CThread::PingThread(DWORD dwTimeout /* timeout in milli-seconds */
				 )
{
    DWORD dwTotal = 0;

	while(TRUE)
	{
		if( dwTotal > dwTimeout && dwTimeout > 0 )
			return FALSE;
		m_mutex.Lock();
			if( m_bRunning )
			{
				m_mutex.Unlock();
				return TRUE;
			}
		dwTotal += m_dwIdle;
		m_mutex.Unlock();
		Sleep(m_dwIdle);
	}

	return FALSE;
}

/**
 *
 * WaitTillExit
 * blocks caller until thread exits
 *
 **/
void CThread::Join()
{
	try 
	{
		if( FromSameThread() )
			throw "\n\tthis function can not be called from within the same thread!\n";

		m_mutex.Lock();
		if( !m_bRunning ) {
			m_mutex.Unlock();
			return;
		}
		m_mutex.Unlock();

#ifdef WINDOWS
		WaitForSingleObject(m_thread,INFINITE);
#else
		LPVOID lpv;

		pthread_join(m_thread,&lpv);
#endif
	}
	catch( char *psz )
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CThread::Join",MB_ICONHAND);
		exit(-1);
#else
		cerr << "Fatal exception CThread::Join: " << psz;
#endif

	}
}




