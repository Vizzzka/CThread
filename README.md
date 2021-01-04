# C++ Multithreading Library

It is cross-platform object-oriented library of classes for multithreading in C++. The interface of this library is similar to Java 11.

It uses WIN API and POSIX threads API to implement multithreading.

It works on Windows/Linux/MacOS platforms. Classes which were implemented: CThread, CEvent, CMutexClass and CThreadPool. CEvent class gives wait/notify functionality.

Complete doxygen documentation is located in /docs folder and unfortunatly is not currently hosting but could simply be opened in browser by yourself.

## Team

 - [Usachova Victoria](https://github.com/Vizzzka)

## Prerequisites

 - **C++ compiler** - needs to support **C++17** standard
 - **CMake** 3.15+
 
Dependencies (such as development libraries) can be found in the [dependencies folder](./dependencies) in the form of the text files with package names for different package managers.

## Installing

1. Clone the project.
    ```bash
    git clone https://github.com/Vizzzka/CThread.git
    ```
2. Install required packages.

   On Ubuntu:
   ```bash
   [[ -r dependencies/apt.txt ]] && sed 's/#.*//' dependencies/apt.txt | xargs sudo apt-get install -y
   ```
   On MacOS:
   ```bash
   [[ -r dependencies/homebrew.txt ]] && sed 's/#.*//' dependencies/homebrew.txt | xargs brew install
   ```
   Use Conan on Windows.
3. Build.
    ```bash
    cmake -Bbuild
    cmake --build build
    ```

## Usage

The CThread class has following methods.

Constructors takes pointer to the runnable object.  
```c++
CThread::CThread(IRunnable *ptr)
```
Start method invokes run() method which was implemented in runnable object of the thread.
```c++
void CThread::Start()
```

Join method waits until the end of thread execution.
```c++
void CThread::Join()
```

Example of use:

```c++
// Example
class MyRunnabmle : public IRunnable {
    virtual void run() {
	       std::cout << "Thread " << CThread::ThreadId() << " is running\n";
	   }
}
CThread* my_thread = new CThread(new MyRunnable());
my_thread->Start();
my_thread->Join();
```
The CEventClass has following methods:

Set method signal thread that event occured:
```c++
void Set();
```

Thread waits until event state is set to signaled.
```c++
bool Wait();
```

Reset event signaled to unsignaled. 
```c++
void Reset();
```


Example of use:

```c++
class MyConsumer : public IRunnable {
	virtual void run() {
		event.Wait();     // wait for an event to occur
		// perform some task
		std::cout << "Consumer get number: " << number << "\n";
		event.Reset();    // reset the event to un-signaled
	}

};

class MyProducer : public IRunnable {
	virtual void run() {
		std::cout << "Producing data...\n";
		Sleep(1000);
		number = 10;
		event.Set();
	}
};

int main() {
	CThread* cons = new CThread(new  MyConsumer());
	CThread* prod = new CThread(new MyProducer());
	cons->Start();
	prod->Start();
	cos->Join();
	prod->Join();
}
```
CExecutor class has following methods:

Constructor creates a pool of pointed size
```c++
CExecutor(int size);
```
Execute submit runnable object to the pool
```c++
void execute(IRunnable *ptr);
```
Example of use:
```c++
CExecutor pool(5);
pool.execute(new CThread(new MyRunnable()));
pool.execute(new new CThread(new MyRunnable());
```

More documentation in /docs.

Help flags `-h`/`--help` support is available.
