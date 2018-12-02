#include <iostream>
#include <WorkerThread.h>
#include <ThreadMsg.h>
#include <AsyncLogger.h>

using namespace std;

std::atomic<int> Loggers::g_loggerId = 0;


WorkerThread workerThread1("WT1");
WorkerThread workerThread2("WT2");

int ThreadMsg::g_seq = 0;

int main(void)
{
	// Create worker threads
	workerThread1.createThread();
	workerThread2.createThread();

	// Create message to send to worker thread 1
	UserData* userData1 = new UserData();
	userData1->msg = "Hello world";
	userData1->year = 2017;

	// Post the message to worker thread 1
	workerThread1.postMsg(userData1);

	// Create message to send to worker thread 2
	UserData* userData2 = new UserData();
	userData2->msg = "Goodbye 2017 and Hello 2018";
	userData2->year = 2018;

	// Post the message to worker thread 2
	workerThread2.postMsg(userData2);

	// Give time for messages processing on worker threads
	this_thread::sleep_for(10s);

	workerThread1.exitThread();
	workerThread2.exitThread();

	return 0;
}