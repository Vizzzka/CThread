#include "multithreading/ThreadPool.h"


/*
Executor::Executor(int size) {
	class MyRunnable : public IRunnable {
	public:
		virtual void run() {
			while(!done)
			{
				std::function<void()> task;
				if(work_queue.try_pop(task))
				{
					task();
				}
				else
				{
					std::this_thread::yield();
				}
			}
		}
	};
	m_iPoolSize = size;
	try {
		for (int i = 0; i < m_iPoolSize; ++i) {
			threads.push_back(new MyRunnable());
		}
	}
	catch(...) {
		done = true;
	}
}
*/

Executor::~Executor() {
	done = true;
	for(int i = 0; i < m_iPoolSize; ++i)
	{
		threads[i]->Join();
	}
};

void Executor::execute(IRunnable *ptr) {
	work_queue.push(ptr);
}



