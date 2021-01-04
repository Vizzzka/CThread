#include "multithreading/Thread.hpp"
#include <iostream>
#include <windows.h>
using namespace std;

CMutexClass m;

class MyThread : public IRunnable {
public:
virtual void run() {
		m.Lock();
		Sleep(1000);
		std::cout << "Thread " << CThread::ThreadId() << " is running\n";
		m.Unlock();
	}
	MyThread() {
		counter = 0;
	}
	void Increment() {
		counter++;
	}
	int GetCounter() {
		return counter;
	}
private:
	int counter;
};

int main(int argc, char *argv[] )
{
	std::cout << "Main thread id " << CThread::ThreadId() << "\n";
	std::cout << "Start\n";
	CThread* arr_thread[20];
	for (int i = 0; i < 20; ++i) {
		MyThread* obj = new MyThread();
		arr_thread[i] = new CThread(obj);
		arr_thread[i]->Start();
	}

	for (int i = 0; i < 20; i++) {
		arr_thread[i]->Join();
	}

	std::cout << "End\n";
	return 0;
}


