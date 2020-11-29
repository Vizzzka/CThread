//
// EventClass.cpp: implementation file
//
// Copyright (C) Walter E. Capers.  All rights reserved
//
// This source is free to use as you like.  If you make
// any changes please keep me in the loop.  Email them to
// walt.capers@comcast.net.
//
// PURPOSE:
//
//  To implement event signals as a C++ object
//
// REVISIONS
// =======================================================
// Date: 10.25.07        
// Name: Walter E. Capers
// Description: File creation
//
// Date: 11/02/07
// Name: Walter E. Capers
// Description: removed unnessary code identified by On Freund from Code Project
//
//
#include "multithreading/Thread.hpp"


#include <iostream>

using namespace std;

CEventClass::CEventClass(void)
:m_bCreated(TRUE)
{
	memset(&m_owner,0,sizeof(ThreadId_t));
#ifdef WINDOWS
	m_event = CreateEvent(NULL,FALSE,FALSE,NULL);
	if( !m_event )
	{
		m_bCreated = FALSE;
	}
#else
	pthread_mutexattr_t mattr;
	
	pthread_mutexattr_init(&mattr);
	pthread_mutex_init(&m_lock,&mattr);
	pthread_cond_init(&m_ready,NULL);

#endif	
}

CEventClass::~CEventClass(void)
{
#ifdef WINDOWS
	CloseHandle(m_event);
#else
	pthread_cond_destroy(&m_ready);
	pthread_mutex_destroy(&m_lock);
#endif
}


/**
 *
 * Set
 * set an event to signaled
 *
 **/
void
CEventClass::Set()
{
#ifdef WINDOWS
	SetEvent(m_event);
#else
	pthread_cond_signal(&m_ready);
#endif
}

/**
 *
 * Wait
 * wait for an event -- wait for an event object
 * to be set to signaled.  must be paired with a
 * call to reset within the same thread.
 *
 **/
BOOL
CEventClass::Wait()
{

	try
	{
		ThreadId_t id = CThread::ThreadId();
		if( CThread::ThreadIdsEqual(&id,&m_owner) )
		{
			throw "\n\tinvalid Wait call, Wait can not be called more than once"
				"\n\twithout a corresponding call to Reset!\n";
		}
		ThreadId_t zero;
		memset(&zero,0,sizeof(ThreadId_t));

		if( memcmp(&zero,&m_owner,sizeof(ThreadId_t)) != 0 )
		{
			throw "\n\tanother thread is already waiting on this event!\n";
		}

		m_owner = CThread::ThreadId();
#ifdef WINDOWS
		if( WaitForSingleObject(m_event,INFINITE) != WAIT_OBJECT_0 )
		{
			return FALSE;
		}
#else
		pthread_mutex_lock(&m_lock);
		pthread_cond_wait(&m_ready,&m_lock);
		return TRUE;
#endif
	}
	catch( char *psz )
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CEventClass::Wait",MB_ICONHAND);
		exit(-1);
#else
		cerr << "Fatal exception CEventClass::Wait: " << psz;
#endif

	}
	return TRUE;
}

/**
 *
 * Reset
 * reset an event flag to unsignaled
 * wait must be paired with reset within the same thread.
 *
 **/
void
CEventClass::Reset()
{
	try 
	{
		ThreadId_t id = CThread::ThreadId();
		if( !CThread::ThreadIdsEqual(&id,&m_owner) )
		{
			throw "\n\tunbalanced call to Reset, Reset must be called from\n"
				  "\n\tthe same Wait-Reset pair!\n";
		}

		memset(&m_owner,0,sizeof(ThreadId_t));

#ifndef WINDOWS
		pthread_mutex_unlock(&m_lock);
#endif
	}
	catch( char *psz )
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CEventClass::Reset",MB_ICONHAND);
		exit(-1);
#else
		cerr << "Fatal exception CEventClass::Reset: " << psz;
#endif

	}
}

