#ifndef MUTEX_CLASS
#define MUTEX_CLASS

#ifndef WINDOWS
#if defined(WIN32) || defined(WIN64)
#define WINDOWS
#endif
#endif

#ifndef WINDOWS
#include <pthread.h>
#endif
#include "Thread.hpp"

class CMutexClass {
private:
#ifdef WINDOWS
	HANDLE m_mutex;
#else
	pthread_mutex_t m_mutex;
#endif
	ThreadId_t m_owner;
public:
	BOOL m_bCreated;
	void Lock();
	void Unlock();
	CMutexClass(void);
	~CMutexClass(void);
};
#endif

