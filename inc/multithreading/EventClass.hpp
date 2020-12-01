#ifndef EVENT_CLASS
#define EVENT_CLASS

class CEventClass {
private:
	ThreadId_t m_owner;
#ifdef WINDOWS
	HANDLE m_event;
#else
	pthread_cond_t m_ready;
	pthread_mutex_t m_lock;
#endif
public:
	BOOL m_bCreated;
	void Set();
	BOOL Wait();
	void Reset();
	CEventClass(void);
	~CEventClass(void);
};

#endif

