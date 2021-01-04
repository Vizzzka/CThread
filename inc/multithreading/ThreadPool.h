#ifndef ADDER_THREADPOOL_H
#define ADDER_THREADPOOL_H

#include "Thread.hpp"
#include "EventClass.hpp"
#include "MutexClass.hpp"
#include <queue>
#include <functional>
#include <thread>

/*
template<class T> class Future {
public:
	Future(ICallable<T> *ptr);
	T get();
	bool isDone();
private:
	bool is_done;
	T m_future;
};
*/

template <typename T>
class thread_safe_queue {
public:
	bool try_pop();
	void push(T value);
private:
	std::queue<T> q;
};

class Executor {
public:
	Executor(int size);
	~Executor();
	void execute(IRunnable *ptr);
private:
	bool done;
	int m_iPoolSize=std::thread::hardware_concurrency();
	thread_safe_queue<IRunnable*> work_queue;
	std::vector<CThread*> threads;
	IRunnable* myWorkerThread;
};

#endif //ADDER_THREADPOOL_H
